// script.js
function irASeccion(id) {
    // Oculta todas las secciones
    const secciones = document.querySelectorAll('.seccion');
    secciones.forEach(seccion => seccion.classList.remove('visible'));

    // Muestra la sección seleccionada
    const seccionActiva = document.getElementById(id);
    if (seccionActiva) {
        seccionActiva.classList.add('visible');
    }
}
function abrirVentanaSensores() {
    window.location.href = "sensores.html"; // Redirige a la página sensores.html
}