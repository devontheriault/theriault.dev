'use strict';

const POLL_INTERVAL_MS = 5000;

/* ---- State ---- */
let activeCountry = null;

/* ---- DOM references ---- */
const elHeatmapGrid      = document.getElementById('heatmap-grid');
const elHeatmapHourLabels = document.getElementById('heatmap-hour-labels');
const elTotalVisits      = document.getElementById('total-visits');
const elUniqueCountries  = document.getElementById('unique-countries');
const elCountryList      = document.getElementById('country-list');
const elCitySection      = document.getElementById('city-section');
const elCityList         = document.getElementById('city-list');
const elFilterClear      = document.getElementById('filter-clear');
const elFilterLabel      = document.getElementById('filter-label');
const elHeatmapFilterClear = document.getElementById('heatmap-filter-clear');
const elHeatmapFilterLabel = document.getElementById('heatmap-filter-label');
const elStatusDot        = document.getElementById('status-dot');

/* ---- Tooltip ---- */
const tooltip = document.createElement('div');
tooltip.className = 'tooltip';
document.body.appendChild(tooltip);

let _ttVisible = false;

function showTooltip(html, e) {
    tooltip.innerHTML = html;
    tooltip.style.display = 'block';
    _ttVisible = true;
    moveTooltip(e);
}

function hideTooltip() {
    tooltip.style.display = 'none';
    _ttVisible = false;
}

function moveTooltip(e) {
    if (!_ttVisible) return;
    const pad = 14;
    const x = e.clientX + pad;
    const y = e.clientY - 10;
    const tw = tooltip.offsetWidth, th = tooltip.offsetHeight;
    tooltip.style.left = (x + tw > window.innerWidth  ? e.clientX - tw - pad : x) + 'px';
    tooltip.style.top  = (y + th > window.innerHeight ? e.clientY - th - pad : y) + 'px';
}

document.addEventListener('mousemove', moveTooltip);

/* ---- Helpers ---- */
function formatNumber(n) {
    return Number(n).toLocaleString();
}

function escapeHtml(s) {
    return s
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;');
}

function setError(msg) {
    if (elStatusDot) elStatusDot.style.background = '#f85149';
    console.error('[sse]', msg);
}

function setOk() {
    if (elStatusDot) elStatusDot.style.background = '#3fb950';
}

/* ---- Render ---- */
function renderBarList(container, items, nameKey, activeValue) {
    if (items.length === 0) {
        container.innerHTML = '<p class="empty-state">No data recorded yet.</p>';
        return;
    }
    const maxCount = items[0].count || 1;
    const isClickable = nameKey === 'country';

    container.innerHTML = items.map(function(item) {
        const name = item[nameKey] || '';
        const pct = Math.max(2, Math.round((item.count / maxCount) * 100));
        const safeName = escapeHtml(name);
        const isActive = activeValue && name === activeValue;
        const classes = 'country-item' +
            (isClickable ? ' clickable' : '') +
            (isActive    ? ' active'    : '');
        return (
            '<div class="' + classes + '" data-' + nameKey + '="' + safeName + '" data-count="' + item.count + '" data-pct="' + pct + '">' +
            '  <span class="country-name" title="' + safeName + '">' + safeName + '</span>' +
            '  <div class="bar-wrap">' +
            '    <div class="bar-fill" style="width:' + pct + '%"></div>' +
            '  </div>' +
            '  <span class="country-count">' + formatNumber(item.count) + '</span>' +
            '</div>'
        );
    }).join('');
}

function renderStats(data) {
    if (elTotalVisits) {
        elTotalVisits.textContent = formatNumber(data.total_visits || 0);
    }
    if (elUniqueCountries) {
        elUniqueCountries.textContent = formatNumber(data.unique_countries || 0);
    }

    if (elCountryList) {
        renderBarList(elCountryList, data.top_countries || [], 'country', data.active_country || null);
    }

    const cities = data.top_cities || [];
    if (elCitySection) {
        elCitySection.style.display = cities.length > 0 ? '' : 'none';
    }
    if (elCityList) {
        renderBarList(elCityList, cities, 'city', null);
    }

    if (elFilterClear && elFilterLabel) {
        const ac = data.active_country || '';
        elFilterClear.style.display = ac ? '' : 'none';
        elFilterLabel.textContent = ac;
        elHeatmapFilterClear.style.display = ac ? '' : 'none';
        elHeatmapFilterLabel.textContent = ac;
    }

    setOk();
}

/* ---- Heatmap ---- */
function renderHeatmap(data) {
    if (!elHeatmapGrid) return;

    if (elHeatmapHourLabels && elHeatmapHourLabels.childElementCount === 0) {
        for (var h = 0; h < 24; h++) {
            var lbl = document.createElement('span');
            lbl.textContent = h % 3 === 0 ? String(h).padStart(2, '0') : '';
            elHeatmapHourLabels.appendChild(lbl);
        }
    }

    var max = data.max || 1;
    var cells = elHeatmapGrid.children;
    var needsBuild = cells.length !== 7 * 24;

    if (needsBuild) elHeatmapGrid.innerHTML = '';

    for (var d = 0; d < 7; d++) {
        for (var h = 0; h < 24; h++) {
            var count = (data.data[d] || [])[h] || 0;
            var intensity = max > 0 ? count / max : 0;
            var cell = needsBuild ? document.createElement('div') : cells[d * 24 + h];
            cell.className = 'heatmap-cell';
            cell.style.background = 'rgba(88,166,255,' + (0.08 + intensity * 0.92).toFixed(3) + ')';
            cell.dataset.tooltip = ['Sun','Mon','Tue','Wed','Thu','Fri','Sat'][d] + ' ' +
                String(h).padStart(2,'0') + ':00 — ' + count + ' visit' + (count !== 1 ? 's' : '');
            if (needsBuild) elHeatmapGrid.appendChild(cell);
        }
    }
}

function fetchHeatmap() {
    var url = activeCountry
        ? '/heatmap?country=' + encodeURIComponent(activeCountry)
        : '/heatmap';
    fetch(url)
        .then(function(res) { return res.json(); })
        .then(renderHeatmap)
        .catch(function(err) { console.error('[app] /heatmap fetch failed:', err); });
}

/* ---- Fetch ---- */
function fetchStats() {
    const url = activeCountry
        ? '/stats?country=' + encodeURIComponent(activeCountry)
        : '/stats';

    fetch(url)
        .then(function(res) {
            if (!res.ok) throw new Error('HTTP ' + res.status);
            return res.json();
        })
        .then(function(data) {
            renderStats(data);
        })
        .catch(function(err) {
            console.error('[app] /stats fetch failed:', err);
            setError(err.message);
        });
}

/* ---- Country click filter ---- */
if (elCountryList) {
    elCountryList.addEventListener('click', function(e) {
        const item = e.target.closest('.country-item.clickable');
        if (!item) return;
        const country = item.getAttribute('data-country');
        activeCountry = (activeCountry === country) ? null : country;
        fetchStats();
        fetchHeatmap();
    });
}

if (elFilterClear) {
    elFilterClear.addEventListener('click', function() {
        activeCountry = null;
        fetchStats();
        fetchHeatmap();
    });
}

if (elHeatmapFilterClear) {
    elHeatmapFilterClear.addEventListener('click', function() {
        activeCountry = null;
        fetchStats();
        fetchHeatmap();
    });
}

/* ---- Tooltip wiring ---- */
function wireBarTooltips(container, nameKey) {
    container.addEventListener('mouseover', function(e) {
        const item = e.target.closest('.country-item');
        if (!item) return;
        const name  = item.dataset[nameKey] || '';
        const count = item.dataset.count || '0';
        showTooltip('<strong>' + escapeHtml(name) + '</strong><br>' + formatNumber(Number(count)) + ' visits', e);
    });
    container.addEventListener('mouseout', function(e) {
        if (!e.target.closest('.country-item')) return;
        hideTooltip();
    });
}

if (elCountryList) wireBarTooltips(elCountryList, 'country');
if (elCityList)    wireBarTooltips(elCityList,    'city');

if (elHeatmapGrid) {
    elHeatmapGrid.addEventListener('mouseover', function(e) {
        const cell = e.target.closest('.heatmap-cell');
        if (!cell || !cell.dataset.tooltip) return;
        showTooltip(cell.dataset.tooltip, e);
    });
    elHeatmapGrid.addEventListener('mouseout', function(e) {
        if (!e.target.closest('.heatmap-cell')) return;
        hideTooltip();
    });
}

document.querySelectorAll('.card[data-tooltip]').forEach(function(card) {
    card.addEventListener('mouseenter', function(e) { showTooltip(card.dataset.tooltip, e); });
    card.addEventListener('mouseleave', hideTooltip);
});

/* ---- Bootstrap ---- */
fetchStats();
fetchHeatmap();

/* ---- SSE: push updates on each new visit ---- */
(function () {
    var es = new EventSource('/events');

    es.onmessage = function (e) {
        try {
            var payload = JSON.parse(e.data);
            /* Only overwrite stats/heatmap when no country filter is active */
            if (!activeCountry) {
                if (payload.stats)   renderStats(payload.stats);
                if (payload.heatmap) renderHeatmap(payload.heatmap);
            }
            if (payload.globe && window._globeUpdateFn)
                window._globeUpdateFn(payload.globe);
        } catch (err) {
            console.error('[sse] parse error:', err);
        }
    };

    es.onerror = function () { setError('SSE disconnected — reconnecting…'); };
    es.onopen  = function () { setOk(); };
}());
