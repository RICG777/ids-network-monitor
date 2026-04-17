// Blocking banner shown during TEST/TTEST (board blocks other comms).

let bannerEl = null;
let labelEl = null;
let progEl = null;
let rafId = null;
let startTs = 0;
let durationMs = 0;

function ensureBanner() {
    if (bannerEl && document.body.contains(bannerEl)) return bannerEl;
    bannerEl = document.createElement('div');
    bannerEl.className = 'blocking-banner';
    bannerEl.innerHTML = `
        <span class="banner-label"></span>
        <progress max="1" value="0"></progress>
    `;
    labelEl = bannerEl.querySelector('.banner-label');
    progEl  = bannerEl.querySelector('progress');
    document.body.appendChild(bannerEl);
    return bannerEl;
}

export function showBlocking({ reason, durationMs: d = 0 }) {
    const el = ensureBanner();
    labelEl.textContent = reason ?? 'Board is busy...';
    startTs = performance.now();
    durationMs = d;
    el.classList.add('active');
    if (d > 0) {
        progEl.style.display = '';
        const tick = () => {
            const pct = Math.min(1, (performance.now() - startTs) / durationMs);
            progEl.value = pct;
            if (pct < 1) rafId = requestAnimationFrame(tick);
        };
        tick();
    } else {
        progEl.style.display = 'none';
    }
}

export function hideBlocking() {
    if (rafId) { cancelAnimationFrame(rafId); rafId = null; }
    if (bannerEl) bannerEl.classList.remove('active');
}
