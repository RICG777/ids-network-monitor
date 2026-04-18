// HTTP transport — talks to the ESP32's built-in web server (firmware v2f.3+).
// Matches the BaseTransport event contract so every tab keeps working, but
// overrides sendCommand entirely: HTTP is request/response, no line parser,
// no ACK correlator needed. Responses from /api/* already return the full
// status doc, which we synthesise into "status" messages so applyStatusLines
// picks them up unchanged.
//
// Base URL:
//   ?host=192.168.68.142       -> http://192.168.68.142
//   (none)                     -> window.location.origin (works when the UI
//                                 is itself served from the board, v2f.5+)

import { BaseTransport, NotConnectedError, CommandError, TimeoutError } from './base.js';
import { getCommandMetadata, parseLine } from '../protocol.js';

function resolveBaseUrl(optHost) {
    if (optHost) return normaliseBase(optHost);
    const params = new URLSearchParams(window.location.search);
    const q = params.get('host');
    if (q) return normaliseBase(q);
    return window.location.origin;
}

function normaliseBase(host) {
    if (/^https?:\/\//i.test(host)) return host.replace(/\/$/, '');
    return `http://${host.replace(/\/$/, '')}`;
}

// Translate the /api/status JSON doc into the same shape applyStatusLines()
// consumes — one {type:'status', key, value, raw} per field. Keeps all
// downstream code (state.js, tab renderers, event log) unchanged.
function jsonToStatusLines(j) {
    const out = [];
    const push = (key, val) => {
        const v = String(val ?? '');
        out.push({ type: 'status', key, value: v, raw: `ArdSTATUS:${key}:${v}` });
    };
    push('FW', j.firmware);
    push('MAC', j.mac);
    push('IP',      j.network?.ip);
    push('SUBNET',  j.network?.subnet);
    push('GATEWAY', j.network?.gateway);
    push('Relay1', j.relay1?.triggers ?? '');
    push('Relay2', j.relay2?.triggers ?? '');
    push('R1Mode', j.relay1?.mode);
    push('R2Mode', j.relay2?.mode);
    push('R1Duration', j.relay1?.duration);
    push('R2Duration', j.relay2?.duration);
    push('R1State', j.relay1?.state);
    push('R2State', j.relay2?.state);
    push('VERBOSITY', j.verbosity);
    push('JUMPER',    j.jumper);
    push('HEAP',      j.heap);
    const p = j.ping ?? {};
    push('PING:EN',     p.enabled ? '1' : '0');
    push('PING:INT',    p.interval);
    push('PING:THR',    p.threshold);
    push('PING:RELAY',  p.relay);
    push('PING:MODE',   p.mode);
    push('PING:DUR',    p.duration);
    push('PING:CNT',    p.count ?? (p.targets ? p.targets.length : 0));
    if (Array.isArray(p.targets)) {
        p.targets.forEach((t, i) => {
            push(`PING:T${i}`, `${t.ip}:${t.state}:${t.fails}`);
        });
    }
    push('PING:RSTATE', p.rstate);
    return out;
}

// Each serial-protocol command maps to exactly one HTTP request.
function mapCommand(cmd) {
    const post = (path, body = {}) => ({ method: 'POST', path, body });
    const del  = (path)            => ({ method: 'DELETE', path, body: null });

    if (cmd === 'STATUS' || cmd === 'PING:LIST') return { method: 'GET', path: '/api/status', body: null };

    if (cmd.startsWith('IP:'))        return post('/api/network',  { ip:      cmd.slice(3) });
    if (cmd.startsWith('SUBNET:'))    return post('/api/network',  { subnet:  cmd.slice(7) });
    if (cmd.startsWith('GATEWAY:'))   return post('/api/network',  { gateway: cmd.slice(8) });

    if (cmd.startsWith('Relay1:'))    return post('/api/relay/1/triggers', { triggers: cmd.slice(7) });
    if (cmd.startsWith('Relay2:'))    return post('/api/relay/2/triggers', { triggers: cmd.slice(7) });
    if (cmd.startsWith('MODE1:'))     return post('/api/relay/1/mode',     { mode: cmd.slice(6) });
    if (cmd.startsWith('MODE2:'))     return post('/api/relay/2/mode',     { mode: cmd.slice(6) });
    if (cmd.startsWith('DURATION1:')) return post('/api/relay/1/duration', { seconds: parseInt(cmd.slice(10), 10) });
    if (cmd.startsWith('DURATION2:')) return post('/api/relay/2/duration', { seconds: parseInt(cmd.slice(10), 10) });
    if (cmd === 'RESET1')             return post('/api/relay/1/reset',    {});
    if (cmd === 'RESET2')             return post('/api/relay/2/reset',    {});
    if (cmd === 'TEST1')              return post('/api/relay/1/test',     { duration: 'quick' });
    if (cmd === 'TEST2')              return post('/api/relay/2/test',     { duration: 'quick' });
    if (cmd === 'TTEST1')             return post('/api/relay/1/test',     { duration: 'timed' });
    if (cmd === 'TTEST2')             return post('/api/relay/2/test',     { duration: 'timed' });

    if (cmd.startsWith('VERBOSITY:')) return post('/api/verbosity', { level: parseInt(cmd.slice(10), 10) });

    if (cmd === 'PING:ON')            return post('/api/ping/config', { enabled: true  });
    if (cmd === 'PING:OFF')           return post('/api/ping/config', { enabled: false });
    if (cmd.startsWith('PING:ADD:'))  return post('/api/ping/targets', { ip: cmd.slice(9) });
    if (cmd.startsWith('PING:DEL:'))  return del (`/api/ping/targets?ip=${encodeURIComponent(cmd.slice(9))}`);
    if (cmd.startsWith('PING:INT:'))  return post('/api/ping/config', { interval:  parseInt(cmd.slice(9),  10) });
    if (cmd.startsWith('PING:THR:'))  return post('/api/ping/config', { threshold: parseInt(cmd.slice(9),  10) });
    if (cmd.startsWith('PING:RELAY:'))return post('/api/ping/config', { relay:     parseInt(cmd.slice(11), 10) });
    if (cmd.startsWith('PING:MODE:')) return post('/api/ping/config', { mode:      cmd.slice(10) });
    if (cmd.startsWith('PING:DUR:'))  return post('/api/ping/config', { duration:  parseInt(cmd.slice(9),  10) });
    if (cmd === 'PING:RESET')         return post('/api/ping/reset', {});
    if (cmd === 'PING:CLR')           return post('/api/ping/clear', {});

    throw new Error(`Unmapped command: ${cmd}`);
}

export class HttpTransport extends BaseTransport {
    constructor(opts = {}) {
        super();
        this.opts = opts;
        this.baseUrl = resolveBaseUrl(opts.host);
    }

    get kind() { return 'http'; }
    get displayName() { return `HTTP ${this.baseUrl}`; }

    static isSupported() { return typeof fetch !== 'undefined'; }

    async _connectImpl(opts = {}) {
        if (opts.host) this.baseUrl = normaliseBase(opts.host);
        // Verify we can reach the board. Use a short timeout — if the board
        // is off-subnet this should fail quickly, not hang the UI.
        const res = await this._fetch('/api/status', { method: 'GET', timeoutMs: 4000 });
        if (!res.ok) {
            if (res.status === 403) {
                // Reachable but jumper open — surface a clearer message.
                throw new Error(`Board reachable at ${this.baseUrl} but /api/* is locked (UEXT jumper open).`);
            }
            throw new Error(`Board returned HTTP ${res.status} from ${this.baseUrl}`);
        }
        return { portInfo: this.baseUrl, displayName: `HTTP ${this.baseUrl}` };
    }

    async _disconnectImpl() { /* stateless */ }
    async _writeLineImpl(_text) { /* unused — sendCommand is overridden */ }

    async sendCommand(cmd, opts = {}) {
        if (this.status !== 'connected') throw new NotConnectedError();
        const meta = getCommandMetadata(cmd, opts, this._stateGetters);
        if (meta.blocking) {
            this.dispatchEvent(new CustomEvent('blocking', {
                detail: { on: true, reason: meta.blockingReason, durationMs: meta.blockingMs ?? 0 },
            }));
        }
        try {
            const mapped = mapCommand(cmd);
            const timeoutMs = meta.timeoutMs ?? opts.timeoutMs ?? 8000;
            const res = await this._fetch(mapped.path, {
                method: mapped.method,
                body:   mapped.body,
                timeoutMs,
            });
            let parsed = null;
            try { parsed = await res.json(); } catch { /* empty body OK for some codes */ }

            if (!res.ok) {
                const errKey = parsed?.error ?? `http-${res.status}`;
                const msg    = parsed?.message ?? errKey;
                // Synthesize an 'err' message so Event Log shows it
                this.dispatchEvent(new CustomEvent('message', { detail: {
                    type: 'err', key: cmd, value: msg,
                    raw: `ArdERR:HTTP:${res.status}:${errKey}${msg ? ':' + msg : ''}`,
                }}));
                throw new CommandError(cmd, msg, parsed);
            }

            // Successful responses return either the full status doc (after a
            // write) or the /api/status doc itself. Either way, parsed is the
            // doc — synthesise status messages.
            const statusLines = parsed ? jsonToStatusLines(parsed) : [];
            for (const line of statusLines) {
                this.dispatchEvent(new CustomEvent('message', { detail: line }));
            }
            // Synthesise an ACK so the Event Log + any ack-dependent UI sees one.
            const ackKey = this._ackKeyFor(cmd);
            const ackLine = parseLine(`ArdACK:${ackKey}:http`);
            this.dispatchEvent(new CustomEvent('message', { detail: ackLine }));

            return { ok: true, parsed: ackLine, raw: ackLine.raw, status: statusLines };
        } finally {
            if (meta.blocking) {
                this.dispatchEvent(new CustomEvent('blocking', { detail: { on: false } }));
            }
        }
    }

    async _fetch(path, { method = 'GET', body = null, timeoutMs = 4000 } = {}) {
        const ctrl = new AbortController();
        const timer = setTimeout(() => ctrl.abort(), timeoutMs);
        try {
            const opts = { method, signal: ctrl.signal };
            if (body != null) {
                opts.headers = { 'Content-Type': 'application/json' };
                opts.body    = JSON.stringify(body);
            }
            return await fetch(this.baseUrl + path, opts);
        } catch (err) {
            if (err.name === 'AbortError') throw new TimeoutError(method + ' ' + path);
            throw err;
        } finally {
            clearTimeout(timer);
        }
    }

    _ackKeyFor(cmd) {
        // Mirror what the serial protocol would send as the ACK key so the
        // Event Log reads consistently across transports.
        if (cmd.startsWith('PING:')) {
            const rest = cmd.slice(5);
            const colon = rest.indexOf(':');
            return 'PING:' + (colon === -1 ? rest : rest.slice(0, colon));
        }
        const colon = cmd.indexOf(':');
        return colon === -1 ? cmd : cmd.slice(0, colon);
    }
}
