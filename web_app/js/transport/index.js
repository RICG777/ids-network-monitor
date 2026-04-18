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

// Priority order for selecting the transport kind:
//   1. ?transport= URL param (explicit override, always wins)
//   2. Non-localhost HTTP origin -> 'http' (served from the board itself,
//      v2f.5+). Means hitting http://<board>/ picks HTTP automatically,
//      no URL params needed.
//   3. localStorage idsnm.transport (last user choice).
//   4. Web Serial if supported, else mock.
//
// The 'webserial' branch gracefully falls back to 'mock' if the browser
// doesn't implement Web Serial (Firefox, Safari).
export function pickTransportKind() {
    const params = new URLSearchParams(window.location.search);
    const fromUrl = params.get('transport');
    if (fromUrl) {
        if (fromUrl === 'webserial' && !WebSerialTransport.isSupported()) return 'mock';
        return fromUrl;
    }

    const host = window.location.hostname;
    const isHttpProto = window.location.protocol === 'http:' || window.location.protocol === 'https:';
    const isLocalhost = host === 'localhost' || host === '127.0.0.1' || host === '';
    if (isHttpProto && !isLocalhost) {
        return 'http';
    }

    let stored = null;
    try { stored = localStorage.getItem('idsnm.transport'); } catch {}
    if (stored) {
        if (stored === 'webserial' && !WebSerialTransport.isSupported()) return 'mock';
        return stored;
    }

    return WebSerialTransport.isSupported() ? 'webserial' : 'mock';
}

export function persistTransportKind(kind) {
    try { localStorage.setItem('idsnm.transport', kind); } catch {}
}
