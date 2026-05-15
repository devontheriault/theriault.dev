import { state } from '../state.js';
import { updateHeatmapHighlight } from './heatmap.js';

const DOW_NAMES = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];

const elFilterClear         = document.getElementById('filter-clear');
const elFilterLabel         = document.getElementById('filter-label');
const elHeatmapCountryClear = document.getElementById('heatmap-country-clear');
const elHeatmapCountryLabel = document.getElementById('heatmap-country-label');
const elHeatmapCityClear    = document.getElementById('heatmap-city-clear');
const elHeatmapCityLabel    = document.getElementById('heatmap-city-label');
const elHeatmapDowClear     = document.getElementById('heatmap-dow-clear');
const elHeatmapDowLabel     = document.getElementById('heatmap-dow-label');
const elHeatmapHourClear    = document.getElementById('heatmap-hour-clear');
const elHeatmapHourLabel    = document.getElementById('heatmap-hour-label');
const elBrowserClear        = document.getElementById('browser-clear');
const elBrowserLabel        = document.getElementById('browser-label');
const elDeviceClear         = document.getElementById('device-clear');
const elDeviceLabel         = document.getElementById('device-label');
const elOsClear             = document.getElementById('os-clear');
const elOsLabel             = document.getElementById('os-label');
const elCityList            = document.getElementById('city-list');
const elBrowserList         = document.getElementById('browser-list');
const elDeviceList          = document.getElementById('device-list');
const elOsList              = document.getElementById('os-list');

function badge(clearEl, labelEl, value) {
    if (!clearEl || !labelEl) return;
    clearEl.style.display = value ? '' : 'none';
    labelEl.textContent   = value || '';
}

export function updateFilterUI() {
    badge(elFilterClear,         elFilterLabel,         state.country);
    badge(elHeatmapCountryClear, elHeatmapCountryLabel, state.country);
    badge(elHeatmapCityClear,    elHeatmapCityLabel,    state.city);
    badge(elBrowserClear,        elBrowserLabel,        state.browser);
    badge(elDeviceClear,         elDeviceLabel,         state.deviceType);
    badge(elOsClear,             elOsLabel,             state.os);

    if (elHeatmapDowClear && elHeatmapDowLabel) {
        elHeatmapDowClear.style.display = state.dow !== null ? '' : 'none';
        elHeatmapDowLabel.textContent   = state.dow !== null ? DOW_NAMES[state.dow] : '';
    }
    if (elHeatmapHourClear && elHeatmapHourLabel) {
        elHeatmapHourClear.style.display = state.hour !== null ? '' : 'none';
        elHeatmapHourLabel.textContent   = state.hour !== null
            ? String(state.hour).padStart(2, '0') + ':00' : '';
    }

    if (elCityList) {
        elCityList.querySelectorAll('.country-item').forEach(function(el) {
            el.classList.toggle('active', el.getAttribute('data-city') === state.city);
        });
    }
    if (elBrowserList) {
        elBrowserList.querySelectorAll('.country-item').forEach(function(el) {
            el.classList.toggle('active', el.getAttribute('data-browser') === state.browser);
        });
    }
    if (elDeviceList) {
        elDeviceList.querySelectorAll('.country-item').forEach(function(el) {
            el.classList.toggle('active', el.getAttribute('data-device_type') === state.deviceType);
        });
    }
    if (elOsList) {
        elOsList.querySelectorAll('.country-item').forEach(function(el) {
            el.classList.toggle('active', el.getAttribute('data-os') === state.os);
        });
    }

    updateHeatmapHighlight();
}
