export function initBackground() {
    const bg = document.getElementById('bg-gradient');
    if (!bg) return;
    const pull = 0.4;
    const ease = 0.04;
    let cx = 50, cy = 50, tx = 50, ty = 50;

    document.addEventListener('mousemove', function(e) {
        tx = 50 + (e.clientX / window.innerWidth  * 100 - 50) * pull;
        ty = 50 + (e.clientY / window.innerHeight * 100 - 50) * pull;
    });

    (function animate() {
        cx += (tx - cx) * ease;
        cy += (ty - cy) * ease;
        bg.style.background = `radial-gradient(circle at ${cx.toFixed(2)}% ${cy.toFixed(2)}%, #1e3a6e 0%, #0f1f3d 20%, #0d1117 40%, #080c12 100%)`;
        requestAnimationFrame(animate);
    }());
}
