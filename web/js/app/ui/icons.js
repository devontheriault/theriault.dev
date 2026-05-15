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

export function browserIcon(name) { return BROWSER_ICONS[name] || 'fa-solid fa-globe'; }
export function osIcon(name)      { return OS_ICONS[name]       || 'fa-solid fa-circle-question'; }
export function deviceIcon(name)  { return DEVICE_ICONS[name]   || 'fa-solid fa-desktop'; }
