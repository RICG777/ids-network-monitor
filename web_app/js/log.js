// Event log ring buffer. Lives for the whole session (keeps collecting even
// when the Event Log tab isn't mounted). Populated by app.js wiring from the
// transport's 'message' event.

class LogBuffer extends EventTarget {
    constructor(max = 2000) {
        super();
        this.max  = max;
        this.entries = [];
        this.seq = 0;
    }
    append(parsed) {
        const entry = {
            seq: ++this.seq,
            t: Date.now(),
            type: parsed.type,
            key: parsed.key,
            value: parsed.value,
            raw: parsed.raw,
        };
        this.entries.push(entry);
        if (this.entries.length > this.max) this.entries.shift();
        this.dispatchEvent(new CustomEvent('append', { detail: entry }));
        return entry;
    }
    clear() {
        this.entries = [];
        this.dispatchEvent(new CustomEvent('clear'));
    }
    all() { return this.entries.slice(); }
}

export const logBuffer = new LogBuffer();
