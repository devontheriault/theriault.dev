export function formatNumber(n) {
    return Number(n).toLocaleString();
}

export function formatDuration(seconds) {
    const s = Math.round(seconds);
    if (s < 60) return s + 's';
    const m = Math.floor(s / 60), r = s % 60;
    return m + 'm ' + r + 's';
}

export function escapeHtml(s) {
    return s
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
}

export function setError(msg) {
    const el = document.getElementById('status-dot');
    if (el) el.style.background = '#f85149';
    console.error('[sse]', msg);
}

export function setOk() {
    const el = document.getElementById('status-dot');
    if (el) el.style.background = '#3fb950';
}
