export default function createTab(root) {
    root.innerHTML = `
        <h2>Event Log</h2>
        <p class="placeholder">Timestamped stream of every line from the board, with filters and log-to-file. Phase 1.</p>
    `;
    return {
        update(_state) {},
        unmount() { root.innerHTML = ''; },
    };
}
