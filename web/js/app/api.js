import { state } from './state.js';
import { setError } from './ui/helpers.js';

function buildQueryParams() {
    const p = new URLSearchParams();
    if (state.country)       p.set('country',     state.country);
    if (state.city)          p.set('city',        state.city);
    if (state.dow  !== null) p.set('dow',         state.dow);
    if (state.hour !== null) p.set('hour',        state.hour);
    if (state.browser)       p.set('browser',     state.browser);
    if (state.deviceType)    p.set('device_type', state.deviceType);
    if (state.os)            p.set('os',          state.os);
    return p.toString();
}

function buildHeatmapParams() {
    const p = new URLSearchParams();
    if (state.country)    p.set('country',     state.country);
    if (state.city)       p.set('city',        state.city);
    if (state.browser)    p.set('browser',     state.browser);
    if (state.deviceType) p.set('device_type', state.deviceType);
    if (state.os)         p.set('os',          state.os);
    return p.toString();
}

export function fetchStats() {
    const qs = buildQueryParams();
    return fetch('/stats' + (qs ? '?' + qs : ''))
        .then(function(res) {
            if (!res.ok) throw new Error('HTTP ' + res.status);
            return res.json();
        })
        .catch(function(err) {
            console.error('[app] /stats fetch failed:', err);
            setError(err.message);
        });
}

export function fetchHeatmap() {
    const qs = buildHeatmapParams();
    return fetch('/heatmap' + (qs ? '?' + qs : ''))
        .then(function(res) { return res.json(); })
        .catch(function(err) { console.error('[app] /heatmap fetch failed:', err); });
}

export function fetchGlobe() {
    const qs = buildQueryParams();
    if (window._fetchGlobeFn) window._fetchGlobeFn(qs ? '?' + qs : '');
}
