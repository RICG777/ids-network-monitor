export default function createTab(root) {
    root.innerHTML = `
        <h2>Connection</h2>
        <p class="placeholder">Port picker, connect/disconnect, network config, MAC, firmware version check. Phase 3.</p>
    `;
    return {
        update(_state) {},
        unmount() { root.innerHTML = ''; },
    };
}
