const tooltip = document.createElement('div');
tooltip.className = 'tooltip';
document.body.appendChild(tooltip);

let _ttVisible = false;

export function showTooltip(html, e) {
    tooltip.innerHTML = html;
    tooltip.style.display = 'block';
    _ttVisible = true;
    moveTooltip(e);
}

export function hideTooltip() {
    tooltip.style.display = 'none';
    _ttVisible = false;
}

export function moveTooltip(e) {
    if (!_ttVisible) return;
    const pad = 14;
    const x = e.clientX + pad;
    const y = e.clientY - 10;
    const tw = tooltip.offsetWidth, th = tooltip.offsetHeight;
    tooltip.style.left = (x + tw > window.innerWidth  ? e.clientX - tw - pad : x) + 'px';
    tooltip.style.top  = (y + th > window.innerHeight ? e.clientY - th - pad : y) + 'px';
}

document.addEventListener('mousemove', moveTooltip);
document.addEventListener('scroll', hideTooltip, { passive: true, capture: true });
