import {
    isValidHostIP, isValidPingInterval, isValidPingThreshold,
    isValidDuration, isValidPingMode
} from '../validators.js';

export default function createTab(root, ctx) {
    const { sendCommand, refreshStatus, confirm, toast, state } = ctx;

    root.innerHTML = `
        <h2>Ping Config <span class="pill" id="ping-rstate">—</span></h2>
        <p class="hint">ICMP heartbeat. Up to 20 targets. If any target misses the threshold consecutively, the chosen relay energises. Independent from syslog triggers.</p>

        <div class="section">
            <h3>Global settings</h3>
            <div class="form-grid">
                <label for="ping-enabled">Enabled</label>
                <label class="filter-toggle">
                    <input type="checkbox" id="ping-enabled"> <span id="ping-enabled-label">Off</span>
                </label>
                <label for="ping-interval">Interval (s)</label>
                <input type="number" id="ping-interval" min="5" max="255" step="1">
                <label for="ping-threshold">Fail threshold</label>
                <input type="number" id="ping-threshold" min="1" max="255" step="1">
                <label for="ping-relay">Target relay</label>
                <select id="ping-relay">
                    <option value="1">Relay 1</option>
                    <option value="2">Relay 2</option>
                </select>
                <label for="ping-mode">Mode</label>
                <select id="ping-mode">
                    <option value="AUTO">AUTO (relay clears when targets recover)</option>
                    <option value="LATCH">LATCH (relay stays on until manual reset)</option>
                    <option value="PULSE">PULSE (relay clears after duration)</option>
                </select>
                <label for="ping-duration">Duration (s)</label>
                <input type="number" id="ping-duration" min="1" max="255" step="1">
            </div>
            <div class="btn-row">
                <button type="button" id="ping-write" disabled>Write settings</button>
                <button type="button" id="ping-refresh" class="secondary" disabled>Refresh</button>
                <button type="button" id="ping-reset-fails" class="secondary" disabled>Reset fails</button>
            </div>
        </div>

        <div class="section">
            <h3>Targets <span class="pill" id="ping-count">0 / 20</span></h3>
            <div class="btn-row">
                <input type="text" id="ping-new-ip" placeholder="192.168.1.10" autocomplete="off" style="width:14rem">
                <button type="button" id="ping-add" disabled>Add target</button>
                <button type="button" id="ping-clear" class="danger" disabled>Clear all</button>
            </div>
            <table class="data" id="ping-targets">
                <thead>
                    <tr><th>IP</th><th>Status</th><th>Fails</th><th></th></tr>
                </thead>
                <tbody id="ping-targets-body">
                    <tr><td colspan="4" class="hint">No targets configured.</td></tr>
                </tbody>
            </table>
        </div>
    `;

    // --- Element refs ---
    const rstatePill    = root.querySelector('#ping-rstate');
    const enabledCb     = root.querySelector('#ping-enabled');
    const enabledLabel  = root.querySelector('#ping-enabled-label');
    const intervalEl    = root.querySelector('#ping-interval');
    const thresholdEl   = root.querySelector('#ping-threshold');
    const relayEl       = root.querySelector('#ping-relay');
    const modeEl        = root.querySelector('#ping-mode');
    const durationEl    = root.querySelector('#ping-duration');
    const btnWrite      = root.querySelector('#ping-write');
    const btnRefresh    = root.querySelector('#ping-refresh');
    const btnResetFails = root.querySelector('#ping-reset-fails');
    const countPill     = root.querySelector('#ping-count');
    const newIpEl       = root.querySelector('#ping-new-ip');
    const btnAdd        = root.querySelector('#ping-add');
    const btnClear      = root.querySelector('#ping-clear');
    const targetsBody   = root.querySelector('#ping-targets-body');

    // --- Validation / diff for Write button ---
    function boardCfg() {
        const p = state.get().ping;
        return {
            enabled:   !!p.enabled,
            interval:  p.interval ?? 30,
            threshold: p.threshold ?? 3,
            relay:     p.relay ?? 1,
            mode:      p.mode ?? 'AUTO',
            duration:  p.duration ?? 10,
        };
    }

    function refreshWriteBtn() {
        const connected = state.get().connection.status === 'connected';
        const blocking  = state.get().ui.blocking;
        const intOk  = isValidPingInterval(Number(intervalEl.value));
        const thrOk  = isValidPingThreshold(Number(thresholdEl.value));
        const durOk  = isValidDuration(Number(durationEl.value));
        const modeOk = isValidPingMode(modeEl.value);
        intervalEl.classList.toggle('invalid',  intervalEl.value !== '' && !intOk);
        thresholdEl.classList.toggle('invalid', thresholdEl.value !== '' && !thrOk);
        durationEl.classList.toggle('invalid',  durationEl.value !== '' && !durOk);
        const cfg = boardCfg();
        const changed =
            enabledCb.checked  !== cfg.enabled ||
            Number(intervalEl.value)  !== cfg.interval  ||
            Number(thresholdEl.value) !== cfg.threshold ||
            Number(relayEl.value)     !== cfg.relay     ||
            modeEl.value              !== cfg.mode      ||
            Number(durationEl.value)  !== cfg.duration;
        btnWrite.disabled = !connected || blocking || !(intOk && thrOk && durOk && modeOk) || !changed;
        enabledLabel.textContent = enabledCb.checked ? 'On' : 'Off';
    }

    [enabledCb, intervalEl, thresholdEl, relayEl, modeEl, durationEl].forEach(el => {
        el.addEventListener('input',  refreshWriteBtn);
        el.addEventListener('change', refreshWriteBtn);
    });

    // --- Write global settings ---
    btnWrite.addEventListener('click', async () => {
        const cfg = boardCfg();
        const newCfg = {
            enabled:   enabledCb.checked,
            interval:  Number(intervalEl.value),
            threshold: Number(thresholdEl.value),
            relay:     Number(relayEl.value),
            mode:      modeEl.value,
            duration:  Number(durationEl.value),
        };
        const commands = [];
        if (newCfg.interval  !== cfg.interval)  commands.push(`PING:INT:${newCfg.interval}`);
        if (newCfg.threshold !== cfg.threshold) commands.push(`PING:THR:${newCfg.threshold}`);
        if (newCfg.relay     !== cfg.relay)     commands.push(`PING:RELAY:${newCfg.relay}`);
        if (newCfg.mode      !== cfg.mode)      commands.push(`PING:MODE:${newCfg.mode}`);
        if (newCfg.duration  !== cfg.duration)  commands.push(`PING:DUR:${newCfg.duration}`);
        if (newCfg.enabled   !== cfg.enabled)   commands.push(newCfg.enabled ? 'PING:ON' : 'PING:OFF');
        if (commands.length === 0) { toast('Nothing to write', 'info', 1500); return; }
        btnWrite.disabled = true;
        try {
            for (const c of commands) await sendCommand(c);
            await refreshStatus();
            toast('Ping settings written', 'ok');
        } catch {} finally { refreshWriteBtn(); }
    });

    btnRefresh.addEventListener('click', async () => {
        try { await refreshStatus(); toast('Refreshed', 'ok', 1200); } catch {}
    });
    btnResetFails.addEventListener('click', async () => {
        try { await sendCommand('PING:RESET'); toast('Fails reset', 'ok', 1500); await refreshStatus(); }
        catch {}
    });

    // --- Add / delete / clear ---
    function refreshAddBtn() {
        const connected = state.get().connection.status === 'connected';
        const cnt = state.get().ping.count ?? 0;
        const ok = isValidHostIP(newIpEl.value.trim());
        newIpEl.classList.toggle('invalid', newIpEl.value.trim() !== '' && !ok);
        btnAdd.disabled = !connected || !ok || cnt >= 20;
    }
    newIpEl.addEventListener('input', refreshAddBtn);
    newIpEl.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' && !btnAdd.disabled) { e.preventDefault(); btnAdd.click(); }
    });
    btnAdd.addEventListener('click', async () => {
        const ip = newIpEl.value.trim();
        btnAdd.disabled = true;
        try {
            await sendCommand(`PING:ADD:${ip}`);
            newIpEl.value = '';
            await refreshStatus();
            toast(`Added ${ip}`, 'ok', 1500);
        } catch {} finally { refreshAddBtn(); }
    });
    btnClear.addEventListener('click', async () => {
        const cnt = state.get().ping.count ?? 0;
        const ok = await confirm({
            title: 'Clear all ping targets?',
            body: `<p>This removes all ${cnt} targets and clears the ping relay. Proceed?</p>`,
            okLabel: 'Clear all',
            danger: true,
        });
        if (!ok) return;
        try { await sendCommand('PING:CLR'); await refreshStatus(); toast('Targets cleared', 'ok'); } catch {}
    });

    // --- Render ---
    function update(s) {
        const connected = s.connection.status === 'connected';
        const blocking  = s.ui.blocking;
        const cfg = boardCfg();

        // Form values (respect focus)
        if (document.activeElement !== enabledCb)   enabledCb.checked    = cfg.enabled;
        if (document.activeElement !== intervalEl)  intervalEl.value     = cfg.interval;
        if (document.activeElement !== thresholdEl) thresholdEl.value    = cfg.threshold;
        if (document.activeElement !== relayEl)     relayEl.value        = String(cfg.relay);
        if (document.activeElement !== modeEl)      modeEl.value         = cfg.mode;
        if (document.activeElement !== durationEl)  durationEl.value     = cfg.duration;

        enabledLabel.textContent = enabledCb.checked ? 'On' : 'Off';

        // Enable/disable
        for (const el of [enabledCb, intervalEl, thresholdEl, relayEl, modeEl, durationEl, newIpEl]) {
            el.disabled = !connected || blocking;
        }
        btnRefresh.disabled    = !connected;
        btnResetFails.disabled = !connected || blocking;

        // RSTATE pill
        const rstate = s.ping.rstate;
        rstatePill.textContent = rstate === 'ON' ? 'Relay ON' : (rstate === 'OFF' ? 'Relay OFF' : '—');
        rstatePill.classList.toggle('err', rstate === 'ON');
        rstatePill.classList.toggle('ok',  rstate === 'OFF');

        // Target count
        const cnt = s.ping.targets.length;
        countPill.textContent = `${cnt} / 20`;

        // Targets table — render from scratch (small list)
        if (cnt === 0) {
            targetsBody.innerHTML = `<tr><td colspan="4" class="hint">No targets configured.</td></tr>`;
        } else {
            const rows = s.ping.targets.map(t => `
                <tr>
                    <td><code>${escapeHtml(t.ip)}</code></td>
                    <td><span class="pill ${t.state === 'OK' ? 'ok' : 'err'}">${escapeHtml(t.state)}</span></td>
                    <td>${t.fails}</td>
                    <td><button type="button" class="secondary" data-del="${escapeHtml(t.ip)}" ${(!connected || blocking) ? 'disabled' : ''}>Delete</button></td>
                </tr>
            `).join('');
            targetsBody.innerHTML = rows;
            for (const btn of targetsBody.querySelectorAll('[data-del]')) {
                btn.addEventListener('click', async () => {
                    const ip = btn.dataset.del;
                    btn.disabled = true;
                    try { await sendCommand(`PING:DEL:${ip}`); await refreshStatus(); toast(`Removed ${ip}`, 'ok', 1500); } catch {}
                });
            }
        }

        btnClear.disabled = !connected || blocking || cnt === 0;
        refreshAddBtn();
        refreshWriteBtn();
    }

    return {
        update,
        unmount() { root.innerHTML = ''; },
    };
}

function escapeHtml(s) {
    return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;').replace(/'/g, '&#39;');
}
