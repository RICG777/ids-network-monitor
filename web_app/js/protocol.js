// Wire protocol for the v2f.1 firmware (and compatible).
// Pure module — no DOM, no transport. Responsible for:
//   1. Parsing a received line into {type, key, value, raw}.
//   2. Metadata table telling the transport how each command should resolve.
//
// Key parsing detail: ACK/ERR/STATUS keys can themselves contain colons
// (e.g. ArdACK:PING:ADD:192.168.1.5 — key is "PING:ADD", value is the IP).
// We do longest-known-prefix matching against a known-key set rather than
// naive first-colon-split.

// Keys that appear in ArdACK: and ArdERR: lines.
// Order matters only for matching: longest prefixes first. We sort on build.
const ACK_KEYS_RAW = [
    'IP', 'SUBNET', 'GATEWAY',
    'Relay1', 'Relay2',
    'MODE1', 'MODE2', 'DURATION1', 'DURATION2',
    'VERBOSITY',
    'RESET1', 'RESET2',
    'TEST1', 'TEST2', 'TTEST1', 'TTEST2',
    'PING:ADD', 'PING:DEL', 'PING:INT', 'PING:THR', 'PING:RELAY',
    'PING:MODE', 'PING:DUR', 'PING:ON', 'PING:OFF', 'PING:RESET', 'PING:CLR',
];

// Keys that appear in ArdSTATUS: lines. PING:T<n> handled separately via regex.
const STATUS_KEYS_RAW = [
    'FW', 'MAC', 'IP', 'SUBNET', 'GATEWAY',
    'Relay1', 'Relay2',
    'R1Mode', 'R2Mode', 'R1Duration', 'R2Duration', 'R1State', 'R2State',
    'VERBOSITY', 'JUMPER', 'HEAP',
    'PING:EN', 'PING:INT', 'PING:THR', 'PING:RELAY', 'PING:MODE',
    'PING:DUR', 'PING:CNT', 'PING:RSTATE',
];

// Sort longest-first so longest-prefix match wins.
const ACK_KEYS    = [...ACK_KEYS_RAW].sort((a, b) => b.length - a.length);
const STATUS_KEYS = [...STATUS_KEYS_RAW].sort((a, b) => b.length - a.length);

function matchKnownKey(rest, knownKeys) {
    for (const key of knownKeys) {
        if (rest === key) return { key, value: '' };
        if (rest.startsWith(key + ':')) return { key, value: rest.slice(key.length + 1) };
    }
    return null;
}

// Parse a single wire line. Returns {type, key, value, raw}.
// type: 'ack' | 'err' | 'status' | 'msg' | 'raw'
export function parseLine(rawLine) {
    const line = rawLine.replace(/\r$/, '');
    if (line.length === 0) return { type: 'raw', key: '', value: '', raw: line };

    if (line.startsWith('ArdACK:')) {
        const rest = line.slice('ArdACK:'.length);
        const hit = matchKnownKey(rest, ACK_KEYS);
        if (hit) return { type: 'ack', key: hit.key, value: hit.value, raw: line };
        // Unknown ACK — still classify as ack, take first colon as key/value split
        const idx = rest.indexOf(':');
        const key = idx === -1 ? rest : rest.slice(0, idx);
        const value = idx === -1 ? '' : rest.slice(idx + 1);
        return { type: 'ack', key, value, raw: line };
    }

    if (line.startsWith('ArdERR:')) {
        const rest = line.slice('ArdERR:'.length);
        const hit = matchKnownKey(rest, ACK_KEYS);
        if (hit) return { type: 'err', key: hit.key, value: hit.value, raw: line };
        const idx = rest.indexOf(':');
        const key = idx === -1 ? rest : rest.slice(0, idx);
        const value = idx === -1 ? '' : rest.slice(idx + 1);
        return { type: 'err', key, value, raw: line };
    }

    if (line.startsWith('ArdSTATUS:')) {
        const rest = line.slice('ArdSTATUS:'.length);
        // Dynamic PING:T<n> keys
        const mT = rest.match(/^(PING:T\d+):(.*)$/);
        if (mT) return { type: 'status', key: mT[1], value: mT[2], raw: line };
        const hit = matchKnownKey(rest, STATUS_KEYS);
        if (hit) return { type: 'status', key: hit.key, value: hit.value, raw: line };
        const idx = rest.indexOf(':');
        const key = idx === -1 ? rest : rest.slice(0, idx);
        const value = idx === -1 ? '' : rest.slice(idx + 1);
        return { type: 'status', key, value, raw: line };
    }

    if (line.startsWith('ArdMsg: ')) {
        return { type: 'msg', key: '', value: line.slice('ArdMsg: '.length), raw: line };
    }
    // Older firmware or malformed — still record, just mark as raw.
    return { type: 'raw', key: '', value: line, raw: line };
}

// Returns command metadata: which ACK key to expect, how to terminate,
// timeout, and whether it blocks the board.
// `stateGetters` is an object giving live access to values like current
// duration, so TTEST timeouts scale with the configured pulse length.
export function getCommandMetadata(cmd, opts = {}, stateGetters = {}) {
    const DEFAULT_TIMEOUT = 2000;

    if (cmd === 'STATUS' || cmd === 'PING:LIST') {
        return {
            expectAckKey: null,
            terminator: 'idle',
            idleMs: 150,
            timeoutMs: 3000,
            blocking: false,
        };
    }

    if (cmd === 'TEST1') {
        return {
            expectAckKey: 'TEST1',
            terminator: 'msg:Relay 1 test complete',
            timeoutMs: 5000,
            blocking: true,
            blockingReason: 'Testing Relay 1 (3 s)...',
            blockingMs: 3000,
        };
    }
    if (cmd === 'TEST2') {
        return {
            expectAckKey: 'TEST2',
            terminator: 'msg:Relay 2 test complete',
            timeoutMs: 5000,
            blocking: true,
            blockingReason: 'Testing Relay 2 (3 s)...',
            blockingMs: 3000,
        };
    }
    if (cmd === 'TTEST1') {
        const dur = stateGetters.relay1Duration?.() ?? 10;
        return {
            expectAckKey: 'TTEST1',
            terminator: 'msg:Relay 1 timed test complete',
            timeoutMs: dur * 1000 + 3000,
            blocking: true,
            blockingReason: `Timed test Relay 1 (${dur} s)...`,
            blockingMs: dur * 1000,
        };
    }
    if (cmd === 'TTEST2') {
        const dur = stateGetters.relay2Duration?.() ?? 10;
        return {
            expectAckKey: 'TTEST2',
            terminator: 'msg:Relay 2 timed test complete',
            timeoutMs: dur * 1000 + 3000,
            blocking: true,
            blockingReason: `Timed test Relay 2 (${dur} s)...`,
            blockingMs: dur * 1000,
        };
    }

    // Everything else: expect an ACK with the command's prefix.
    // Command format is either "KEY" or "KEY:value".
    let expectAckKey;
    if (cmd.startsWith('PING:')) {
        // PING:ADD:ip -> ACK key is "PING:ADD"
        // PING:ON -> ACK key is "PING:ON"
        const rest = cmd.slice('PING:'.length);
        const firstColon = rest.indexOf(':');
        expectAckKey = firstColon === -1 ? cmd : cmd.slice(0, cmd.indexOf(':', 'PING:'.length));
    } else {
        const colon = cmd.indexOf(':');
        expectAckKey = colon === -1 ? cmd : cmd.slice(0, colon);
    }

    return {
        expectAckKey,
        terminator: 'ack',
        timeoutMs: opts.timeoutMs ?? DEFAULT_TIMEOUT,
        blocking: false,
    };
}

// Watchword: any line starting with this indicates the board just rebooted.
export const REBOOT_SENTINEL = 'IDS Network Monitor v';
