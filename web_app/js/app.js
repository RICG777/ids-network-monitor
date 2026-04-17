// Application entry point.
// Wires up: transport creation, state store, tab router, status bar.
// Real command flows arrive in Phase 1+.

import { get, update, subscribe } from './state.js';
import { createTransport, pickTransportKind } from './transport/index.js';

import createConnectionTab from './ui/tab-connection.js';
import createRelaysTab     from './ui/tab-relays.js';
import createPingTab       from './ui/tab-ping.js';
import createEventsTab     from './ui/tab-events.js';
import createAboutTab      from './ui/tab-about.js';

const TAB_FACTORIES = {
    connection: createConnectionTab,
    relays:     createRelaysTab,
    ping:       createPingTab,
    events:     createEventsTab,
    about:      createAboutTab,
};

const DEFAULT_TAB = 'connection';

// ---------- Transport ----------

const kind = pickTransportKind();
update({ connection: { transport: kind } });
const transport = createTransport({ kind });

transport.addEventListener('state', (e) => {
    const { status } = e.detail;
    update({ connection: { status } });
});

// ---------- Tab router ----------

const panelEl = document.getElementById('panel');
const tabsEl  = document.getElementById('tabs');

let currentTab    = null;
let currentTabId  = null;
let unsubCurrent  = null;

function activate(tabId) {
    if (currentTabId === tabId) return;

    if (currentTab) {
        if (unsubCurrent) { unsubCurrent(); unsubCurrent = null; }
        currentTab.unmount();
    }

    const factory = TAB_FACTORIES[tabId];
    if (!factory) {
        panelEl.innerHTML = `<p class="placeholder">Unknown tab: ${tabId}</p>`;
        return;
    }
    currentTab = factory(panelEl);
    currentTabId = tabId;
    unsubCurrent = subscribe((state) => currentTab.update(state));

    for (const btn of tabsEl.querySelectorAll('.tab')) {
        btn.classList.toggle('active', btn.dataset.tab === tabId);
    }
    try {
        localStorage.setItem('idsnm.ui.lastTab', tabId);
    } catch {}
}

tabsEl.addEventListener('click', (e) => {
    const btn = e.target.closest('.tab');
    if (btn) activate(btn.dataset.tab);
});

// ---------- Status bar ----------

const statusConn      = document.getElementById('status-connection');
const statusTransport = document.getElementById('status-transport');
const statusBoardIP   = document.getElementById('status-board-ip');
const statusFW        = document.getElementById('status-firmware');

subscribe((state) => {
    const s = state.connection.status;
    statusConn.textContent = s.charAt(0).toUpperCase() + s.slice(1);
    statusConn.className = s === 'connected' ? 'connected'
                         : s === 'error' ? 'error' : 'disconnected';

    statusTransport.textContent = state.connection.transport;
    statusBoardIP.textContent   = `Board IP: ${state.network.ip ?? '—'}`;
    statusFW.textContent        = `FW: ${state.firmware.version ?? '—'}`;
});

// ---------- Boot ----------

const startTab = localStorage.getItem('idsnm.ui.lastTab') || DEFAULT_TAB;
activate(TAB_FACTORIES[startTab] ? startTab : DEFAULT_TAB);

// Stash for debugging
window.__idsnm = { get, update, transport };
