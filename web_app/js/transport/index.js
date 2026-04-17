// Transport factory. The rest of the app never imports webserial/http/mock
// directly — it asks for a transport by kind and works against the uniform
// interface (connect, disconnect, sendCommand, EventTarget events).

import { MockTransport } from './mock.js';
import { WebSerialTransport } from './webserial.js';
import { HttpTransport } from './http.js';

export { WebSerialTransport };  // re-export for isSupported/requestPort

export function createTransport({ kind = 'mock', ...opts } = {}) {
    switch (kind) {
        case 'mock':      return new MockTransport(opts);
        case 'webserial': return new WebSerialTransport(opts);
        case 'http':      return new HttpTransport(opts);
        default: throw new Error(`Unknown transport kind: ${kind}`);
    }
}

// Resolve which transport to use from URL param or localStorage.
// Falls back to 'mock' if the preferred transport isn't supported in this
// browser, so the app is still usable on Firefox/Safari (Web Serial is
// Chrome/Edge only).
export function pickTransportKind() {
    const params = new URLSearchParams(window.location.search);
    const fromUrl = params.get('transport');
    const candidate = fromUrl
        ?? (() => { try { return localStorage.getItem('idsnm.transport'); } catch { return null; } })()
        ?? 'webserial';

    if (candidate === 'webserial' && !WebSerialTransport.isSupported()) {
        // Graceful fallback — a banner in the Connection tab will tell the user.
        return 'mock';
    }
    return candidate;
}

export function persistTransportKind(kind) {
    try { localStorage.setItem('idsnm.transport', kind); } catch {}
}
