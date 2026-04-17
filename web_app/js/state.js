// Single-source-of-truth store for the app.
// Tabs and UI components subscribe to slices; the transport layer pushes
// updates as ACK/STATUS lines arrive from the board.

const state = {
    connection: { status: 'disconnected', transport: 'mock', portInfo: null, displayName: null },
    firmware:   { version: null, expected: '2f.1', mismatch: false, mismatchDismissed: false },
    network:    { ip: null, subnet: null, gateway: null, mac: null },
    relay1:     { triggers: '', mode: null, duration: null, state: null },
    relay2:     { triggers: '', mode: null, duration: null, state: null },
    system:     { verbosity: null, jumper: null, heap: null },
    ping:       {
        enabled: false, interval: null, threshold: null, relay: null,
        mode: null, duration: null, count: 0, rstate: null, targets: []
    },
    ui:         { blocking: false, blockingReason: '', blockingDurationMs: 0, blockingStart: 0 },
};

const subscribers = new Set();

export function get() { return state; }

// Shallow-merge partial at the top level; each top-level key can itself be
// a partial object, merged one level deep.
export function update(partial) {
    for (const key of Object.keys(partial)) {
        if (state[key] && typeof state[key] === 'object' && !Array.isArray(state[key])) {
            state[key] = { ...state[key], ...partial[key] };
        } else {
            state[key] = partial[key];
        }
    }
    for (const handler of subscribers) handler(state);
}

export function subscribe(handler) {
    subscribers.add(handler);
    handler(state);
    return () => subscribers.delete(handler);
}

// Translate a list of parsed ArdSTATUS: lines into a state update.
// STATUS dumps arrive as a batch; we collect and push one coherent update
// at the end to minimise re-renders.
export function applyStatusLines(lines) {
    const partial = {
        firmware: { ...state.firmware },
        network:  { ...state.network },
        relay1:   { ...state.relay1 },
        relay2:   { ...state.relay2 },
        system:   { ...state.system },
        ping:     { ...state.ping },
    };
    const targets = [];

    for (const line of lines) {
        if (line.type !== 'status') continue;
        const { key, value } = line;
        switch (key) {
            case 'FW':         partial.firmware.version = value; break;
            case 'MAC':        partial.network.mac = value; break;
            case 'IP':         partial.network.ip = value; break;
            case 'SUBNET':     partial.network.subnet = value; break;
            case 'GATEWAY':    partial.network.gateway = value; break;
            case 'Relay1':     partial.relay1.triggers = value; break;
            case 'Relay2':     partial.relay2.triggers = value; break;
            case 'R1Mode':     partial.relay1.mode = value; break;
            case 'R2Mode':     partial.relay2.mode = value; break;
            case 'R1Duration': partial.relay1.duration = Number(value); break;
            case 'R2Duration': partial.relay2.duration = Number(value); break;
            case 'R1State':    partial.relay1.state = value; break;
            case 'R2State':    partial.relay2.state = value; break;
            case 'VERBOSITY':  partial.system.verbosity = Number(value); break;
            case 'JUMPER':     partial.system.jumper = value; break;
            case 'HEAP':       partial.system.heap = Number(value); break;
            case 'PING:EN':    partial.ping.enabled = value === '1'; break;
            case 'PING:INT':   partial.ping.interval = Number(value); break;
            case 'PING:THR':   partial.ping.threshold = Number(value); break;
            case 'PING:RELAY': partial.ping.relay = Number(value); break;
            case 'PING:MODE':  partial.ping.mode = value; break;
            case 'PING:DUR':   partial.ping.duration = Number(value); break;
            case 'PING:CNT':   partial.ping.count = Number(value); break;
            case 'PING:RSTATE': partial.ping.rstate = value; break;
            default: {
                const m = key.match(/^PING:T(\d+)$/);
                if (m) {
                    const idx = Number(m[1]);
                    const parts = value.split(':');
                    if (parts.length === 3) {
                        targets[idx] = { ip: parts[0], state: parts[1], fails: Number(parts[2]) };
                    }
                }
            }
        }
    }
    // PING:T entries come after PING:CNT; construct the targets array from
    // those indices only (gaps filtered) so stale entries from previous
    // dumps don't bleed in.
    partial.ping.targets = targets.filter(Boolean);

    // Firmware mismatch detection (respects per-version dismissal)
    if (partial.firmware.version) {
        const dismissed = tryReadLocalStorage('idsnm.fwMismatchDismissed');
        partial.firmware.mismatch = partial.firmware.version !== partial.firmware.expected;
        partial.firmware.mismatchDismissed = dismissed === partial.firmware.version;
    }

    update(partial);
}

// Reset connection-dependent state (called on reboot sentinel or disconnect).
export function clearBoardState() {
    update({
        firmware: { version: null, mismatch: false, mismatchDismissed: false,
                    expected: state.firmware.expected },
        network:  { ip: null, subnet: null, gateway: null, mac: null },
        relay1:   { triggers: '', mode: null, duration: null, state: null },
        relay2:   { triggers: '', mode: null, duration: null, state: null },
        system:   { verbosity: null, jumper: null, heap: null },
        ping:     { enabled: false, interval: null, threshold: null, relay: null,
                    mode: null, duration: null, count: 0, rstate: null, targets: [] },
        ui:       { blocking: false, blockingReason: '', blockingDurationMs: 0, blockingStart: 0 },
    });
}

function tryReadLocalStorage(key) {
    try { return localStorage.getItem(key); } catch { return null; }
}
