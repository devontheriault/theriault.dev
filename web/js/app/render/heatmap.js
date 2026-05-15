import { state } from '../state.js';

const elHeatmapGrid       = document.getElementById('heatmap-grid');
const elHeatmapHourLabels = document.getElementById('heatmap-hour-labels');
const DOW_NAMES = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];

export function updateHeatmapHighlight() {
    document.querySelectorAll('.heatmap-day-labels span').forEach(function(span, i) {
        span.classList.toggle('active', state.dow === i);
    });
    if (elHeatmapHourLabels) {
        elHeatmapHourLabels.querySelectorAll('span').forEach(function(span) {
            const h = parseInt(span.dataset.hour, 10);
            span.classList.toggle('active', state.hour !== null && h === state.hour);
        });
    }
    if (elHeatmapGrid) {
        Array.from(elHeatmapGrid.children).forEach(function(cell) {
            const d = parseInt(cell.dataset.dow,  10);
            const h = parseInt(cell.dataset.hour, 10);
            cell.classList.toggle('active',
                state.dow !== null && d === state.dow &&
                state.hour !== null && h === state.hour);
        });
    }
}

export function renderHeatmap(data) {
    if (!elHeatmapGrid) return;

    if (elHeatmapHourLabels && elHeatmapHourLabels.childElementCount === 0) {
        for (let h = 0; h < 24; h++) {
            const lbl = document.createElement('span');
            lbl.textContent = h % 3 === 0 ? String(h).padStart(2, '0') : '';
            lbl.dataset.hour = h;
            lbl.title = String(h).padStart(2, '0') + ':00';
            elHeatmapHourLabels.appendChild(lbl);
        }
    }

    const max = data.max || 1;
    const cells = elHeatmapGrid.children;
    const needsBuild = cells.length !== 7 * 24;
    if (needsBuild) elHeatmapGrid.innerHTML = '';

    for (let d = 0; d < 7; d++) {
        for (let h = 0; h < 24; h++) {
            const count = (data.data[d] || [])[h] || 0;
            const intensity = max > 0 ? count / max : 0;
            const cell = needsBuild ? document.createElement('div') : cells[d * 24 + h];
            cell.className = 'heatmap-cell';
            cell.style.background = 'rgba(88,166,255,' + (0.08 + intensity * 0.92).toFixed(3) + ')';
            cell.dataset.tooltip = DOW_NAMES[d] + ' ' +
                String(h).padStart(2, '0') + ':00 — ' + count + ' visit' + (count !== 1 ? 's' : '');
            if (needsBuild) {
                cell.dataset.dow  = d;
                cell.dataset.hour = h;
                elHeatmapGrid.appendChild(cell);
            }
        }
    }
    updateHeatmapHighlight();
}
