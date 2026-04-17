export default function createTab(root) {
    root.innerHTML = `
        <h2>Ping Config</h2>
        <p class="placeholder">Enable/interval/threshold/relay/mode/duration + target list (up to 20) with per-target OK/FAIL status. Phase 5.</p>
    `;
    return {
        update(_state) {},
        unmount() { root.innerHTML = ''; },
    };
}
