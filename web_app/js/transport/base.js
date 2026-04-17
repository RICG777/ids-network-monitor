// Abstract base transport. Holds everything that's independent of the
// physical link: FIFO command queue, request/response correlation, event
// dispatch. Concrete transports (mock, webserial, http) just implement
// the write path and feed lines back via _handleLine().
//
// Events emitted:
//   'state'    { status: 'connecting'|'connected'|'disconnected'|'error', ...extra }
//   'message'  {type, key, value, raw}  — every parsed line, solicited or not
//   'line'     {raw}                    — every line, pre-parse, for raw log
//   'blocking' { on: bool, reason?, durationMs? }
//   'reboot'   {}                       — fired when reboot sentinel observed

import { parseLine, getCommandMetadata, REBOOT_SENTINEL } from '../protocol.js';

export class TimeoutError extends Error {
    constructor(cmd) { super(`Timeout waiting for response to "${cmd}"`); this.cmd = cmd; this.name = 'TimeoutError'; }
}
export class CommandError extends Error {
    constructor(cmd, message, parsed) { super(`${cmd}: ${message}`); this.cmd = cmd; this.parsed = parsed; this.name = 'CommandError'; }
}
export class NotConnectedError extends Error {
    constructor() { super('Transport is not connected'); this.name = 'NotConnectedError'; }
}

export class BaseTransport extends EventTarget {
    constructor() {
        super();
        this.status = 'disconnected';
        this._queue = [];
        this._inflight = null;
        this._stateGetters = {};
    }

    // Subclasses override. Must throw on failure.
    async _connectImpl(_opts) { throw new Error('not implemented'); }
    async _disconnectImpl() { throw new Error('not implemented'); }
    async _writeLineImpl(_text) { throw new Error('not implemented'); }

    get kind() { return 'base'; }
    get displayName() { return 'Base'; }

    // Allow UI to supply state accessors for dynamic timeout calculation
    // (e.g. TTEST duration from current relay config).
    setStateGetters(getters) { this._stateGetters = getters ?? {}; }

    async connect(opts = {}) {
        if (this.status === 'connected' || this.status === 'connecting') return;
        this._setState('connecting');
        try {
            const extras = (await this._connectImpl(opts)) ?? {};
            this._setState('connected', extras);
        } catch (err) {
            this._setState('error', { error: err?.message ?? String(err) });
            this._setState('disconnected');
            throw err;
        }
    }

    async disconnect() {
        if (this.status === 'disconnected') return;
        this._cancelAllInflight(new NotConnectedError());
        try { await this._disconnectImpl(); } catch {}
        this._setState('disconnected');
    }

    // Queue a command, return a promise that resolves/rejects when the board
    // responds (or times out). Options:
    //   timeoutMs:  override default timeout
    //   meta:       pre-computed metadata (rare — engine derives it otherwise)
    async sendCommand(cmd, opts = {}) {
        if (this.status !== 'connected') {
            throw new NotConnectedError();
        }
        return new Promise((resolve, reject) => {
            const metadata = opts.meta ?? getCommandMetadata(cmd, opts, this._stateGetters);
            this._queue.push({
                cmd,
                opts,
                metadata,
                resolve,
                reject,
                started: false,
                timeoutId: null,
                idleId: null,
                collectedStatus: [],
                sawAck: false,
            });
            this._pump();
        });
    }

    // --- internal ---

    _setState(status, extra = {}) {
        this.status = status;
        this.dispatchEvent(new CustomEvent('state', { detail: { status, ...extra } }));
    }

    _pump() {
        if (this._inflight || this._queue.length === 0) return;
        if (this.status !== 'connected') {
            // Drain queue with errors if connection dropped
            this._cancelAllInflight(new NotConnectedError());
            return;
        }
        const entry = this._queue.shift();
        this._inflight = entry;
        entry.started = true;

        const { metadata } = entry;

        if (metadata.blocking) {
            this.dispatchEvent(new CustomEvent('blocking', {
                detail: { on: true, reason: metadata.blockingReason, durationMs: metadata.blockingMs ?? 0 }
            }));
        }

        entry.timeoutId = setTimeout(() => {
            this._finishInflight({ error: new TimeoutError(entry.cmd) });
        }, metadata.timeoutMs);

        // Fire the write
        Promise.resolve()
            .then(() => this._writeLineImpl(entry.cmd + '\r\n'))
            .catch(err => this._finishInflight({ error: err }));
    }

    _finishInflight({ result, error }) {
        const entry = this._inflight;
        if (!entry) return;
        this._inflight = null;
        if (entry.timeoutId) { clearTimeout(entry.timeoutId); entry.timeoutId = null; }
        if (entry.idleId) { clearTimeout(entry.idleId); entry.idleId = null; }
        if (entry.metadata.blocking) {
            this.dispatchEvent(new CustomEvent('blocking', { detail: { on: false } }));
        }
        if (error) entry.reject(error);
        else       entry.resolve(result);
        // Schedule next
        setTimeout(() => this._pump(), 0);
    }

    _cancelAllInflight(err) {
        if (this._inflight) {
            const e = this._inflight;
            this._inflight = null;
            if (e.timeoutId) clearTimeout(e.timeoutId);
            if (e.idleId)    clearTimeout(e.idleId);
            if (e.metadata.blocking) {
                this.dispatchEvent(new CustomEvent('blocking', { detail: { on: false } }));
            }
            e.reject(err);
        }
        const q = this._queue;
        this._queue = [];
        for (const e of q) e.reject(err);
    }

    // Called by subclasses when a line arrives from the wire.
    _handleLine(rawLine) {
        if (rawLine === null || rawLine === undefined) return;
        this.dispatchEvent(new CustomEvent('line', { detail: { raw: rawLine } }));
        const parsed = parseLine(rawLine);
        this.dispatchEvent(new CustomEvent('message', { detail: parsed }));

        if (parsed.type === 'msg' && parsed.value.startsWith(REBOOT_SENTINEL)) {
            this.dispatchEvent(new CustomEvent('reboot', { detail: {} }));
        }

        const entry = this._inflight;
        if (!entry) return;
        const { metadata } = entry;

        if (metadata.terminator === 'ack') {
            if (parsed.type === 'ack' && parsed.key === metadata.expectAckKey) {
                this._finishInflight({ result: { ok: true, parsed, raw: parsed.raw } });
                return;
            }
            if (parsed.type === 'err' && parsed.key === metadata.expectAckKey) {
                this._finishInflight({ error: new CommandError(entry.cmd, parsed.value, parsed) });
                return;
            }
            return;
        }

        if (metadata.terminator === 'idle') {
            if (parsed.type === 'status') {
                entry.collectedStatus.push(parsed);
                if (entry.idleId) clearTimeout(entry.idleId);
                entry.idleId = setTimeout(() => {
                    this._finishInflight({ result: { ok: true, status: entry.collectedStatus } });
                }, metadata.idleMs ?? 150);
            } else if (parsed.type === 'err') {
                this._finishInflight({ error: new CommandError(entry.cmd, parsed.value, parsed) });
            }
            return;
        }

        if (typeof metadata.terminator === 'string' && metadata.terminator.startsWith('msg:')) {
            const substr = metadata.terminator.slice(4);
            if (parsed.type === 'ack' && parsed.key === metadata.expectAckKey) {
                entry.sawAck = true;
                return;
            }
            if (parsed.type === 'err' && parsed.key === metadata.expectAckKey) {
                this._finishInflight({ error: new CommandError(entry.cmd, parsed.value, parsed) });
                return;
            }
            if (parsed.type === 'msg' && parsed.value.includes(substr)) {
                this._finishInflight({ result: { ok: true, completion: parsed } });
                return;
            }
        }
    }
}
