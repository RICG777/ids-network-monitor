const APP_VERSION = '0.1.0-dev';

export default function createTab(root) {
    root.innerHTML = `
        <h2>About</h2>
        <dl class="about">
            <dt>Application</dt><dd>IDS Network Monitor (web)</dd>
            <dt>App version</dt><dd>${APP_VERSION}</dd>
            <dt>Firmware target</dt><dd id="about-fw">—</dd>
            <dt>Source</dt><dd><a href="https://github.com/RICG777/ids-network-monitor" target="_blank" rel="noreferrer">github.com/RICG777/ids-network-monitor</a></dd>
        </dl>
    `;
    const fwEl = root.querySelector('#about-fw');
    return {
        update(state) {
            const fw = state.firmware.version ?? 'not connected';
            const expected = state.firmware.expected;
            fwEl.textContent = `${fw} (expected ${expected})`;
        },
        unmount() { root.innerHTML = ''; },
    };
}
