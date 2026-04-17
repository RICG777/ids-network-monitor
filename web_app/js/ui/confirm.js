// Modal confirmation dialog, promise-based. Uses native <dialog>.

export function confirm({ title, body, okLabel = 'Confirm', cancelLabel = 'Cancel', danger = false }) {
    return new Promise((resolve) => {
        const dlg = document.createElement('dialog');
        dlg.className = 'confirm';
        dlg.innerHTML = `
            <h3></h3>
            <p></p>
            <div class="btn-row">
                <button type="button" class="secondary" data-act="cancel"></button>
                <button type="button" data-act="ok"></button>
            </div>
        `;
        dlg.querySelector('h3').textContent = title;
        dlg.querySelector('p').innerHTML = body;
        const okBtn = dlg.querySelector('[data-act="ok"]');
        const cancelBtn = dlg.querySelector('[data-act="cancel"]');
        okBtn.textContent = okLabel;
        cancelBtn.textContent = cancelLabel;
        if (danger) okBtn.classList.add('danger');
        document.body.appendChild(dlg);
        dlg.showModal();

        const finish = (ok) => {
            try { dlg.close(); } catch {}
            dlg.remove();
            resolve(ok);
        };
        okBtn.addEventListener('click', () => finish(true));
        cancelBtn.addEventListener('click', () => finish(false));
        dlg.addEventListener('cancel', (e) => { e.preventDefault(); finish(false); });
        cancelBtn.focus();
    });
}
