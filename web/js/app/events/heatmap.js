import { state } from '../state.js';
import { updateFilterUI } from '../render/filters.js';
import { renderStats } from '../render/stats.js';
import { fetchStats, fetchGlobe } from '../api.js';

function refreshTimeFilters() {
    updateFilterUI();
    fetchStats().then(function(data) { if (data) renderStats(data); });
    fetchGlobe();
}

export function bindHeatmapEvents() {
    const elHeatmapGrid       = document.getElementById('heatmap-grid');
    const elHeatmapHourLabels = document.getElementById('heatmap-hour-labels');
    const elHeatmapDowClear   = document.getElementById('heatmap-dow-clear');
    const elHeatmapHourClear  = document.getElementById('heatmap-hour-clear');
    const elDayLabels         = document.querySelector('.heatmap-day-labels');

    if (elHeatmapGrid) {
        elHeatmapGrid.addEventListener('click', function(e) {
            const cell = e.target.closest('.heatmap-cell');
            if (!cell) return;
            const d = parseInt(cell.dataset.dow,  10);
            const h = parseInt(cell.dataset.hour, 10);
            if (state.dow === d && state.hour === h) {
                state.dow = null;
                state.hour = null;
            } else {
                state.dow = d;
                state.hour = h;
            }
            refreshTimeFilters();
        });
    }

    if (elDayLabels) {
        elDayLabels.addEventListener('click', function(e) {
            const span = e.target.closest('span');
            if (!span) return;
            const d = Array.from(elDayLabels.querySelectorAll('span')).indexOf(span);
            if (d < 0) return;
            state.dow = (state.dow === d) ? null : d;
            refreshTimeFilters();
        });
    }

    if (elHeatmapHourLabels) {
        elHeatmapHourLabels.addEventListener('click', function(e) {
            const span = e.target.closest('span');
            if (!span || span.dataset.hour === undefined) return;
            const h = parseInt(span.dataset.hour, 10);
            state.hour = (state.hour === h) ? null : h;
            refreshTimeFilters();
        });
    }

    if (elHeatmapDowClear) {
        elHeatmapDowClear.addEventListener('click', function() {
            state.dow = null;
            refreshTimeFilters();
        });
    }

    if (elHeatmapHourClear) {
        elHeatmapHourClear.addEventListener('click', function() {
            state.hour = null;
            refreshTimeFilters();
        });
    }
}
