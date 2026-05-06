'use strict';

const POLL_INTERVAL_MS = 5000;

/* ---- State ---- */
let activeCountry = null;

/* ---- DOM references ---- */
const elTotalVisits      = document.getElementById('total-visits');
const elUniqueCountries  = document.getElementById('unique-countries');
const elCountryList      = document.getElementById('country-list');
const elCitySection      = document.getElementById('city-section');
const elCityList         = document.getElementById('city-list');
const elFilterClear      = document.getElementById('filter-clear');
const elFilterLabel      = document.getElementById('filter-label');
const elLastUpdated      = document.getElementById('last-updated');
const elStatusDot        = document.getElementById('status-dot');

/* ---- Helpers ---- */
function formatNumber(n) {
    return Number(n).toLocaleString();
}

function formatTime(d) {
    return d.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
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
    if (elLastUpdated) elLastUpdated.textContent = 'Error: ' + msg;
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
            '<div class="' + classes + '" data-' + nameKey + '="' + safeName + '">' +
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
    }

    if (elLastUpdated) {
        elLastUpdated.textContent = 'Last updated: ' + formatTime(new Date());
    }

    setOk();
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
    });
}

if (elFilterClear) {
    elFilterClear.addEventListener('click', function() {
        activeCountry = null;
        fetchStats();
    });
}

/* ---- Bootstrap ---- */
fetchStats();
setInterval(fetchStats, POLL_INTERVAL_MS);
