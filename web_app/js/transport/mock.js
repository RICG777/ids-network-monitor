// Mock transport for development without hardware.
// Phase 0: skeleton only — connect/disconnect emit state events, sendCommand
// just logs. Phase 1 wires up scripted ACK/STATUS responses so the full UI
// can be driven without a board.

export class MockTransport extends EventTarget {
    constructor(opts = {}) {
        super();
        this.opts = opts;
        this.connected = false;
    }

    get kind() { return 'mock'; }

    get displayName() { return 'Mock (no hardware)'; }

    async connect() {
        this._emit('state', { status: 'connecting' });
        await new Promise(r => setTimeout(r, 50));
        this.connected = true;
        this._emit('state', { status: 'connected' });
    }

    async disconnect() {
        this.connected = false;
        this._emit('state', { status: 'disconnected' });
    }

    async sendCommand(cmd, opts = {}) {
        console.log('[mock] sendCommand:', cmd, opts);
        return { ok: true, raw: `(mock) ${cmd}` };
    }

    _emit(type, detail) {
        this.dispatchEvent(new CustomEvent(type, { detail }));
    }
}
