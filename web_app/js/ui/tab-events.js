import { logBuffer } from '../log.js';

function formatTime(t) {
    const d = new Date(t);
    const hh = String(d.getHours()).padStart(2, '0');
    const mm = String(d.getMinutes()).padStart(2, '0');
    const ss = String(d.getSeconds()).padStart(2, '0');
    const ms = String(d.getMilliseconds()).padStart(3, '0');
    return `${hh}:${mm}:${ss}.${ms}`;
}

function typeLabel(type) {
    switch (type) {
        case 'ack':    return 'ACK';
        case 'err':    return 'ERR';
        case 'status': return 'STATUS';
        case 'msg':    return 'MSG';
        default:       return 'RAW';
    }
}

export default function createTab(root, _ctx = {}) {
    root.innerHTML = `
        <div class="log-header">
            <h2>Event Log</h2>
            <div class="log-controls">
                <label class="filter-field"><input type="text" id="log-search" placeholder="Search…"></label>
                <label class="filter-toggle"><input type="checkbox" id="log-filter-ack" checked> ACK</label>
                <label class="filter-toggle"><input type="checkbox" id="log-filter-err" checked> ERR</label>
                <label class="filter-toggle"><input type="checkbox" id="log-filter-msg" checked> MSG</label>
                <label class="filter-toggle"><input type="checkbox" id="log-filter-status"> STATUS</label>
                <label class="filter-toggle"><input type="checkbox" id="log-hide-syslog" checked> Hide Syslog debug</label>
                <button type="button" id="log-download">Download</button>
                <button type="button" id="log-clear">Clear</button>
            </div>
        </div>
        <div id="log-view" class="log-view" role="log" aria-live="polite"></div>
        <p id="log-meta" class="log-meta">0 lines shown.</p>
    `;

    const view     = root.querySelector('#log-view');
    const metaEl   = root.querySelector('#log-meta');
    const searchEl = root.querySelector('#log-search');
    const flt = {
        ack:    root.querySelector('#log-filter-ack'),
        err:    root.querySelector('#log-filter-err'),
        msg:    root.querySelector('#log-filter-msg'),
        status: root.querySelector('#log-filter-status'),
        hideSyslog: root.querySelector('#log-hide-syslog'),
    };
    const clearBtn = root.querySelector('#log-clear');
    const dlBtn    = root.querySelector('#log-download');

    // Persisted filter state
    try {
        const saved = JSON.parse(localStorage.getItem('idsnm.ui.log.filters') || 'null');
        if (saved) {
            flt.ack.checked    = !!saved.ack;
            flt.err.checked    = !!saved.err;
            flt.msg.checked    = !!saved.msg;
            flt.status.checked = !!saved.status;
            flt.hideSyslog.checked = saved.hideSyslog !== false;
            searchEl.value     = saved.query ?? '';
        }
    } catch {}

    const saveFilters = () => {
        try {
            localStorage.setItem('idsnm.ui.log.filters', JSON.stringify({
                ack:    flt.ack.checked,
                err:    flt.err.checked,
                msg:    flt.msg.checked,
                status: flt.status.checked,
                hideSyslog: flt.hideSyslog.checked,
                query:  searchEl.value,
            }));
        } catch {}
    };

    let autoScroll = true;
    view.addEventListener('scroll', () => {
        autoScroll = (view.scrollTop + view.clientHeight >= view.scrollHeight - 4);
    });

    let shownCount = 0;

    function passes(entry) {
        const typeOn = ({ ack: flt.ack.checked, err: flt.err.checked,
                          msg: flt.msg.checked, status: flt.status.checked,
                          raw: true })[entry.type];
        if (!typeOn) return false;
        if (flt.hideSyslog.checked && entry.type === 'msg' && entry.value.startsWith('Syslog (')) {
            return false;
        }
        const q = searchEl.value.trim().toLowerCase();
        if (q && !entry.raw.toLowerCase().includes(q)) return false;
        return true;
    }

    function renderRow(entry) {
        const row = document.createElement('div');
        row.className = `log-row log-${entry.type}`;
        row.dataset.seq = entry.seq;
        const time = document.createElement('span');
        time.className = 'log-time'; time.textContent = formatTime(entry.t);
        const type = document.createElement('span');
        type.className = 'log-type'; type.textContent = typeLabel(entry.type);
        const txt  = document.createElement('span');
        txt.className = 'log-raw'; txt.textContent = entry.raw;
        row.append(time, type, txt);
        return row;
    }

    function renderAll() {
        view.innerHTML = '';
        shownCount = 0;
        for (const entry of logBuffer.all()) {
            if (passes(entry)) {
                view.appendChild(renderRow(entry));
                shownCount++;
            }
        }
        if (autoScroll) view.scrollTop = view.scrollHeight;
        updateMeta();
    }

    function updateMeta() {
        metaEl.textContent = `${shownCount} shown / ${logBuffer.entries.length} total (auto-scroll ${autoScroll ? 'on' : 'off'}).`;
    }

    function onAppend(e) {
        const entry = e.detail;
        if (!passes(entry)) { updateMeta(); return; }
        view.appendChild(renderRow(entry));
        shownCount++;
        if (autoScroll) view.scrollTop = view.scrollHeight;
        // Trim rendered rows once we exceed capacity — keep DOM light
        while (view.children.length > logBuffer.max) view.removeChild(view.firstChild);
        updateMeta();
    }
    function onClear() { renderAll(); }

    logBuffer.addEventListener('append', onAppend);
    logBuffer.addEventListener('clear',  onClear);

    for (const el of Object.values(flt)) {
        el.addEventListener('change', () => { saveFilters(); renderAll(); });
    }
    searchEl.addEventListener('input', () => { saveFilters(); renderAll(); });

    clearBtn.addEventListener('click', () => logBuffer.clear());

    dlBtn.addEventListener('click', () => {
        const text = logBuffer.all()
            .map(e => `${formatTime(e.t)}  ${typeLabel(e.type).padEnd(6)} ${e.raw}`)
            .join('\n');
        const blob = new Blob([text], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        const ts = new Date().toISOString().replace(/[:.]/g, '-').slice(0, 19);
        a.href = url; a.download = `ids-log-${ts}.txt`;
        document.body.appendChild(a); a.click(); a.remove();
        setTimeout(() => URL.revokeObjectURL(url), 1000);
    });

    renderAll();

    return {
        update(_state) {},
        unmount() {
            logBuffer.removeEventListener('append', onAppend);
            logBuffer.removeEventListener('clear',  onClear);
            root.innerHTML = '';
        },
    };
}
