// HTTP transport — stub. Arrives when firmware v2f.2 lands with an
// in-board HTTP API at http://<board>/api/*.
//
// The interface matches BaseTransport so swapping it in is a one-line
// change in the factory. For now, activating it via ?transport=http
// throws at construction time so no UI path silently breaks.

import { BaseTransport } from './base.js';

export class HttpTransport extends BaseTransport {
    constructor(opts = {}) {
        super();
        this.opts = opts;
    }

    get kind() { return 'http'; }
    get displayName() { return 'HTTP (ESP32-hosted)'; }

    static isSupported() { return true; }

    async _connectImpl(_opts) {
        throw new Error('HTTP transport not yet implemented — ships with firmware v2f.2');
    }
    async _disconnectImpl() { /* no-op */ }
    async _writeLineImpl(_text) {
        throw new Error('HTTP transport not yet implemented');
    }
}
