// Small toast helper. Mount a host div once; append + auto-remove.

let host = null;

function ensureHost() {
    if (host && document.body.contains(host)) return host;
    host = document.createElement('div');
    host.className = 'toast-host';
    document.body.appendChild(host);
    return host;
}

export function toast(message, kind = 'info', ms = 3200) {
    const h = ensureHost();
    const el = document.createElement('div');
    el.className = `toast ${kind}`;
    el.textContent = message;
    h.appendChild(el);
    setTimeout(() => {
        el.style.opacity = '0';
        el.style.transition = 'opacity 0.2s';
        setTimeout(() => el.remove(), 220);
    }, ms);
}
