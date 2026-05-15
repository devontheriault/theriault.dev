import { state } from '../state.js';
import { renderStats } from '../render/stats.js';
import { renderHeatmap } from '../render/heatmap.js';
import { setError, setOk } from '../ui/helpers.js';

function hasActiveFilter() {
    return state.country || state.city || state.dow !== null || state.hour !== null
        || state.browser || state.deviceType || state.os;
}

export function initSSE() {
    const es = new EventSource('/events');

    es.onmessage = function(e) {
        try {
            const payload = JSON.parse(e.data);
            if (!hasActiveFilter()) {
                if (payload.stats)   renderStats(payload.stats);
                if (payload.heatmap) renderHeatmap(payload.heatmap);
                if (payload.globe) {
                    if (window._globeUpdateFn) window._globeUpdateFn(payload.globe);
                    else window._pendingGlobeData = payload.globe;
                }
            }
        } catch (err) {
            console.error('[sse] parse error:', err);
        }
    };

    es.onerror = function() { setError('SSE disconnected — reconnecting…'); };
    es.onopen  = function() { setOk(); };
}
