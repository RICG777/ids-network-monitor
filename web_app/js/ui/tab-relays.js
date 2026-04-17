import {
    isValidTriggerString, isValidDuration, isValidRelayMode, normaliseTriggerInput
} from '../validators.js';

function relaySection(n) {
    return `
        <div class="section" data-relay="${n}">
            <h3>Relay ${n} <span class="pill" id="r${n}-state-pill">—</span></h3>
            <div class="form-grid">
                <label for="r${n}-triggers">Triggers</label>
                <textarea id="r${n}-triggers" rows="2"
                    placeholder="port up; port down; link down"
                    maxlength="119"></textarea>
                <label for="r${n}-mode">Mode</label>
                <select id="r${n}-mode">
                    <option value="PULSE">PULSE (auto-release after duration)</option>
                    <option value="LATCH">LATCH (stay on until manual reset)</option>
                </select>
                <label for="r${n}-duration">Duration (s)</label>
                <input type="number" id="r${n}-duration" min="1" max="255" step="1">
            </div>
            <p class="hint">Semicolon-separated triggers; match is case-insensitive substring in the syslog message.
                Total string ≤119 characters. Newlines are stripped on save.</p>
            <div class="btn-row">
                <button type="button" id="r${n}-write" disabled>Write config</button>
                <button type="button" id="r${n}-reset" class="secondary" disabled>Reset</button>
                <button type="button" id="r${n}-test-quick" class="secondary" disabled>Quick test (3 s)</button>
                <button type="button" id="r${n}-test-timed" class="secondary" disabled>Timed test</button>
            </div>
            <p class="hint" id="r${n}-char-count"></p>
        </div>
    `;
}

export default function createTab(root, ctx) {
    const { sendCommand, refreshStatus, confirm, toast, state } = ctx;

    root.innerHTML = `
        <h2>Relay Config</h2>
        <p class="hint">Both relays run independently. Syslog triggers and ping failures can both drive the same relay — whichever wants the relay on wins.</p>
        ${relaySection(1)}
        ${relaySection(2)}
    `;

    // Cache references per relay
    const relays = [1, 2].map(n => ({
        n,
        triggers: root.querySelector(`#r${n}-triggers`),
        mode:     root.querySelector(`#r${n}-mode`),
        duration: root.querySelector(`#r${n}-duration`),
        state:    root.querySelector(`#r${n}-state-pill`),
        charCount: root.querySelector(`#r${n}-char-count`),
        btnWrite:  root.querySelector(`#r${n}-write`),
        btnReset:  root.querySelector(`#r${n}-reset`),
        btnTestQ:  root.querySelector(`#r${n}-test-quick`),
        btnTestT:  root.querySelector(`#r${n}-test-timed`),
    }));

    // --- Per-relay validation + "dirty" diff to control Write button ---
    function currentBoardConfig(n) {
        const s = state.get();
        const r = n === 1 ? s.relay1 : s.relay2;
        return { triggers: r.triggers ?? '', mode: r.mode ?? 'PULSE', duration: r.duration ?? 10 };
    }

    function refreshFormRow(r) {
        const raw = r.triggers.value;
        const trigOk = isValidTriggerString(raw);
        r.triggers.classList.toggle('invalid', raw.length > 0 && !trigOk);
        const durOk = isValidDuration(Number(r.duration.value));
        r.duration.classList.toggle('invalid', r.duration.value !== '' && !durOk);
        const connected = state.get().connection.status === 'connected';
        const board = currentBoardConfig(r.n);
        const modeVal = r.mode.value;
        const changed =
            raw !== board.triggers ||
            modeVal !== board.mode ||
            Number(r.duration.value) !== board.duration;
        r.btnWrite.disabled = !connected || !trigOk || !durOk || !isValidRelayMode(modeVal) || !changed;
        // Character count hint
        r.charCount.textContent = `${raw.length} / 119`;
    }

    for (const r of relays) {
        r.triggers.addEventListener('input', () => refreshFormRow(r));
        r.mode.addEventListener('change', () => refreshFormRow(r));
        r.duration.addEventListener('input', () => refreshFormRow(r));

        r.btnWrite.addEventListener('click', async () => {
            const board = currentBoardConfig(r.n);
            const newTrig = normaliseTriggerInput(r.triggers.value);
            const newMode = r.mode.value;
            const newDur  = Number(r.duration.value);
            const changes = [];
            if (newTrig !== board.triggers)
                changes.push({ cmd: `Relay${r.n}:${newTrig}`, desc: `Triggers: "${board.triggers}" → "${newTrig}"` });
            if (newMode !== board.mode)
                changes.push({ cmd: `MODE${r.n}:${newMode}`,  desc: `Mode: ${board.mode} → ${newMode}` });
            if (newDur  !== board.duration)
                changes.push({ cmd: `DURATION${r.n}:${newDur}`, desc: `Duration: ${board.duration} → ${newDur} s` });
            if (changes.length === 0) { toast('Nothing to write', 'info', 2000); return; }

            // Confirm only if triggers changed (overwrites persistent strings)
            const changedTriggers = changes.some(c => c.cmd.startsWith(`Relay${r.n}:`));
            if (changedTriggers) {
                const ok = await confirm({
                    title: `Write Relay ${r.n} config?`,
                    body:
                        `<ul>${changes.map(c => `<li>${c.desc}</li>`).join('')}</ul>
                         <p class="hint">Writing triggers overwrites the current list. Confirm before saving.</p>`,
                    okLabel: 'Write',
                    danger: true,
                });
                if (!ok) return;
            }

            r.btnWrite.disabled = true;
            try {
                for (const c of changes) await sendCommand(c.cmd);
                await refreshStatus();
                toast(`Relay ${r.n} written`, 'ok');
            } catch {} finally { refreshFormRow(r); }
        });

        r.btnReset.addEventListener('click', async () => {
            try { await sendCommand(`RESET${r.n}`); toast(`Relay ${r.n} reset`, 'ok', 1500); await refreshStatus(); }
            catch {}
        });

        r.btnTestQ.addEventListener('click', async () => {
            try { await sendCommand(`TEST${r.n}`); toast(`Relay ${r.n} test complete`, 'ok', 1500); await refreshStatus(); }
            catch {}
        });

        r.btnTestT.addEventListener('click', async () => {
            const board = currentBoardConfig(r.n);
            if (board.duration >= 30) {
                const ok = await confirm({
                    title: `Timed test of ${board.duration} seconds?`,
                    body:
                        `<p>The board will block other commands for the full ${board.duration} s. ` +
                        `The UI stays locked until the pulse completes.</p>`,
                    okLabel: 'Run test',
                    danger: false,
                });
                if (!ok) return;
            }
            try { await sendCommand(`TTEST${r.n}`); toast(`Relay ${r.n} timed test complete`, 'ok', 1500); await refreshStatus(); }
            catch {}
        });
    }

    // --- Render from state ---
    function update(s) {
        const connected = s.connection.status === 'connected';
        const blocking  = s.ui.blocking;

        for (const r of relays) {
            const board = currentBoardConfig(r.n);

            // Only pull into the form if the user isn't currently typing / editing
            if (document.activeElement !== r.triggers) r.triggers.value = board.triggers;
            if (document.activeElement !== r.mode)     r.mode.value     = board.mode;
            if (document.activeElement !== r.duration) r.duration.value = board.duration ?? '';

            // State pill
            const relayState = (r.n === 1 ? s.relay1 : s.relay2).state;
            r.state.textContent = relayState ?? '—';
            r.state.classList.toggle('ok',   relayState === 'OFF');
            r.state.classList.toggle('err',  relayState === 'ON');

            // Enable/disable controls
            const canInteract = connected && !blocking;
            for (const el of [r.triggers, r.mode, r.duration]) el.disabled = !connected;
            r.btnReset.disabled  = !canInteract;
            r.btnTestQ.disabled  = !canInteract;
            r.btnTestT.disabled  = !canInteract;
            refreshFormRow(r);  // sets btnWrite.disabled appropriately
            if (blocking) r.btnWrite.disabled = true;
        }
    }

    return {
        update,
        unmount() { root.innerHTML = ''; },
    };
}
