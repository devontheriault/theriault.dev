import { showTooltip, hideTooltip } from '../render/tooltips.js';
import { escapeHtml, formatNumber } from '../ui/helpers.js';

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

export function bindTooltipEvents() {
    const elCountryList = document.getElementById('country-list');
    const elCityList    = document.getElementById('city-list');
    const elBrowserList = document.getElementById('browser-list');
    const elDeviceList  = document.getElementById('device-list');
    const elOsList      = document.getElementById('os-list');
    const elHeatmapGrid = document.getElementById('heatmap-grid');

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

    document.querySelectorAll('.card[data-tooltip]').forEach(function(card) {
        card.addEventListener('mouseenter', function(e) { showTooltip(card.dataset.tooltip, e); });
        card.addEventListener('mouseleave', hideTooltip);
        card.addEventListener('touchend', hideTooltip);
    });
}
