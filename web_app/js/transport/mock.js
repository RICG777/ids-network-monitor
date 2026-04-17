// Mock transport — simulates a v2f.1 firmware in memory. Lets us drive the
// full UI without real hardware attached. Activated via ?transport=mock.
//
// URL-driven fault injection:
//   ?mock=slow      — 200 ms extra latency per response
//   ?mock=offline   — connect rejects
//   ?mock=reboot    — emits boot-message storm 10 s after connect
//   ?mock=timeout   — TEST1 never emits completion msg (exercise timeout path)

import { BaseTransport } from './base.js';

function mockFaults() {
    const params = new URLSearchParams(window.location.search);
    const raw = params.getAll('mock');
    const set = new Set(raw.flatMap(v => v.split(',')).map(s => s.trim()).filter(Boolean));
    return set;
}

function defaultFirmwareState() {
    return {
        fw: '2f.1',
        mac: 'EC:C9:FF:BC:A7:C4',
        ip: '192.168.68.142',
        subnet: '255.255.255.0',
        gateway: '0.0.0.0',
        r1Triggers: 'port up',
        r2Triggers: 'port down',
        r1Mode: 'PULSE',
        r2Mode: 'PULSE',
        r1Duration: 10,
        r2Duration: 10,
        r1State: 'OFF',
        r2State: 'OFF',
        verbosity: 1,
        jumper: 'OPEN',
        heap: 271480,
        ping: {
            enabled: false,
            interval: 30,
            threshold: 3,
            relay: 1,
            mode: 'AUTO',
            duration: 10,
            targets: [], // [{ip,fails,state}]
            rstate: 'OFF',
        },
    };
}

export class MockTransport extends BaseTransport {
    constructor(opts = {}) {
        super();
        this.opts = opts;
        this._fw = defaultFirmwareState();
        this._faults = mockFaults();
        this._rebootTimeout = null;
    }

    get kind() { return 'mock'; }
    get displayName() { return 'Mock (no hardware)'; }

    async _connectImpl() {
        if (this._faults.has('offline')) {
            await new Promise(r => setTimeout(r, 100));
            throw new Error('Mock connect refused (?mock=offline)');
        }
        await this._delay(50);

        // Emit a small "boot-like" settle so the Event Log looks alive
        this._emitLineLater(`ArdMsg: (mock) connected — simulated v${this._fw.fw} board`, 20);

        if (this._faults.has('reboot')) {
            this._rebootTimeout = setTimeout(() => this._simulateReboot(), 10000);
        }
    }

    async _disconnectImpl() {
        if (this._rebootTimeout) { clearTimeout(this._rebootTimeout); this._rebootTimeout = null; }
    }

    async _writeLineImpl(text) {
        const cmd = text.replace(/\r?\n$/, '').replace(/\r$/, '');
        await this._delay(this._faults.has('slow') ? 200 : 5);
        await this._handleCommand(cmd);
    }

    _emitLine(line) { this._handleLine(line); }

    _emitLineLater(line, ms) { setTimeout(() => this._emitLine(line), ms); }

    async _delay(ms) { return new Promise(r => setTimeout(r, ms)); }

    async _handleCommand(cmd) {
        const fw = this._fw;

        // --- Network ---
        if (cmd.startsWith('IP:')) {
            const val = cmd.slice(3);
            if (!isValidIP(val)) return this._emitLine(`ArdERR:IP:invalid IP`);
            fw.ip = val;
            return this._emitLine(`ArdACK:IP:${val}`);
        }
        if (cmd.startsWith('SUBNET:')) {
            const val = cmd.slice(7);
            fw.subnet = val;
            return this._emitLine(`ArdACK:SUBNET:${val}`);
        }
        if (cmd.startsWith('GATEWAY:')) {
            const val = cmd.slice(8);
            fw.gateway = val;
            return this._emitLine(`ArdACK:GATEWAY:${val}`);
        }

        // --- Relay triggers ---
        if (cmd.startsWith('Relay1:')) {
            const val = cmd.slice(7);
            if (val.length === 0 || val.length >= 120) {
                return this._emitLine(`ArdERR:Relay1:must be 1-119 chars`);
            }
            fw.r1Triggers = val;
            return this._emitLine(`ArdACK:Relay1:${val}`);
        }
        if (cmd.startsWith('Relay2:')) {
            const val = cmd.slice(7);
            if (val.length === 0 || val.length >= 120) {
                return this._emitLine(`ArdERR:Relay2:must be 1-119 chars`);
            }
            fw.r2Triggers = val;
            return this._emitLine(`ArdACK:Relay2:${val}`);
        }

        // --- Relay mode / duration ---
        if (cmd.startsWith('MODE1:')) {
            const val = cmd.slice(6) === 'LATCH' ? 'LATCH' : 'PULSE';
            fw.r1Mode = val;
            return this._emitLine(`ArdACK:MODE1:${val}`);
        }
        if (cmd.startsWith('MODE2:')) {
            const val = cmd.slice(6) === 'LATCH' ? 'LATCH' : 'PULSE';
            fw.r2Mode = val;
            return this._emitLine(`ArdACK:MODE2:${val}`);
        }
        if (cmd.startsWith('DURATION1:')) {
            const n = parseInt(cmd.slice(10), 10);
            if (!(n >= 1 && n <= 255)) return this._emitLine(`ArdERR:DURATION1:must be 1-255`);
            fw.r1Duration = n;
            return this._emitLine(`ArdACK:DURATION1:${n}`);
        }
        if (cmd.startsWith('DURATION2:')) {
            const n = parseInt(cmd.slice(10), 10);
            if (!(n >= 1 && n <= 255)) return this._emitLine(`ArdERR:DURATION2:must be 1-255`);
            fw.r2Duration = n;
            return this._emitLine(`ArdACK:DURATION2:${n}`);
        }

        // --- Verbosity ---
        if (cmd.startsWith('VERBOSITY:')) {
            const n = parseInt(cmd.slice(10), 10);
            if (!(n >= 0 && n <= 2)) return this._emitLine(`ArdERR:VERBOSITY:must be 0-2`);
            fw.verbosity = n;
            return this._emitLine(`ArdACK:VERBOSITY:${n}`);
        }

        // --- Relay manual ---
        if (cmd === 'RESET1') { fw.r1State = 'OFF'; return this._emitLine('ArdACK:RESET1:OK'); }
        if (cmd === 'RESET2') { fw.r2State = 'OFF'; return this._emitLine('ArdACK:RESET2:OK'); }

        if (cmd === 'TEST1') {
            this._emitLine('ArdACK:TEST1:pulsing 3s');
            if (this._faults.has('timeout')) return;  // never complete
            setTimeout(() => this._emitLine('ArdMsg: Relay 1 test complete.'), 3000);
            return;
        }
        if (cmd === 'TEST2') {
            this._emitLine('ArdACK:TEST2:pulsing 3s');
            setTimeout(() => this._emitLine('ArdMsg: Relay 2 test complete.'), 3000);
            return;
        }
        if (cmd === 'TTEST1') {
            this._emitLine(`ArdACK:TTEST1:pulsing ${fw.r1Duration}s`);
            setTimeout(() => this._emitLine('ArdMsg: Relay 1 timed test complete.'),
                       fw.r1Duration * 1000);
            return;
        }
        if (cmd === 'TTEST2') {
            this._emitLine(`ArdACK:TTEST2:pulsing ${fw.r2Duration}s`);
            setTimeout(() => this._emitLine('ArdMsg: Relay 2 timed test complete.'),
                       fw.r2Duration * 1000);
            return;
        }

        // --- Ping commands (all under PING:) ---
        if (cmd.startsWith('PING:')) return this._handlePing(cmd.slice(5));

        // --- STATUS ---
        if (cmd === 'STATUS') return this._emitStatus();

        // Unknown — silent in the real firmware
        this._emitLine(`ArdMsg: (mock) unknown command: ${cmd}`);
    }

    _handlePing(sub) {
        const fw = this._fw;
        const p = fw.ping;

        if (sub.startsWith('ADD:')) {
            const ip = sub.slice(4);
            if (!isValidIP(ip)) return this._emitLine('ArdERR:PING:ADD:invalid IP');
            if (p.targets.find(t => t.ip === ip)) return this._emitLine('ArdERR:PING:ADD:duplicate');
            if (p.targets.length >= 20) return this._emitLine('ArdERR:PING:ADD:full');
            p.targets.push({ ip, fails: 0, state: 'OK' });
            return this._emitLine(`ArdACK:PING:ADD:${ip}`);
        }
        if (sub.startsWith('DEL:')) {
            const ip = sub.slice(4);
            const i = p.targets.findIndex(t => t.ip === ip);
            if (i === -1) return this._emitLine('ArdERR:PING:DEL:not found');
            p.targets.splice(i, 1);
            return this._emitLine(`ArdACK:PING:DEL:${ip}`);
        }
        if (sub === 'LIST')   return this._emitPingStatus();
        if (sub.startsWith('INT:')) {
            const n = parseInt(sub.slice(4), 10);
            if (!(n >= 5 && n <= 255)) return this._emitLine('ArdERR:PING:INT must be 5-255');
            p.interval = n;
            return this._emitLine(`ArdACK:PING:INT:${n}`);
        }
        if (sub.startsWith('THR:')) {
            const n = parseInt(sub.slice(4), 10);
            if (!(n >= 1 && n <= 255)) return this._emitLine('ArdERR:PING:THR must be 1-255');
            p.threshold = n;
            return this._emitLine(`ArdACK:PING:THR:${n}`);
        }
        if (sub.startsWith('RELAY:')) {
            const n = parseInt(sub.slice(6), 10);
            if (!(n >= 1 && n <= 2)) return this._emitLine('ArdERR:PING:RELAY must be 1-2');
            p.relay = n;
            return this._emitLine(`ArdACK:PING:RELAY:${n}`);
        }
        if (sub.startsWith('MODE:')) {
            const mode = sub.slice(5);
            if (!['AUTO', 'LATCH', 'PULSE'].includes(mode))
                return this._emitLine('ArdERR:PING:MODE must be AUTO/LATCH/PULSE');
            p.mode = mode;
            return this._emitLine(`ArdACK:PING:MODE:${mode}`);
        }
        if (sub.startsWith('DUR:')) {
            const n = parseInt(sub.slice(4), 10);
            if (!(n >= 1 && n <= 255)) return this._emitLine('ArdERR:PING:DUR must be 1-255');
            p.duration = n;
            return this._emitLine(`ArdACK:PING:DUR:${n}`);
        }
        if (sub === 'ON')    { p.enabled = true;  return this._emitLine('ArdACK:PING:ON'); }
        if (sub === 'OFF')   { p.enabled = false; return this._emitLine('ArdACK:PING:OFF'); }
        if (sub === 'RESET') {
            p.rstate = 'OFF';
            for (const t of p.targets) { t.fails = 0; t.state = 'OK'; }
            return this._emitLine('ArdACK:PING:RESET:OK');
        }
        if (sub === 'CLR') {
            p.targets = [];
            p.rstate = 'OFF';
            return this._emitLine('ArdACK:PING:CLR:OK');
        }
        this._emitLine(`ArdMsg: (mock) unknown PING subcommand: ${sub}`);
    }

    _emitStatus() {
        const fw = this._fw;
        const lines = [
            `ArdSTATUS:FW:${fw.fw}`,
            `ArdSTATUS:MAC:${fw.mac}`,
            `ArdSTATUS:IP:${fw.ip}`,
            `ArdSTATUS:SUBNET:${fw.subnet}`,
            `ArdSTATUS:GATEWAY:${fw.gateway}`,
            `ArdSTATUS:Relay1:${fw.r1Triggers}`,
            `ArdSTATUS:Relay2:${fw.r2Triggers}`,
            `ArdSTATUS:R1Mode:${fw.r1Mode}`,
            `ArdSTATUS:R2Mode:${fw.r2Mode}`,
            `ArdSTATUS:R1Duration:${fw.r1Duration}`,
            `ArdSTATUS:R2Duration:${fw.r2Duration}`,
            `ArdSTATUS:R1State:${fw.r1State}`,
            `ArdSTATUS:R2State:${fw.r2State}`,
            `ArdSTATUS:VERBOSITY:${fw.verbosity}`,
            `ArdSTATUS:JUMPER:${fw.jumper}`,
            `ArdSTATUS:HEAP:${fw.heap}`,
        ];
        const pingLines = this._pingStatusLines();
        for (const l of [...lines, ...pingLines]) this._emitLine(l);
    }

    _emitPingStatus() {
        for (const l of this._pingStatusLines()) this._emitLine(l);
    }

    _pingStatusLines() {
        const p = this._fw.ping;
        const out = [
            `ArdSTATUS:PING:EN:${p.enabled ? 1 : 0}`,
            `ArdSTATUS:PING:INT:${p.interval}`,
            `ArdSTATUS:PING:THR:${p.threshold}`,
            `ArdSTATUS:PING:RELAY:${p.relay}`,
            `ArdSTATUS:PING:MODE:${p.mode}`,
            `ArdSTATUS:PING:DUR:${p.duration}`,
            `ArdSTATUS:PING:CNT:${p.targets.length}`,
        ];
        p.targets.forEach((t, i) => {
            out.push(`ArdSTATUS:PING:T${i}:${t.ip}:${t.state}:${t.fails}`);
        });
        out.push(`ArdSTATUS:PING:RSTATE:${p.rstate}`);
        return out;
    }

    _simulateReboot() {
        const fw = this._fw;
        this._emitLine(`ArdMsg: IDS Network Monitor v${fw.fw} starting...`);
        this._emitLine('ArdMsg: BLE disabled.');
        this._emitLine('ArdMsg: Jumper on boot: OPEN');
        this._emitLine(`ArdMsg: Ethernet cable connected.`);
        this._emitLine(`ArdMsg: Board IP: ${fw.ip}`);
        this._emitLine('ArdMsg: WiFi disabled.');
        this._emitLine('ArdMsg: Setup complete. Listening for syslog messages...');
    }

    // Test-only: let dev panel inject a raw line
    injectLine(raw) { this._emitLine(raw); }
}

function isValidIP(s) {
    const m = s.match(/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/);
    if (!m) return false;
    for (let i = 1; i <= 4; i++) {
        const n = Number(m[i]);
        if (n < 0 || n > 255) return false;
    }
    return !(m[1] === '0' && m[2] === '0' && m[3] === '0' && m[4] === '0') &&
           !(m[1] === '255' && m[2] === '255' && m[3] === '255' && m[4] === '255');
}
