// Single-source-of-truth store for the app.
// Tabs and UI components subscribe to slices; the transport layer pushes
// updates as ACK/STATUS lines arrive from the board. Subscribers get the
// full state snapshot — picking out the relevant slice is their job.

const state = {
    connection: { status: 'disconnected', transport: 'mock', portInfo: null },
    firmware:   { version: null, expected: '2f.1', mismatch: false },
    network:    { ip: null, subnet: null, gateway: null, mac: null },
    relay1:     { triggers: '', mode: null, duration: null, state: null },
    relay2:     { triggers: '', mode: null, duration: null, state: null },
    system:     { verbosity: null, jumper: null, heap: null },
    ping:       {
        enabled: false, interval: null, threshold: null, relay: null,
        mode: null, duration: null, count: 0, rstate: null, targets: []
    },
    ui:         { blocking: false, blockingReason: '' },
};

const subscribers = new Set();

export function get() {
    return state;
}

// Shallow-merge partial at the top level; each top-level key can itself be
// a partial object, merged one level deep. Example:
//   update({ connection: { status: 'connected' } })
// merges only .connection.status, leaves other connection fields alone.
export function update(partial) {
    for (const key of Object.keys(partial)) {
        if (state[key] && typeof state[key] === 'object' && !Array.isArray(state[key])) {
            state[key] = { ...state[key], ...partial[key] };
        } else {
            state[key] = partial[key];
        }
    }
    for (const handler of subscribers) {
        handler(state);
    }
}

export function subscribe(handler) {
    subscribers.add(handler);
    handler(state);
    return () => subscribers.delete(handler);
}
