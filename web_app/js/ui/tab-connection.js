import { WebSerialTransport } from '../transport/index.js';
import { isValidHostIP, isValidGateway, isValidSubnet } from '../validators.js';

function el(html) {
    const t = document.createElement('template');
    t.innerHTML = html.trim();
    return t.content.firstElementChild;
}

export default function createTab(root, ctx) {
    const { transport, sendCommand, refreshStatus, confirm, toast, state } = ctx;
    const isWebSerial = transport.kind === 'webserial';
    const isMock = transport.kind === 'mock';

    root.innerHTML = `
        <h2>Connection</h2>

        <div class="section">
            <h3>Transport</h3>
            <p class="hint" id="conn-transport-hint"></p>
            <div class="btn-row" id="conn-link-row">
                <button type="button" id="btn-connect">Connect</button>
                <button type="button" id="btn-disconnect" class="secondary" hidden>Disconnect</button>
                <button type="button" id="btn-request-port" class="secondary" hidden>Pick different port...</button>
            </div>
            <p class="port-info" id="conn-port-info" hidden></p>
            <div class="fw-warn" id="conn-fw-warn" hidden></div>
        </div>

        <div class="section">
            <h3>Identity</h3>
            <dl class="about">
                <dt>MAC address</dt><dd id="conn-mac">—</dd>
                <dt>Firmware version</dt><dd id="conn-fw">—</dd>
            </dl>
        </div>

        <div class="section">
            <h3>Network configuration</h3>
            <p class="hint">Changes take effect immediately. The board stores values in NVS and re-binds UDP. The USB serial link is unaffected by IP changes.</p>
            <div class="form-grid">
                <label for="conn-ip">IP address</label>
                <input type="text" id="conn-ip" placeholder="192.168.68.142" autocomplete="off">
                <label for="conn-subnet">Subnet mask</label>
                <input type="text" id="conn-subnet" placeholder="255.255.255.0" autocomplete="off">
                <label for="conn-gateway">Gateway</label>
                <input type="text" id="conn-gateway" placeholder="0.0.0.0 (none)" autocomplete="off">
            </div>
            <div class="btn-row">
                <button type="button" id="btn-write-network" disabled>Write network config</button>
                <button type="button" id="btn-refresh" class="secondary" disabled>Refresh</button>
            </div>
        </div>
    `;

    // --- Transport hint ---
    const transportHint = root.querySelector('#conn-transport-hint');
    if (isMock) {
        transportHint.innerHTML =
            `Transport: <span class="pill warn">mock</span> — simulated firmware, no hardware attached. ` +
            `To talk to a real board, reload with <code>?transport=webserial</code>.`;
    } else if (isWebSerial) {
        if (!WebSerialTransport.isSupported()) {
            transportHint.innerHTML =
                `<span class="pill err">Web Serial not supported</span> — this browser doesn't expose ` +
                `<code>navigator.serial</code>. Use Chrome or Edge 89+.`;
        } else {
            transportHint.innerHTML =
                `Transport: <span class="pill">USB (Web Serial)</span> — connect over USB CDC at 115200 8N1. ` +
                `First connect prompts Chrome's port picker; later connects reuse the granted port.`;
        }
    }

    // --- Element refs ---
    const btnConnect    = root.querySelector('#btn-connect');
    const btnDisconnect = root.querySelector('#btn-disconnect');
    const btnReqPort    = root.querySelector('#btn-request-port');
    const portInfoEl    = root.querySelector('#conn-port-info');
    const fwWarnEl      = root.querySelector('#conn-fw-warn');
    const macEl         = root.querySelector('#conn-mac');
    const fwEl          = root.querySelector('#conn-fw');
    const ipEl          = root.querySelector('#conn-ip');
    const subnetEl      = root.querySelector('#conn-subnet');
    const gatewayEl     = root.querySelector('#conn-gateway');
    const btnWriteNet   = root.querySelector('#btn-write-network');
    const btnRefresh    = root.querySelector('#btn-refresh');

    // --- Connect / disconnect ---

    async function doConnect(forcePicker = false) {
        try {
            let port = null;
            if (isWebSerial) {
                if (!WebSerialTransport.isSupported()) {
                    toast('Web Serial not supported in this browser', 'err', 4000);
                    return;
                }
                if (forcePicker) {
                    port = await WebSerialTransport.requestPort();
                }
            }
            await transport.connect(port ? { port } : {});
            // After connect, immediately refresh status
            try { await refreshStatus(); } catch (e) { /* surface via toast elsewhere */ }
            toast('Connected', 'ok', 2000);
        } catch (err) {
            // Benign case: user cancelled the port picker
            if (err?.name === 'NotFoundError' || /no port selected/i.test(err?.message ?? '')) {
                return;
            }
            toast(`Connect failed: ${err?.message ?? err}`, 'err', 5000);
        }
    }
    async function doDisconnect() {
        try { await transport.disconnect(); } catch {}
        toast('Disconnected', 'info', 1500);
    }
    btnConnect.addEventListener('click', () => doConnect(false));
    btnReqPort.addEventListener('click', async () => {
        // Disconnect first if currently connected
        if (transport.status === 'connected') await doDisconnect();
        await doConnect(true);
    });
    btnDisconnect.addEventListener('click', doDisconnect);

    // --- Form validation ---

    function updateWriteButton() {
        if (transport.status !== 'connected') {
            btnWriteNet.disabled = true;
            return;
        }
        const ipOk  = isValidHostIP(ipEl.value.trim());
        const snOk  = isValidSubnet(subnetEl.value.trim());
        const gwOk  = isValidGateway(gatewayEl.value.trim());
        ipEl.classList.toggle('invalid', ipEl.value.trim() !== '' && !ipOk);
        subnetEl.classList.toggle('invalid', subnetEl.value.trim() !== '' && !snOk);
        gatewayEl.classList.toggle('invalid', gatewayEl.value.trim() !== '' && !gwOk);
        btnWriteNet.disabled = !(ipOk && snOk && gwOk);
    }
    [ipEl, subnetEl, gatewayEl].forEach(inp => inp.addEventListener('input', updateWriteButton));

    // --- Write handlers ---

    btnWriteNet.addEventListener('click', async () => {
        const ip = ipEl.value.trim();
        const sn = subnetEl.value.trim();
        const gw = gatewayEl.value.trim();
        const current = state.get().network;
        const diff = [];
        if (ip !== current.ip)      diff.push(`<li>IP: <code>${current.ip ?? '—'}</code> → <code>${ip}</code></li>`);
        if (sn !== current.subnet)  diff.push(`<li>Subnet: <code>${current.subnet ?? '—'}</code> → <code>${sn}</code></li>`);
        if (gw !== current.gateway) diff.push(`<li>Gateway: <code>${current.gateway ?? '—'}</code> → <code>${gw}</code></li>`);
        if (diff.length === 0) { toast('Nothing to write — values match board', 'info', 2500); return; }

        const ok = await confirm({
            title: 'Write network config?',
            body:
                `<ul>${diff.join('')}</ul>` +
                `<p class="hint">This changes the board's Ethernet IP. Syslog senders pointing at ` +
                `the old IP will stop reaching the board until reconfigured. ` +
                `The USB serial link (this session) is unaffected.</p>`,
            okLabel: 'Write',
            danger: true,
        });
        if (!ok) return;

        // Send the three commands sequentially. Transport queues them anyway.
        btnWriteNet.disabled = true;
        try {
            if (ip !== current.ip)      await sendCommand(`IP:${ip}`);
            if (sn !== current.subnet)  await sendCommand(`SUBNET:${sn}`);
            if (gw !== current.gateway) await sendCommand(`GATEWAY:${gw}`);
            await refreshStatus();
            toast('Network config written', 'ok');
        } catch (err) {
            // toast already fired via sendCommand wrapper
        } finally {
            updateWriteButton();
        }
    });

    btnRefresh.addEventListener('click', async () => {
        try {
            await refreshStatus();
            toast('Refreshed', 'ok', 1200);
        } catch {}
    });

    // --- Render on state change ---

    function update(s) {
        const connected = s.connection.status === 'connected';
        btnConnect.hidden    = connected;
        btnDisconnect.hidden = !connected;
        btnReqPort.hidden    = !isWebSerial;
        btnReqPort.textContent = connected ? 'Pick different port...' : 'Pick port...';

        btnRefresh.disabled  = !connected;

        // Port info
        if (isWebSerial && s.connection.portInfo) {
            portInfoEl.hidden = false;
            portInfoEl.textContent = `Port: ${s.connection.portInfo}`;
        } else if (isMock) {
            portInfoEl.hidden = false;
            portInfoEl.textContent = 'Simulated firmware (no port)';
        } else {
            portInfoEl.hidden = true;
        }

        macEl.textContent = s.network.mac ?? '—';
        fwEl.textContent  = s.firmware.version
            ? `${s.firmware.version} (expected ${s.firmware.expected})`
            : '—';

        // Firmware mismatch warning (dismissible per version)
        if (s.firmware.mismatch && !s.firmware.mismatchDismissed) {
            fwWarnEl.hidden = false;
            fwWarnEl.innerHTML = `
                <strong>Firmware mismatch:</strong> board reports v${s.firmware.version},
                app expects v${s.firmware.expected}. Some features may misbehave.
                <button type="button" class="secondary" id="fw-dismiss" style="margin-left:0.5rem">Dismiss</button>
            `;
            const dismissBtn = fwWarnEl.querySelector('#fw-dismiss');
            dismissBtn.addEventListener('click', () => {
                try { localStorage.setItem('idsnm.fwMismatchDismissed', s.firmware.version); } catch {}
                state.update({ firmware: { mismatchDismissed: true } });
            });
        } else {
            fwWarnEl.hidden = true;
        }

        // Pre-fill network form from board state (only if the input isn't focused
        // — don't overwrite typing from under the user)
        if (document.activeElement !== ipEl)      ipEl.value      = s.network.ip      ?? '';
        if (document.activeElement !== subnetEl)  subnetEl.value  = s.network.subnet  ?? '';
        if (document.activeElement !== gatewayEl) gatewayEl.value = s.network.gateway ?? '';
        for (const inp of [ipEl, subnetEl, gatewayEl]) inp.disabled = !connected;
        updateWriteButton();
    }

    return {
        update,
        unmount() { root.innerHTML = ''; },
    };
}
