export default function createTab(root) {
    root.innerHTML = `
        <h2>Relay Config</h2>
        <p class="placeholder">Per-relay triggers, mode (Pulse/Latch), duration, reset, Quick Test, Timed Test. Phase 4.</p>
    `;
    return {
        update(_state) {},
        unmount() { root.innerHTML = ''; },
    };
}
