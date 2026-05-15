import { state } from '../state.js';
import { refreshAll } from '../refresh.js';

export function bindFilterEvents() {
    const elCountryList         = document.getElementById('country-list');
    const elCityList            = document.getElementById('city-list');
    const elBrowserList         = document.getElementById('browser-list');
    const elDeviceList          = document.getElementById('device-list');
    const elOsList              = document.getElementById('os-list');
    const elFilterClear         = document.getElementById('filter-clear');
    const elHeatmapCountryClear = document.getElementById('heatmap-country-clear');
    const elHeatmapCityClear    = document.getElementById('heatmap-city-clear');
    const elBrowserClear        = document.getElementById('browser-clear');
    const elDeviceClear         = document.getElementById('device-clear');
    const elOsClear             = document.getElementById('os-clear');

    if (elCountryList) {
        elCountryList.addEventListener('click', function(e) {
            const item = e.target.closest('.country-item.clickable');
            if (!item) return;
            const country = item.getAttribute('data-country');
            state.country = (state.country === country) ? null : country;
            state.city = null;
            refreshAll();
        });
    }

    if (elCityList) {
        elCityList.addEventListener('click', function(e) {
            const item = e.target.closest('.country-item.clickable');
            if (!item) return;
            const city = item.getAttribute('data-city');
            state.city = (state.city === city) ? null : city;
            refreshAll();
        });
    }

    if (elBrowserList) {
        elBrowserList.addEventListener('click', function(e) {
            const item = e.target.closest('.country-item.clickable');
            if (!item) return;
            state.browser = (state.browser === item.getAttribute('data-browser')) ? null : item.getAttribute('data-browser');
            refreshAll();
        });
    }

    if (elDeviceList) {
        elDeviceList.addEventListener('click', function(e) {
            const item = e.target.closest('.country-item.clickable');
            if (!item) return;
            const val = item.getAttribute('data-device_type');
            state.deviceType = (state.deviceType === val) ? null : val;
            refreshAll();
        });
    }

    if (elOsList) {
        elOsList.addEventListener('click', function(e) {
            const item = e.target.closest('.country-item.clickable');
            if (!item) return;
            state.os = (state.os === item.getAttribute('data-os')) ? null : item.getAttribute('data-os');
            refreshAll();
        });
    }

    if (elFilterClear) {
        elFilterClear.addEventListener('click', function() {
            state.country = null;
            state.city = null;
            refreshAll();
        });
    }

    if (elHeatmapCountryClear) {
        elHeatmapCountryClear.addEventListener('click', function() {
            state.country = null;
            state.city = null;
            refreshAll();
        });
    }

    if (elHeatmapCityClear) {
        elHeatmapCityClear.addEventListener('click', function() {
            state.city = null;
            refreshAll();
        });
    }

    if (elBrowserClear) {
        elBrowserClear.addEventListener('click', function() {
            state.browser = null;
            refreshAll();
        });
    }

    if (elDeviceClear) {
        elDeviceClear.addEventListener('click', function() {
            state.deviceType = null;
            refreshAll();
        });
    }

    if (elOsClear) {
        elOsClear.addEventListener('click', function() {
            state.os = null;
            refreshAll();
        });
    }
}
