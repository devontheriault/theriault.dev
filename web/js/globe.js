'use strict';

(function () {

    var globeEl = document.getElementById('globe-container');
    if (!globeEl || typeof Globe === 'undefined') {
        console.error('[globe] Globe library or container missing');
        return;
    }

    /* Heat color: t=0 cool blue, t=1 hot red */
    function heatColor(t) {
        t = Math.max(0, Math.min(1, t));
        var r, g, b;
        if (t < 0.5) {
            var s = t / 0.5;
            r = Math.round(31  + (247 - 31)  * s);
            g = Math.round(111 + (201 - 111) * s);
            b = Math.round(235 + (72  - 235) * s);
        } else {
            var s2 = (t - 0.5) / 0.5;
            r = Math.round(247 + (255 - 247) * s2);
            g = Math.round(201 + (32  - 201) * s2);
            b = Math.round(72  + (32  - 72)  * s2);
        }
        return 'rgb(' + r + ',' + g + ',' + b + ')';
    }

    var W = globeEl.clientWidth  || 800;
    var H = globeEl.clientHeight || 500;

    var myGlobe = Globe({ animateIn: false })
        .width(W)
        .height(H)
        .globeImageUrl('https://unpkg.com/three-globe/example/img/earth-dark.jpg')
        .backgroundImageUrl('https://unpkg.com/three-globe/example/img/night-sky.png')
        .pointsData([])
        .pointLat('lat')
        .pointLng('lng')
        .pointColor('color')
        .pointAltitude('altitude')
        .pointRadius('radius')
        .pointLabel(function (d) {
            if (!d.isHover) return '';
            var label = d.country ? d.city + ', ' + d.country : d.city;
            return '<div style="font-size:0.8rem;background:#1c2230;padding:4px 8px;border-radius:6px;border:1px solid #30363d">' +
                '<strong>' + label + '</strong><br>' + d.count.toLocaleString() + ' visits</div>';
        })
        .ringsData([])
        .ringLat('lat')
        .ringLng('lng')
        .ringColor(function (d) { return d.color; })
        .ringMaxRadius(function (d) { return d.maxRadius; })
        .ringPropagationSpeed(function (d) { return d.speed; })
        .ringRepeatPeriod(function (d) { return d.period; })
        (globeEl);

    myGlobe.controls().autoRotate      = true;
    myGlobe.controls().autoRotateSpeed = 0.06;
    myGlobe.pointOfView({ lat: 48, lng: -100, altitude: 2.5 });

    // Keep globe sized to its container (fixes raycaster drift on resize/orientation change)
    if (typeof ResizeObserver !== 'undefined') {
        new ResizeObserver(function (entries) {
            var rect = entries[0].contentRect;
            if (rect.width > 0 && rect.height > 0) {
                myGlobe.width(rect.width).height(rect.height);
            }
        }).observe(globeEl);
    }

    // Touch: show label on tap (tablets have no mousemove hover)
    myGlobe.onPointClick(function (point) {
        if (!point || !point.isHover) return;
        var label = point.country ? point.city + ', ' + point.country : point.city;
        var html = '<div style="font-size:0.8rem;background:#1c2230;padding:4px 8px;border-radius:6px;border:1px solid #30363d">' +
            '<strong>' + label + '</strong><br>' + point.count.toLocaleString() + ' visits</div>';
        var existing = globeEl.querySelector('.globe-tap-label');
        if (existing) existing.remove();
        var el = document.createElement('div');
        el.className = 'globe-tap-label';
        el.innerHTML = html;
        el.style.cssText = 'position:absolute;bottom:12px;left:50%;transform:translateX(-50%);pointer-events:none;z-index:10;';
        globeEl.appendChild(el);
        clearTimeout(myGlobe._tapLabelTimer);
        myGlobe._tapLabelTimer = setTimeout(function () { el.remove(); }, 3000);
    });

    if (typeof topojson !== 'undefined') {
        fetch('https://unpkg.com/world-atlas@2.0.2/countries-110m.json')
            .then(function (r) { return r.json(); })
            .then(function (topo) {
                var features = topojson.feature(topo, topo.objects.countries).features;
                myGlobe
                    .polygonsData(features)
                    .polygonCapColor(function () { return 'rgba(0,0,0,0)'; })
                    .polygonSideColor(function () { return 'rgba(0,0,0,0)'; })
                    .polygonStrokeColor(function () { return '#30363d'; })
                    .polygonAltitude(0.001);
            })
            .catch(function (e) { console.warn('[globe] country outlines failed:', e); });
    }

    function dismissLoader() {
        var loader = document.getElementById('globe-loader');
        if (loader) {
            loader.classList.add('hidden');
            setTimeout(function () { loader.remove(); }, 650);
        }
    }

    function updateGlobe(data) {
        var cities = data.cities || [];
        if (!cities.length) return;

        dismissLoader();

        var maxCount = cities.reduce(function (m, c) { return Math.max(m, c.count); }, 1);

        var dots  = [];
        var rings = [];

        cities.forEach(function (c) {
            if (!c.lat && !c.lng) return;
            var t = Math.log1p(c.count) / Math.log1p(maxCount);
            var color = heatColor(t);
            var base = {
                lat:     c.lat,
                lng:     c.lng,
                city:    c.city,
                country: c.country || '',
                count:   c.count,
                color:   color,
            };
            // Large nearly-invisible dot at higher altitude — intercepts raycaster for hover
            dots.push(Object.assign({}, base, {
                radius: 0.45 + t * 0.7,
                altitude: 0.006,
                color: color.replace('rgb(', 'rgba(').replace(')', ',0.04)'),
                isHover: true,
            }));
            // Small bright dot at lower altitude — visual only
            dots.push(Object.assign({}, base, {
                radius: 0.18 + t * 0.28,
                altitude: 0.001,
                isHover: false,
            }));
            rings.push(Object.assign({}, base, {
                maxRadius: 1.5 + t * 3.5,
                speed:     0.8 + t * 1.5,
                period:    2500 - t * 1500,
            }));
        });

        myGlobe.pointsData(dots).ringsData(rings);
    }

    var _globeReady = false;

    function fetchGlobe(attempt) {
        attempt = attempt || 1;
        fetch('/globe')
            .then(function (r) {
                if (!r.ok) throw new Error('HTTP ' + r.status);
                return r.json();
            })
            .then(function (data) {
                console.log('[globe] fetchGlobe data:', data);
                if (data.cities && data.cities.length) {
                    updateGlobe(data);
                } else if (attempt < 5) {
                    setTimeout(function () { fetchGlobe(attempt + 1); }, 2000 * attempt);
                }
            })
            .catch(function (e) {
                console.error('[globe] fetch failed:', e);
                if (attempt < 5) {
                    setTimeout(function () { fetchGlobe(attempt + 1); }, 2000 * attempt);
                }
            });
    }

    // Safety net: dismiss loader after 10s regardless, so the globe is never permanently hidden
    setTimeout(dismissLoader, 10000);

    window._globeUpdateFn = function (data) {
        if (_globeReady) updateGlobe(data);
        else window._pendingGlobeData = data;
    };

    myGlobe.onGlobeReady(function () {
        _globeReady = true;
        fetchGlobe();
        if (window._pendingGlobeData) {
            updateGlobe(window._pendingGlobeData);
            window._pendingGlobeData = null;
        }
    });

}());
