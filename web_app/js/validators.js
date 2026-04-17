// Shared client-side validators. Firmware still validates server-side —
// these are UX polish (disable submit until valid, inline red borders).

export function isValidIPv4(s) {
    if (typeof s !== 'string') return false;
    const m = s.match(/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/);
    if (!m) return false;
    for (let i = 1; i <= 4; i++) {
        const n = Number(m[i]);
        if (!Number.isFinite(n) || n < 0 || n > 255) return false;
    }
    return true;
}

// Reject all-zero and broadcast as invalid host/subnet — the firmware
// treats these as invalid too (see isValidIP() in syslog_mon_v2e.ino).
export function isValidHostIP(s) {
    if (!isValidIPv4(s)) return false;
    const p = s.split('.').map(Number);
    if (p[0] === 0 && p[1] === 0 && p[2] === 0 && p[3] === 0) return false;
    if (p[0] === 255 && p[1] === 255 && p[2] === 255 && p[3] === 255) return false;
    return true;
}

// Gateway may be 0.0.0.0 (meaning "no gateway"). Otherwise valid IPv4.
export function isValidGateway(s) {
    if (!isValidIPv4(s)) return false;
    return true;
}

// Subnet mask: must be a valid IPv4 pattern. Stricter contiguous-bit check
// is nice-to-have; firmware doesn't enforce it either, so keep lax.
export function isValidSubnet(s) {
    return isValidIPv4(s);
}

// Trigger string: 1..119 chars, no newlines.
export function isValidTriggerString(s) {
    if (typeof s !== 'string') return false;
    if (s.length < 1 || s.length > 119) return false;
    if (/[\r\n]/.test(s)) return false;
    return true;
}

// Duration in seconds, 1..255.
export function isValidDuration(n) {
    const v = Number(n);
    return Number.isInteger(v) && v >= 1 && v <= 255;
}

// Ping interval: 5..255 s.
export function isValidPingInterval(n) {
    const v = Number(n);
    return Number.isInteger(v) && v >= 5 && v <= 255;
}

// Ping threshold: 1..255 missed pings.
export function isValidPingThreshold(n) {
    const v = Number(n);
    return Number.isInteger(v) && v >= 1 && v <= 255;
}

export function isValidVerbosity(n) {
    const v = Number(n);
    return v === 0 || v === 1 || v === 2;
}

export function isValidPingMode(s) {
    return s === 'AUTO' || s === 'LATCH' || s === 'PULSE';
}

export function isValidRelayMode(s) {
    return s === 'PULSE' || s === 'LATCH';
}

// Normalise a user-pasted trigger string: strip CR/LF to protect the
// line-based wire protocol.
export function normaliseTriggerInput(s) {
    if (typeof s !== 'string') return '';
    return s.replace(/[\r\n]+/g, ' ').trim();
}
