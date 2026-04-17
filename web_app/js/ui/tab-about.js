import { persistTransportKind } from '../transport/index.js';

const APP_VERSION = '0.1.0';

export default function createTab(root, ctx) {
    const { transport, state } = ctx;
    const expected = state.get().firmware.expected;

    root.innerHTML = `
        <h2>About</h2>
        <img src="assets/tier8-logo.png" alt="Tier8" class="logo" onerror="this.style.display='none'">

        <div class="section">
            <dl class="about">
                <dt>Application</dt><dd>IDS Network Monitor (web)</dd>
                <dt>App version</dt><dd>${APP_VERSION}</dd>
                <dt>Firmware target</dt><dd id="about-fw">—</dd>
                <dt>Transport in use</dt><dd id="about-transport">—</dd>
                <dt>Source</dt>
                <dd><a href="https://github.com/RICG777/ids-network-monitor" target="_blank" rel="noreferrer">github.com/RICG777/ids-network-monitor</a></dd>
            </dl>
        </div>

        <div class="section">
            <h3>Switch transport</h3>
            <p class="hint">Changes apply on reload. Use USB when connecting a real ESP32 over its USB CDC port; use Mock for UI development and demos without hardware.</p>
            <div class="btn-row">
                <button type="button" id="about-use-webserial">Use USB (Web Serial)</button>
                <button type="button" id="about-use-mock" class="secondary">Use Mock</button>
            </div>
            <p class="hint" id="about-webserial-note"></p>
        </div>

        <div class="section">
            <h3>Browser support</h3>
            <p class="hint">Web Serial requires Chrome or Edge (89 or later). Firefox and Safari don't implement it — on those browsers the app falls back to Mock and Connect won't reach a real board.</p>
            <p id="about-browser-status" class="hint"></p>
        </div>

        <div class="section">
            <h3>URL parameters</h3>
            <p class="hint">Useful for dev and debugging:</p>
            <ul class="hint">
                <li><code>?transport=webserial</code> | <code>mock</code> — override transport.</li>
                <li><code>?mock=slow</code> — add 200 ms latency (mock only).</li>
                <li><code>?mock=reboot</code> — emit simulated boot storm 10 s after connect.</li>
                <li><code>?mock=offline</code> — refuse to connect.</li>
                <li><code>?mock=timeout</code> — TEST1 never completes (exercise timeout handling).</li>
            </ul>
        </div>
    `;

    const fwEl       = root.querySelector('#about-fw');
    const tEl        = root.querySelector('#about-transport');
    const browserEl  = root.querySelector('#about-browser-status');
    const wsBtn      = root.querySelector('#about-use-webserial');
    const mockBtn    = root.querySelector('#about-use-mock');
    const wsNoteEl   = root.querySelector('#about-webserial-note');

    const wsSupported = typeof navigator !== 'undefined' && 'serial' in navigator;
    browserEl.textContent = wsSupported
        ? 'Web Serial API: available in this browser.'
        : 'Web Serial API: NOT available. Falling back to Mock transport.';
    if (!wsSupported) {
        wsBtn.disabled = true;
        wsBtn.textContent = 'Use USB (requires Chrome/Edge)';
    }
    wsNoteEl.textContent = wsSupported
        ? 'First connect will prompt Chrome to pick the USB port.'
        : '';

    function switchTransport(kind) {
        persistTransportKind(kind);
        const url = new URL(window.location.href);
        url.searchParams.set('transport', kind);
        window.location.assign(url.toString());
    }
    wsBtn.addEventListener('click', () => switchTransport('webserial'));
    mockBtn.addEventListener('click', () => switchTransport('mock'));

    return {
        update(s) {
            const fw = s.firmware.version ?? '(not connected)';
            fwEl.textContent = `${fw} (expected ${s.firmware.expected})`;
            const label = s.connection.displayName ?? s.connection.transport;
            tEl.textContent = `${label} — ${s.connection.status}`;
        },
        unmount() { root.innerHTML = ''; },
    };
}
