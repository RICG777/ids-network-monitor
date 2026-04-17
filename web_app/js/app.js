// Application entry point.
// Wires up: transport creation, state store, log buffer, tab router,
// status bar, blocking banner, periodic STATUS poll, reboot handling.

import { get, update, subscribe, applyStatusLines, clearBoardState } from './state.js';
import { createTransport, pickTransportKind } from './transport/index.js';
import { logBuffer } from './log.js';
import { showBlocking, hideBlocking } from './ui/banner.js';
import { toast } from './ui/toast.js';
import { confirm as confirmDialog } from './ui/confirm.js';

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
const STATUS_POLL_MS = 5000;

// ---------- Transport ----------
const kind = pickTransportKind();
update({ connection: { transport: kind } });
const transport = createTransport({ kind });

// Provide dynamic state accessors to the command-metadata layer so that
// TTEST timeouts scale with current relay duration.
transport.setStateGetters({
    relay1Duration: () => get().relay1.duration ?? 10,
    relay2Duration: () => get().relay2.duration ?? 10,
});

transport.addEventListener('state', (e) => {
    const { status, error, portInfo, displayName } = e.detail;
    const patch = { connection: { status } };
    if (portInfo    !== undefined) patch.connection.portInfo = portInfo;
    if (displayName !== undefined) patch.connection.displayName = displayName;
    update(patch);
    if (status === 'disconnected') clearBoardState();
    if (error) toast(`Connection error: ${error}`, 'err', 5000);
});

transport.addEventListener('blocking', (e) => {
    const { on, reason, durationMs } = e.detail;
    update({ ui: { blocking: !!on, blockingReason: reason ?? '',
                   blockingDurationMs: durationMs ?? 0, blockingStart: Date.now() } });
    if (on) showBlocking({ reason, durationMs });
    else    hideBlocking();
});

transport.addEventListener('message', (e) => {
    logBuffer.append(e.detail);
});

transport.addEventListener('reboot', () => {
    toast('Board rebooted — refreshing state', 'warn', 4000);
    clearBoardState();
    // Re-fetch status after a short grace period so the boot storm can land.
    setTimeout(() => refreshStatus().catch(() => {}), 1500);
});

// ---------- Command convenience ----------

async function sendCommand(cmd, opts = {}) {
    try {
        return await transport.sendCommand(cmd, opts);
    } catch (err) {
        const msg = err?.message ?? String(err);
        if (!opts.silent) toast(msg, 'err', 4500);
        throw err;
    }
}

async function refreshStatus() {
    if (transport.status !== 'connected') return null;
    const result = await transport.sendCommand('STATUS');
    if (result?.status?.length) applyStatusLines(result.status);
    return result;
}

// ---------- Periodic STATUS poll ----------
let pollTimer = null;
function startPolling() { stopPolling(); pollTimer = setInterval(pollTick, STATUS_POLL_MS); }
function stopPolling()  { if (pollTimer) { clearInterval(pollTimer); pollTimer = null; } }

async function pollTick() {
    if (transport.status !== 'connected') return;
    if (get().ui.blocking) return;
    if (document.visibilityState !== 'visible') return;
    try { await refreshStatus(); } catch {}
}

transport.addEventListener('state', (e) => {
    if (e.detail.status === 'connected') startPolling();
    else stopPolling();
});

// ---------- Context object shared with tabs ----------
const ctx = {
    transport,
    logBuffer,
    state: { get, update, subscribe },
    sendCommand,
    refreshStatus,
    confirm: confirmDialog,
    toast,
};

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
        try { currentTab.unmount(); } catch (e) { console.error('tab unmount', e); }
    }

    const factory = TAB_FACTORIES[tabId];
    if (!factory) {
        panelEl.innerHTML = `<p class="placeholder">Unknown tab: ${tabId}</p>`;
        currentTab = null;
        currentTabId = null;
        return;
    }
    currentTab = factory(panelEl, ctx);
    currentTabId = tabId;
    unsubCurrent = subscribe((state) => {
        try { currentTab.update(state); } catch (e) { console.error('tab update', e); }
    });

    for (const btn of tabsEl.querySelectorAll('.tab')) {
        btn.classList.toggle('active', btn.dataset.tab === tabId);
    }
    try { localStorage.setItem('idsnm.ui.lastTab', tabId); } catch {}
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

    const label = state.connection.displayName ?? state.connection.transport;
    statusTransport.textContent = label;
    statusBoardIP.textContent   = `Board IP: ${state.network.ip ?? '—'}`;
    statusFW.textContent        = `FW: ${state.firmware.version ?? '—'}`;
});

// ---------- Boot ----------
const startTab = localStorage.getItem('idsnm.ui.lastTab') || DEFAULT_TAB;
activate(TAB_FACTORIES[startTab] ? startTab : DEFAULT_TAB);

// Stash for debugging / console tinkering
window.__idsnm = { get, update, transport, logBuffer, sendCommand, refreshStatus, ctx };
