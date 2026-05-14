'use strict';

const POLL_INTERVAL_MS = 5000;

/* ---- State ---- */
let activeCountry    = null;
let activeCity       = null;
let activeDow        = null;
let activeHour       = null;
let activeBrowser    = null;
let activeDeviceType = null;
let activeOs         = null;

/* ---- DOM references ---- */
const elHeatmapGrid      = document.getElementById('heatmap-grid');
const elHeatmapHourLabels = document.getElementById('heatmap-hour-labels');
const elUniqueVisitors   = document.getElementById('unique-visitors');
const elTotalVisits      = document.getElementById('total-visits');
const elUniqueCountries  = document.getElementById('unique-countries');
const elCountryList      = document.getElementById('country-list');
const elCitySection      = document.getElementById('city-section');
const elCityList         = document.getElementById('city-list');
const elFilterClear         = document.getElementById('filter-clear');
const elFilterLabel         = document.getElementById('filter-label');
const elHeatmapCountryClear = document.getElementById('heatmap-country-clear');
const elHeatmapCountryLabel = document.getElementById('heatmap-country-label');
const elHeatmapCityClear    = document.getElementById('heatmap-city-clear');
const elHeatmapCityLabel    = document.getElementById('heatmap-city-label');
const elStatusDot           = document.getElementById('status-dot');
const elBrowserList         = document.getElementById('browser-list');
const elDeviceList          = document.getElementById('device-list');
const elOsList              = document.getElementById('os-list');
const elHeatmapDowClear  = document.getElementById('heatmap-dow-clear');
const elHeatmapDowLabel  = document.getElementById('heatmap-dow-label');
const elHeatmapHourClear = document.getElementById('heatmap-hour-clear');
const elHeatmapHourLabel = document.getElementById('heatmap-hour-label');
const elBrowserClear     = document.getElementById('browser-clear');
const elBrowserLabel     = document.getElementById('browser-label');
const elDeviceClear      = document.getElementById('device-clear');
const elDeviceLabel      = document.getElementById('device-label');
const elOsClear          = document.getElementById('os-clear');
const elOsLabel          = document.getElementById('os-label');

/* ---- Filter UI ---- */
function updateFilterUI() {
    // Top Cities section: shows active country
    if (elFilterClear && elFilterLabel) {
        elFilterClear.style.display = activeCountry ? '' : 'none';
        elFilterLabel.textContent   = activeCountry || '';
    }
    // Heatmap: separate country and city badges
    if (elHeatmapCountryClear && elHeatmapCountryLabel) {
        elHeatmapCountryClear.style.display = activeCountry ? '' : 'none';
        elHeatmapCountryLabel.textContent   = activeCountry || '';
    }
    if (elHeatmapCityClear && elHeatmapCityLabel) {
        elHeatmapCityClear.style.display = activeCity ? '' : 'none';
        elHeatmapCityLabel.textContent   = activeCity || '';
    }
    // Also keep city list active highlights in sync
    if (elCityList) {
        elCityList.querySelectorAll('.country-item').forEach(function(el) {
            el.classList.toggle('active', el.getAttribute('data-city') === activeCity);
        });
    }
    var DOW_NAMES = ['Sun','Mon','Tue','Wed','Thu','Fri','Sat'];
    if (elHeatmapDowClear && elHeatmapDowLabel) {
        elHeatmapDowClear.style.display = activeDow !== null ? '' : 'none';
        elHeatmapDowLabel.textContent   = activeDow !== null ? DOW_NAMES[activeDow] : '';
    }
    if (elHeatmapHourClear && elHeatmapHourLabel) {
        elHeatmapHourClear.style.display = activeHour !== null ? '' : 'none';
        elHeatmapHourLabel.textContent   = activeHour !== null
            ? (String(activeHour).padStart(2, '0') + ':00') : '';
    }
    if (elBrowserClear && elBrowserLabel) {
        elBrowserClear.style.display = activeBrowser ? '' : 'none';
        elBrowserLabel.textContent   = activeBrowser || '';
    }
    if (elDeviceClear && elDeviceLabel) {
        elDeviceClear.style.display = activeDeviceType ? '' : 'none';
        elDeviceLabel.textContent   = activeDeviceType || '';
    }
    if (elOsClear && elOsLabel) {
        elOsClear.style.display = activeOs ? '' : 'none';
        elOsLabel.textContent   = activeOs || '';
    }
    // Update active highlights in browser/device/os lists
    if (elBrowserList) {
        elBrowserList.querySelectorAll('.country-item').forEach(function(el) {
            el.classList.toggle('active', el.getAttribute('data-browser') === activeBrowser);
        });
    }
    if (elDeviceList) {
        elDeviceList.querySelectorAll('.country-item').forEach(function(el) {
            el.classList.toggle('active', el.getAttribute('data-device_type') === activeDeviceType);
        });
    }
    if (elOsList) {
        elOsList.querySelectorAll('.country-item').forEach(function(el) {
            el.classList.toggle('active', el.getAttribute('data-os') === activeOs);
        });
    }
    updateHeatmapHighlight();
}

function updateHeatmapHighlight() {
    document.querySelectorAll('.heatmap-day-labels span').forEach(function(span, i) {
        span.classList.toggle('active', activeDow === i);
    });
    if (elHeatmapHourLabels) {
        Array.from(elHeatmapHourLabels.querySelectorAll('span')).forEach(function(span) {
            var h = parseInt(span.dataset.hour, 10);
            span.classList.toggle('active', activeHour !== null && h === activeHour);
        });
    }
    if (elHeatmapGrid) {
        Array.from(elHeatmapGrid.children).forEach(function(cell) {
            var d = parseInt(cell.dataset.dow,  10);
            var h = parseInt(cell.dataset.hour, 10);
            cell.classList.toggle('active',
                activeDow !== null && d === activeDow &&
                activeHour !== null && h === activeHour);
        });
    }
}

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
document.addEventListener('scroll', hideTooltip, { passive: true, capture: true });

/* ---- Helpers ---- */
function formatNumber(n) {
    return Number(n).toLocaleString();
}

function formatDuration(seconds) {
    var s = Math.round(seconds);
    if (s < 60) return s + 's';
    var m = Math.floor(s / 60), r = s % 60;
    return m + 'm ' + r + 's';
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

/* ---- Icons ---- */
const BROWSER_ICONS = {
    'Chrome':  'fa-brands fa-chrome',
    'Firefox': 'fa-brands fa-firefox-browser',
    'Safari':  'fa-brands fa-safari',
    'Edge':    'fa-brands fa-edge',
    'Opera':   'fa-brands fa-opera',
    'Other':   'fa-solid fa-globe',
};

const OS_ICONS = {
    'Windows':  'fa-brands fa-windows',
    'macOS':    'fa-brands fa-apple',
    'Linux':    'fa-brands fa-linux',
    'Android':  'fa-brands fa-android',
    'iOS':      'fa-brands fa-apple',
    'ChromeOS': 'fa-brands fa-chrome',
    'Unknown':  'fa-solid fa-circle-question',
};

const DEVICE_ICONS = {
    'Desktop': 'fa-solid fa-desktop',
    'Mobile':  'fa-solid fa-mobile-screen',
    'Tablet':  'fa-solid fa-tablet-screen-button',
};

function browserIcon(name)  { return BROWSER_ICONS[name] || 'fa-solid fa-globe'; }
function osIcon(name)       { return OS_ICONS[name]       || 'fa-solid fa-circle-question'; }
function deviceIcon(name)   { return DEVICE_ICONS[name]   || 'fa-solid fa-desktop'; }

/* ---- Render ---- */
function renderBarList(container, items, nameKey, activeValue, iconFn) {
    if (items.length === 0) {
        container.innerHTML = '<p class="empty-state">No data recorded yet.</p>';
        return;
    }
    const maxCount = items[0].count || 1;
    const isClickable = nameKey === 'country' || nameKey === 'city'
        || nameKey === 'browser' || nameKey === 'device_type' || nameKey === 'os';

    container.innerHTML = items.map(function(item) {
        const name = item[nameKey] || '';
        const pct = Math.max(2, Math.round((item.count / maxCount) * 100));
        const safeName = escapeHtml(name);
        const isActive = activeValue && name === activeValue;
        const classes = 'country-item' +
            (isClickable ? ' clickable' : '') +
            (isActive    ? ' active'    : '');
        const icon = iconFn
            ? '<i class="item-icon ' + escapeHtml(iconFn(name)) + '" aria-hidden="true"></i>'
            : '';
        return (
            '<div class="' + classes + '" data-' + nameKey + '="' + safeName + '" data-count="' + item.count + '" data-pct="' + pct + '">' +
            icon +
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
    if (elUniqueVisitors) {
        elUniqueVisitors.textContent = formatNumber(data.unique_visitors || 0);
    }
    if (elTotalVisits) {
        elTotalVisits.textContent = formatNumber(data.total_visits || 0);
    }
    if (elUniqueCountries) {
        elUniqueCountries.textContent = formatNumber(data.unique_countries || 0);
    }

    var elAvgTime = document.getElementById('avg-time-on-page');
    if (elAvgTime) {
        var avg = data.avg_time_on_page || 0;
        elAvgTime.textContent = avg > 0 ? formatDuration(avg) : '—';
    }

    if (elCountryList) {
        renderBarList(elCountryList, data.top_countries || [], 'country', data.active_country || null);
    }

    const cities = data.top_cities || [];
    if (elCitySection) {
        elCitySection.style.display = cities.length > 0 ? '' : 'none';
    }
    if (elCityList) {
        renderBarList(elCityList, cities, 'city', activeCity);
    }

    if (elBrowserList) {
        renderBarList(elBrowserList, data.browsers || [], 'browser', activeBrowser, browserIcon);
    }
    if (elDeviceList) {
        renderBarList(elDeviceList, data.device_types || [], 'device_type', activeDeviceType, deviceIcon);
    }
    if (elOsList) {
        renderBarList(elOsList, data.os || [], 'os', activeOs, osIcon);
    }

    updateFilterUI();
    setOk();
}

/* ---- Heatmap ---- */
function renderHeatmap(data) {
    if (!elHeatmapGrid) return;

    if (elHeatmapHourLabels && elHeatmapHourLabels.childElementCount === 0) {
        for (var h = 0; h < 24; h++) {
            var lbl = document.createElement('span');
            lbl.textContent = h % 3 === 0 ? String(h).padStart(2, '0') : '';
            lbl.dataset.hour = h;
            lbl.title = String(h).padStart(2, '0') + ':00';
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
            if (needsBuild) {
                cell.dataset.dow  = d;
                cell.dataset.hour = h;
                elHeatmapGrid.appendChild(cell);
            }
        }
    }
    updateHeatmapHighlight();
}

function fetchHeatmap() {
    var params = [];
    if (activeCountry)    params.push('country='     + encodeURIComponent(activeCountry));
    if (activeCity)       params.push('city='        + encodeURIComponent(activeCity));
    if (activeBrowser)    params.push('browser='     + encodeURIComponent(activeBrowser));
    if (activeDeviceType) params.push('device_type=' + encodeURIComponent(activeDeviceType));
    if (activeOs)         params.push('os='          + encodeURIComponent(activeOs));
    var url = '/heatmap' + (params.length ? '?' + params.join('&') : '');
    fetch(url)
        .then(function(res) { return res.json(); })
        .then(renderHeatmap)
        .catch(function(err) { console.error('[app] /heatmap fetch failed:', err); });
}

/* ---- Fetch ---- */
function fetchStats() {
    var params = [];
    if (activeCountry)       params.push('country='     + encodeURIComponent(activeCountry));
    if (activeCity)          params.push('city='        + encodeURIComponent(activeCity));
    if (activeDow  !== null) params.push('dow='         + activeDow);
    if (activeHour !== null) params.push('hour='        + activeHour);
    if (activeBrowser)       params.push('browser='     + encodeURIComponent(activeBrowser));
    if (activeDeviceType)    params.push('device_type=' + encodeURIComponent(activeDeviceType));
    if (activeOs)            params.push('os='          + encodeURIComponent(activeOs));
    var url = '/stats' + (params.length ? '?' + params.join('&') : '');

    return fetch(url)
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
        activeCity = null;
        updateFilterUI();
        fetchStats();
        fetchHeatmap();
    });
}

/* ---- City click filter ---- */
if (elCityList) {
    elCityList.addEventListener('click', function(e) {
        const item = e.target.closest('.country-item.clickable');
        if (!item) return;
        const city = item.getAttribute('data-city');
        activeCity = (activeCity === city) ? null : city;
        elCityList.querySelectorAll('.country-item').forEach(function(el) {
            el.classList.toggle('active', el.getAttribute('data-city') === activeCity);
        });
        updateFilterUI();
        fetchStats();
        fetchHeatmap();
    });
}

/* ---- Browser / Device / OS click filters ---- */
if (elBrowserList) {
    elBrowserList.addEventListener('click', function(e) {
        const item = e.target.closest('.country-item.clickable');
        if (!item) return;
        const val = item.getAttribute('data-browser');
        activeBrowser = (activeBrowser === val) ? null : val;
        updateFilterUI();
        fetchStats();
        fetchHeatmap();
    });
}

if (elDeviceList) {
    elDeviceList.addEventListener('click', function(e) {
        const item = e.target.closest('.country-item.clickable');
        if (!item) return;
        const val = item.getAttribute('data-device_type');
        activeDeviceType = (activeDeviceType === val) ? null : val;
        updateFilterUI();
        fetchStats();
        fetchHeatmap();
    });
}

if (elOsList) {
    elOsList.addEventListener('click', function(e) {
        const item = e.target.closest('.country-item.clickable');
        if (!item) return;
        const val = item.getAttribute('data-os');
        activeOs = (activeOs === val) ? null : val;
        updateFilterUI();
        fetchStats();
        fetchHeatmap();
    });
}

if (elBrowserClear) {
    elBrowserClear.addEventListener('click', function() {
        activeBrowser = null;
        updateFilterUI();
        fetchStats();
        fetchHeatmap();
    });
}

if (elDeviceClear) {
    elDeviceClear.addEventListener('click', function() {
        activeDeviceType = null;
        updateFilterUI();
        fetchStats();
        fetchHeatmap();
    });
}

if (elOsClear) {
    elOsClear.addEventListener('click', function() {
        activeOs = null;
        updateFilterUI();
        fetchStats();
        fetchHeatmap();
    });
}

// Clear country (top cities section) → clears both
if (elFilterClear) {
    elFilterClear.addEventListener('click', function() {
        activeCountry = null;
        activeCity = null;
        updateFilterUI();
        fetchStats();
        fetchHeatmap();
    });
}

// Clear country (heatmap) → clears both
if (elHeatmapCountryClear) {
    elHeatmapCountryClear.addEventListener('click', function() {
        activeCountry = null;
        activeCity = null;
        updateFilterUI();
        fetchStats();
        fetchHeatmap();
    });
}

// Clear city (heatmap) → clears city only, country stays
if (elHeatmapCityClear) {
    elHeatmapCityClear.addEventListener('click', function() {
        activeCity = null;
        updateFilterUI();
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
    container.addEventListener('touchend', hideTooltip);
}

if (elCountryList) wireBarTooltips(elCountryList, 'country');
if (elCityList)    wireBarTooltips(elCityList,    'city');
if (elBrowserList) wireBarTooltips(elBrowserList, 'browser');
if (elDeviceList)  wireBarTooltips(elDeviceList,  'device_type');
if (elOsList)      wireBarTooltips(elOsList,       'os');

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
    elHeatmapGrid.addEventListener('touchend', hideTooltip);
}

if (elHeatmapGrid) {
    elHeatmapGrid.addEventListener('click', function(e) {
        var cell = e.target.closest('.heatmap-cell');
        if (!cell) return;
        var d = parseInt(cell.dataset.dow,  10);
        var h = parseInt(cell.dataset.hour, 10);
        if (activeDow === d && activeHour === h) {
            activeDow = null; activeHour = null;
        } else {
            activeDow = d; activeHour = h;
        }
        updateFilterUI();
        fetchStats();
    });
}

var elDayLabels = document.querySelector('.heatmap-day-labels');
if (elDayLabels) {
    elDayLabels.addEventListener('click', function(e) {
        var span = e.target.closest('span');
        if (!span) return;
        var spans = Array.from(elDayLabels.querySelectorAll('span'));
        var d = spans.indexOf(span);
        if (d < 0) return;
        activeDow = (activeDow === d) ? null : d;
        updateFilterUI();
        fetchStats();
    });
}

if (elHeatmapHourLabels) {
    elHeatmapHourLabels.addEventListener('click', function(e) {
        var span = e.target.closest('span');
        if (!span || span.dataset.hour === undefined) return;
        var h = parseInt(span.dataset.hour, 10);
        activeHour = (activeHour === h) ? null : h;
        updateFilterUI();
        fetchStats();
    });
}

if (elHeatmapDowClear) {
    elHeatmapDowClear.addEventListener('click', function() {
        activeDow = null;
        updateFilterUI();
        fetchStats();
    });
}

if (elHeatmapHourClear) {
    elHeatmapHourClear.addEventListener('click', function() {
        activeHour = null;
        updateFilterUI();
        fetchStats();
    });
}

document.querySelectorAll('.card[data-tooltip]').forEach(function(card) {
    card.addEventListener('mouseenter', function(e) { showTooltip(card.dataset.tooltip, e); });
    card.addEventListener('mouseleave', hideTooltip);
    card.addEventListener('touchend', hideTooltip);
});

/* ---- Your Visit ---- */
(function () {
    fetch('/me')
        .then(function(res) { return res.json(); })
        .then(function(data) {
            var loc = [data.city, data.country].filter(Boolean).join(', ') || '—';
            var el;
            el = document.getElementById('your-location');
            if (el) el.textContent = loc;
            el = document.getElementById('your-browser');
            if (el) {
                el.innerHTML = '<i class="' + escapeHtml(browserIcon(data.browser)) + '" aria-hidden="true"></i> ' + escapeHtml(data.browser || '—');
            }
            el = document.getElementById('your-device');
            if (el) {
                el.innerHTML = '<i class="' + escapeHtml(deviceIcon(data.device_type)) + '" aria-hidden="true"></i> ' + escapeHtml(data.device_type || '—');
            }
            el = document.getElementById('your-os');
            if (el) {
                el.innerHTML = '<i class="' + escapeHtml(osIcon(data.os)) + '" aria-hidden="true"></i> ' + escapeHtml(data.os || '—');
            }
        })
        .catch(function(err) { console.error('[app] /me fetch failed:', err); });
}());

/* ---- Time on page beacon ---- */
(function () {
    var startTime = Date.now();
    document.addEventListener('visibilitychange', function () {
        if (document.visibilityState === 'hidden') {
            var seconds = Math.round((Date.now() - startTime) / 1000);
            if (seconds > 0) {
                navigator.sendBeacon('/duration', 'seconds=' + seconds);
            }
        }
    });
}());

/* ---- SSE: push updates on each new visit ---- */
function initSSE() {
    var es = new EventSource('/events');

    es.onmessage = function (e) {
        try {
            var payload = JSON.parse(e.data);
            /* Only overwrite stats/heatmap when no filter is active */
            if (!activeCountry && !activeCity && activeDow === null && activeHour === null
                    && !activeBrowser && !activeDeviceType && !activeOs) {
                if (payload.stats)   renderStats(payload.stats);
                if (payload.heatmap) renderHeatmap(payload.heatmap);
            }
            if (payload.globe) {
                if (window._globeUpdateFn) window._globeUpdateFn(payload.globe);
                else window._pendingGlobeData = payload.globe;
            }
        } catch (err) {
            console.error('[sse] parse error:', err);
        }
    };

    es.onerror = function () { setError('SSE disconnected — reconnecting…'); };
    es.onopen  = function () { setOk(); };
}

/* ---- Bootstrap ---- */
fetchStats().then(initSSE);
fetchHeatmap();

/* ---- Mouse-following background gradient ---- */
(function () {
    const bg = document.getElementById('bg-gradient');
    if (!bg) return;
    const pull = 0.4;
    const ease = 0.04; // lerp speed (lower = slower/lazier)
    let cx = 50, cy = 50, tx = 50, ty = 50;

    document.addEventListener('mousemove', function (e) {
        const mx = e.clientX / window.innerWidth  * 100;
        const my = e.clientY / window.innerHeight * 100;
        tx = 50 + (mx - 50) * pull;
        ty = 50 + (my - 50) * pull;
    });

    (function animate() {
        cx += (tx - cx) * ease;
        cy += (ty - cy) * ease;
        bg.style.background = `radial-gradient(circle at ${cx.toFixed(2)}% ${cy.toFixed(2)}%, #1e3a6e 0%, #0f1f3d 20%, #0d1117 40%, #080c12 100%)`;
        requestAnimationFrame(animate);
    }());
}());
