import { state } from '../state.js';
import { formatNumber, formatDuration, escapeHtml, setOk } from '../ui/helpers.js';
import { browserIcon, osIcon, deviceIcon } from '../ui/icons.js';

const elUniqueVisitors  = document.getElementById('unique-visitors');
const elTotalVisits     = document.getElementById('total-visits');
const elUniqueCountries = document.getElementById('unique-countries');
const elCountryList     = document.getElementById('country-list');
const elCitySection     = document.getElementById('city-section');
const elCityList        = document.getElementById('city-list');
const elBrowserList     = document.getElementById('browser-list');
const elDeviceList      = document.getElementById('device-list');
const elOsList          = document.getElementById('os-list');
const elReferrerList    = document.getElementById('referrer-list');

export function renderBarList(container, items, nameKey, activeValue, iconFn) {
    if (items.length === 0) {
        container.innerHTML = '<p class="empty-state">No data recorded yet.</p>';
        return;
    }
    const maxCount = items[0].count || 1;
    const isClickable = ['country', 'city', 'browser', 'device_type', 'os'].includes(nameKey);

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
            '  <div class="bar-wrap"><div class="bar-fill" style="width:' + pct + '%"></div></div>' +
            '  <span class="country-count">' + formatNumber(item.count) + '</span>' +
            '</div>'
        );
    }).join('');
}

export function renderStats(data) {
    if (!data) return;
    if (elUniqueVisitors)  elUniqueVisitors.textContent  = formatNumber(data.unique_visitors  || 0);
    if (elTotalVisits)     elTotalVisits.textContent     = formatNumber(data.total_visits      || 0);
    if (elUniqueCountries) elUniqueCountries.textContent = formatNumber(data.unique_countries  || 0);

    const elAvgTime = document.getElementById('avg-time-on-page');
    if (elAvgTime) {
        const avg = data.avg_time_on_page || 0;
        elAvgTime.textContent = avg > 0 ? formatDuration(avg) : '—';
    }

    if (elCountryList) renderBarList(elCountryList, data.top_countries || [], 'country',     state.country,     null);
    if (elCitySection) elCitySection.style.display = (data.top_cities || []).length > 0 ? '' : 'none';
    if (elCityList)    renderBarList(elCityList,    data.top_cities   || [], 'city',         state.city,        null);
    if (elBrowserList)  renderBarList(elBrowserList,  data.browsers     || [], 'browser',      state.browser,    browserIcon);
    if (elDeviceList)   renderBarList(elDeviceList,   data.device_types || [], 'device_type',  state.deviceType, deviceIcon);
    if (elOsList)       renderBarList(elOsList,        data.os          || [], 'os',           state.os,         osIcon);
    if (elReferrerList) renderBarList(elReferrerList,  data.referrers   || [], 'referrer',     null,             null);

    setOk();
}
