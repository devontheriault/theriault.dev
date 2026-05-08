(function () {
    const canvas = document.getElementById('bg-canvas');
    const ctx = canvas.getContext('2d');

    const DOTS_PER_PX = 1 / 10000;
    const BASE_SPEED = 0.15;
    const MOUSE_RADIUS = 140;
    const MOUSE_RADIUS_SQ = MOUSE_RADIUS * MOUSE_RADIUS;
    const MOUSE_FORCE = 5;
    const CONNECT_DIST = 160;
    const CONNECT_DIST_SQ = CONNECT_DIST * CONNECT_DIST;
    const GRAB_RADIUS = 18;
    const R = 88, G = 166, B = 255;
    const TAU = Math.PI * 2;
    const MAX_PACKETS = 20;

    const TIERS = [
        { r: 2.5, weight: 60 },
        { r: 4.5, weight: 30 },
        { r: 7,   weight: 10 },
    ];
    const TIER_TOTAL = TIERS.reduce((s, t) => s + t.weight, 0);

    function pickTier() {
        let roll = Math.random() * TIER_TOTAL;
        for (const t of TIERS) { if ((roll -= t.weight) < 0) return t; }
        return TIERS[0];
    }

    let W, H;
    let mouse = { x: -9999, y: -9999 };
    let gridCols, gridRows, grid = [];
    let dots = [];
    let packets = [];

    // Drag state
    let dragged = null;
    let prevMX = 0, prevMY = 0;
    let dragVX = 0, dragVY = 0;

    function makeDot() {
        const tier = pickTier();
        return {
            x: Math.random() * W,
            y: Math.random() * H,
            vx: (Math.random() - 0.5) * BASE_SPEED * 2,
            vy: (Math.random() - 0.5) * BASE_SPEED * 2,
            r: tier.r,
            proximity: 0,
            grabbed: false,
        };
    }

    function resize() {
        W = canvas.width = window.innerWidth;
        H = canvas.height = window.innerHeight;
        gridCols = Math.ceil(W / CONNECT_DIST) + 1;
        gridRows = Math.ceil(H / CONNECT_DIST) + 1;
        dots = Array.from({ length: Math.round(W * H * DOTS_PER_PX) }, makeDot);
        dragged = null;
        packets = [];
    }

    function dotAtPoint(mx, my) {
        let best = null, bestDist = Infinity;
        for (const d of dots) {
            const dx = d.x - mx, dy = d.y - my;
            const dist = Math.sqrt(dx * dx + dy * dy);
            if (dist < d.r + GRAB_RADIUS && dist < bestDist) {
                best = d;
                bestDist = dist;
            }
        }
        return best;
    }

    window.addEventListener('mousedown', e => {
        const hit = dotAtPoint(e.clientX, e.clientY);
        if (hit) {
            dragged = hit;
            dragged.grabbed = true;
            dragged.vx = 0;
            dragged.vy = 0;
            prevMX = e.clientX;
            prevMY = e.clientY;
            dragVX = 0;
            dragVY = 0;
        }
    });

    window.addEventListener('mousemove', e => {
        dragVX = e.clientX - prevMX;
        dragVY = e.clientY - prevMY;
        prevMX = e.clientX;
        prevMY = e.clientY;
        mouse.x = e.clientX;
        mouse.y = e.clientY;

        if (dragged) {
            dragged.x = e.clientX;
            dragged.y = e.clientY;
        }
    });

    window.addEventListener('mouseup', () => {
        if (dragged) {
            dragged.vx = dragVX * 0.4;
            dragged.vy = dragVY * 0.4;
            dragged.grabbed = false;
            dragged = null;
        }
    });

    // Update cursor when hovering over a draggable dot
    window.addEventListener('mousemove', e => {
        if (dragged) { document.body.style.cursor = 'grabbing'; return; }
        document.body.style.cursor = dotAtPoint(e.clientX, e.clientY) ? 'grab' : '';
    });

    window.addEventListener('mouseleave', () => { mouse.x = -9999; mouse.y = -9999; });
    window.addEventListener('resize', resize);
    resize();

    function buildGrid() {
        const total = gridCols * gridRows;
        while (grid.length < total) grid.push([]);
        for (let i = 0; i < total; i++) grid[i].length = 0;
        for (let i = 0; i < dots.length; i++) {
            const d = dots[i];
            const cx = (d.x / CONNECT_DIST) | 0;
            const cy = (d.y / CONNECT_DIST) | 0;
            if (cx >= 0 && cx < gridCols && cy >= 0 && cy < gridRows)
                grid[cy * gridCols + cx].push(i);
        }
    }

    function step() {
        ctx.clearRect(0, 0, W, H);

        for (let i = 0; i < dots.length; i++) {
            const d = dots[i];

            if (d.grabbed) {
                d.proximity = 1;
                continue;
            }

            const dx = d.x - mouse.x, dy = d.y - mouse.y;
            const distSq = dx * dx + dy * dy;

            if (distSq < MOUSE_RADIUS_SQ && distSq > 0) {
                const dist = Math.sqrt(distSq);
                const strength = (1 - dist / MOUSE_RADIUS) * MOUSE_FORCE;
                d.vx += (dx / dist) * strength * 0.05;
                d.vy += (dy / dist) * strength * 0.05;
                d.proximity = 1 - dist / MOUSE_RADIUS;
            } else {
                d.proximity = 0;
            }

            d.vx *= 0.98;
            d.vy *= 0.98;
            const spd = Math.sqrt(d.vx * d.vx + d.vy * d.vy);
            if (spd > BASE_SPEED * 4) {
                const s = (BASE_SPEED * 4) / spd;
                d.vx *= s; d.vy *= s;
            } else if (spd < BASE_SPEED * 0.3 && d.proximity === 0) {
                d.vx += (Math.random() - 0.5) * 0.03;
                d.vy += (Math.random() - 0.5) * 0.03;
            }

            d.x += d.vx;
            d.y += d.vy;
            if (d.x < -d.r) d.x = W + d.r;
            else if (d.x > W + d.r) d.x = -d.r;
            if (d.y < -d.r) d.y = H + d.r;
            else if (d.y > H + d.r) d.y = -d.r;
        }

        buildGrid();

        // Edges
        ctx.lineWidth = 1;
        ctx.beginPath();
        for (let cy = 0; cy < gridRows; cy++) {
            for (let cx = 0; cx < gridCols; cx++) {
                const cell = grid[cy * gridCols + cx];
                if (!cell.length) continue;
                for (let a = 0; a < cell.length; a++) {
                    const di = dots[cell[a]];
                    for (let b = a + 1; b < cell.length; b++) {
                        const dj = dots[cell[b]];
                        const ex = di.x - dj.x, ey = di.y - dj.y;
                        if (ex * ex + ey * ey < CONNECT_DIST_SQ) {
                            ctx.moveTo(di.x, di.y); ctx.lineTo(dj.x, dj.y);
                            if (packets.length < MAX_PACKETS && Math.random() < 0.003)
                                packets.push({ a: di, b: dj, t: 0, speed: 0.007 + Math.random() * 0.009 });
                        }
                    }
                    const neighbors = [cx+1,cy, cx-1,cy+1, cx,cy+1, cx+1,cy+1];
                    for (let n = 0; n < neighbors.length; n += 2) {
                        const nx = neighbors[n], ny = neighbors[n+1];
                        if (nx < 0 || nx >= gridCols || ny >= gridRows) continue;
                        const nc = grid[ny * gridCols + nx];
                        for (let b = 0; b < nc.length; b++) {
                            const dj = dots[nc[b]];
                            const ex = di.x - dj.x, ey = di.y - dj.y;
                            if (ex * ex + ey * ey < CONNECT_DIST_SQ) {
                                ctx.moveTo(di.x, di.y); ctx.lineTo(dj.x, dj.y);
                                if (packets.length < MAX_PACKETS && Math.random() < 0.003)
                                    packets.push({ a: di, b: dj, t: 0, speed: 0.007 + Math.random() * 0.009 });
                            }
                        }
                    }
                }
            }
        }
        ctx.strokeStyle = `rgba(${R},${G},${B},0.18)`;
        ctx.stroke();

        // Packets (network traffic)
        for (let i = packets.length - 1; i >= 0; i--) {
            const p = packets[i];
            p.t += p.speed;
            if (p.t >= 1) { packets.splice(i, 1); continue; }
            for (let s = 4; s >= 0; s--) {
                const st = Math.max(0, p.t - s * 0.028);
                const px = p.a.x + (p.b.x - p.a.x) * st;
                const py = p.a.y + (p.b.y - p.a.y) * st;
                const frac = 1 - s / 5;
                ctx.beginPath();
                ctx.arc(px, py, 2.5 * frac, 0, TAU);
                ctx.fillStyle = `rgba(${R},${G},${B},${0.9 * frac})`;
                ctx.shadowColor = `rgba(${R},${G},${B},${0.7 * frac})`;
                ctx.shadowBlur = 7 * frac;
                ctx.fill();
            }
            ctx.shadowBlur = 0;
            ctx.shadowColor = 'transparent';
        }

        // Nodes
        for (let i = 0; i < dots.length; i++) {
            const d = dots[i];
            const p = d.proximity;
            const r = d.r + p * 3;
            const fillAlpha = 0.18 + p * 0.5;
            const strokeAlpha = 0.55 + p * 0.45;

            if (d.r >= 7 || p > 0) {
                ctx.shadowColor = `rgba(${R},${G},${B},${0.35 + p * 0.4})`;
                ctx.shadowBlur = d.grabbed ? r * 4 : r * 2.5;
            }

            ctx.beginPath();
            ctx.arc(d.x, d.y, r, 0, TAU);
            ctx.fillStyle = `rgba(${R},${G},${B},${fillAlpha})`;
            ctx.fill();
            ctx.strokeStyle = `rgba(${R},${G},${B},${strokeAlpha})`;
            ctx.lineWidth = d.grabbed ? 2 : (d.r >= 4.5 ? 1.5 : 1);
            ctx.stroke();

            if (d.r >= 7 || p > 0) {
                ctx.shadowBlur = 0;
                ctx.shadowColor = 'transparent';
            }
        }

        requestAnimationFrame(step);
    }

    step();
})();
