import { fetchStats, fetchHeatmap } from './api.js';
import { renderStats } from './render/stats.js';
import { renderHeatmap } from './render/heatmap.js';
import { initSSE } from './realtime/sse.js';
import { bindFilterEvents } from './events/filters.js';
import { bindHeatmapEvents } from './events/heatmap.js';
import { bindTooltipEvents } from './events/tooltips.js';
import { initBackground } from './ui/background.js';
import { escapeHtml } from './ui/helpers.js';
import { browserIcon, deviceIcon, osIcon } from './ui/icons.js';

bindFilterEvents();
bindHeatmapEvents();
bindTooltipEvents();
initBackground();

(function fetchYourVisit() {
    fetch('/me')
        .then(function(res) { return res.json(); })
        .then(function(data) {
            const loc = [data.city, data.country].filter(Boolean).join(', ') || '—';
            let el;
            el = document.getElementById('your-location');
            if (el) el.textContent = loc;
            el = document.getElementById('your-browser');
            if (el) el.innerHTML = '<i class="' + escapeHtml(browserIcon(data.browser)) + '" aria-hidden="true"></i> ' + escapeHtml(data.browser || '—');
            el = document.getElementById('your-device');
            if (el) el.innerHTML = '<i class="' + escapeHtml(deviceIcon(data.device_type)) + '" aria-hidden="true"></i> ' + escapeHtml(data.device_type || '—');
            el = document.getElementById('your-os');
            if (el) el.innerHTML = '<i class="' + escapeHtml(osIcon(data.os)) + '" aria-hidden="true"></i> ' + escapeHtml(data.os || '—');
        })
        .catch(function(err) { console.error('[app] /me fetch failed:', err); });
}());

(function timeOnPageBeacon() {
    const startTime = Date.now();
    document.addEventListener('visibilitychange', function() {
        if (document.visibilityState === 'hidden') {
            const seconds = Math.round((Date.now() - startTime) / 1000);
            if (seconds > 0) navigator.sendBeacon('/duration', 'seconds=' + seconds + '&exit_page=' + encodeURIComponent(window.location.pathname));
        }
    });
}());

fetchStats()
    .then(function(data) { if (data) renderStats(data); })
    .then(initSSE);
fetchHeatmap().then(function(data) { if (data) renderHeatmap(data); });
