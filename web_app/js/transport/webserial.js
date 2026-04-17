// Web Serial API transport. Runs in Chrome/Edge 89+ over USB CDC serial.
// All the hard bits of Web Serial live here: port lifecycle, reader/writer
// cleanup, disconnect detection, CRLF line splitting.
//
// Notes on reliability:
//   - flowControl MUST stay 'none'. Hardware RTS/CTS would silently stall.
//   - Physical unplug fires navigator.serial 'disconnect' AND closes the
//     reader pipeline. Handle both paths, dedupe.
//   - Clean shutdown order on our disconnect: writer.close() ->
//     readable.cancel() -> await pipeline promise -> port.close().
//     Skipping any of those risks "port.readable is locked" on reconnect.

import { BaseTransport, NotConnectedError } from './base.js';

function createLineSplitter() {
    let buffer = '';
    return new TransformStream({
        transform(chunk, controller) {
            buffer += chunk;
            const lines = buffer.split(/\r?\n/);
            buffer = lines.pop();
            for (const l of lines) controller.enqueue(l);
        },
        flush(controller) {
            if (buffer.length) controller.enqueue(buffer);
            buffer = '';
        },
    });
}

export class WebSerialTransport extends BaseTransport {
    constructor(opts = {}) {
        super();
        this.opts = opts;
        this.port = null;
        this.writer = null;
        this.readableClosed = null;
        this._disconnectHandler = null;
        this._manualDisconnect = false;
    }

    get kind() { return 'webserial'; }
    get displayName() { return this._portLabel() ?? 'USB (Web Serial)'; }

    static isSupported() {
        return typeof navigator !== 'undefined' && 'serial' in navigator;
    }

    // Return remembered/granted ports. Persisted per-origin by Chrome.
    static async getPorts() {
        if (!this.isSupported()) return [];
        return navigator.serial.getPorts();
    }

    // User-gesture port picker. Pass the resulting port to connect().
    static async requestPort(filters) {
        if (!this.isSupported()) throw new Error('Web Serial not supported — use Chrome or Edge 89+');
        const opts = filters ? { filters } : undefined;
        return navigator.serial.requestPort(opts);
    }

    async _connectImpl(opts = {}) {
        if (!WebSerialTransport.isSupported()) {
            throw new Error('Web Serial not supported — use Chrome or Edge 89+');
        }
        let port = opts.port ?? this.opts.port ?? null;
        if (!port) {
            const existing = await navigator.serial.getPorts();
            if (existing.length === 1) {
                port = existing[0];
            } else if (existing.length > 1 && opts.preferGrantedIndex != null) {
                port = existing[opts.preferGrantedIndex] ?? existing[0];
            } else {
                // No grant or ambiguous — prompt user (needs user gesture upstream)
                port = await navigator.serial.requestPort();
            }
        }
        await port.open({
            baudRate: opts.baudRate ?? 115200,
            dataBits: 8,
            stopBits: 1,
            parity: 'none',
            flowControl: 'none',
        });
        this.port = port;
        this._manualDisconnect = false;

        this._startReadLoop();
        try {
            this.writer = port.writable.getWriter();
        } catch (err) {
            // If writer acquisition fails, tear down the reader we just started
            await this._teardown();
            throw err;
        }

        this._disconnectHandler = (ev) => {
            if (ev.target === this.port) this._onExternalDisconnect();
        };
        navigator.serial.addEventListener('disconnect', this._disconnectHandler);

        const label = this._portLabel();
        return { portInfo: label, displayName: label };
    }

    _startReadLoop() {
        const decoder = new TextDecoderStream('utf-8', { fatal: false });
        const splitter = createLineSplitter();
        // Fire-and-forget: settle when the pipeline closes (cancel or unplug).
        this.readableClosed = this.port.readable
            .pipeThrough(decoder)
            .pipeThrough(splitter)
            .pipeTo(new WritableStream({
                write: (line) => { this._handleLine(line); },
            }))
            .catch(() => { /* expected on cancel/unplug */ });
        this.readableClosed.then(() => {
            if (!this._manualDisconnect) this._onExternalDisconnect();
        });
    }

    async _writeLineImpl(text) {
        if (!this.writer) throw new NotConnectedError();
        const data = new TextEncoder().encode(text);
        await this.writer.write(data);
    }

    async _disconnectImpl() {
        this._manualDisconnect = true;
        await this._teardown();
    }

    async _teardown() {
        if (this._disconnectHandler) {
            try { navigator.serial.removeEventListener('disconnect', this._disconnectHandler); } catch {}
            this._disconnectHandler = null;
        }
        try { if (this.writer) await this.writer.close(); } catch {}
        try { if (this.writer) this.writer.releaseLock(); } catch {}
        this.writer = null;
        try { if (this.port?.readable) await this.port.readable.cancel(); } catch {}
        try { if (this.readableClosed) await this.readableClosed; } catch {}
        this.readableClosed = null;
        try { if (this.port) await this.port.close(); } catch {}
        this.port = null;
    }

    _onExternalDisconnect() {
        if (this._manualDisconnect) return;
        if (this.status === 'disconnected') return;
        // Reader stream already done / port unplugged. Clean up lightly and
        // propagate state so UI shows Disconnected and offers Reconnect.
        try { if (this.writer) this.writer.releaseLock(); } catch {}
        this.writer = null;
        try { if (this.port) this.port.close(); } catch {}
        this.port = null;
        this._cancelAllInflight(new NotConnectedError());
        this._setState('disconnected', { portInfo: null, displayName: null });
    }

    _portLabel() {
        if (!this.port) return this.opts.lastLabel ?? null;
        try {
            const info = this.port.getInfo();
            if (info.usbVendorId != null && info.usbProductId != null) {
                const vid = info.usbVendorId.toString(16).padStart(4, '0').toUpperCase();
                const pid = info.usbProductId.toString(16).padStart(4, '0').toUpperCase();
                return `USB ${vid}:${pid}`;
            }
        } catch {}
        return 'USB serial';
    }
}
