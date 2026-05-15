import { updateFilterUI } from './render/filters.js';
import { renderStats } from './render/stats.js';
import { renderHeatmap } from './render/heatmap.js';
import { fetchStats, fetchHeatmap, fetchGlobe } from './api.js';

export function refreshAll() {
    updateFilterUI();
    fetchStats().then(function(data) { if (data) renderStats(data); });
    fetchHeatmap().then(function(data) { if (data) renderHeatmap(data); });
    fetchGlobe();
}
