// Transport factory. The rest of the app never imports webserial/http/mock
// directly — it asks for a transport by kind and works against the uniform
// interface (connect, disconnect, sendCommand, EventTarget events).

import { MockTransport } from './mock.js';

export function createTransport({ kind = 'mock', ...opts } = {}) {
    switch (kind) {
        case 'mock':
            return new MockTransport(opts);
        case 'webserial':
            throw new Error('Web Serial transport arrives in Phase 2');
        case 'http':
            throw new Error('HTTP transport arrives with firmware v2f.2');
        default:
            throw new Error(`Unknown transport kind: ${kind}`);
    }
}

// Resolve which transport to use from URL param or localStorage.
export function pickTransportKind() {
    const params = new URLSearchParams(window.location.search);
    const fromUrl = params.get('transport');
    if (fromUrl) return fromUrl;
    const stored = localStorage.getItem('idsnm.transport');
    if (stored) return stored;
    return 'mock';
}
