document.getElementById('cultivoSelect').addEventListener('change', async function() {
    const cultivo = this.value;
    try {
        const response = await fetch(`/api/cultivo/${cultivo}`);
        const datos = await response.json();

        const datosDiv = document.getElementById('datosCultivo');
        datosDiv.className = 'datos-cultivo'; // Asegurar que la clase persiste


        if (datos.length > 0) {
            const cultivoData = datos[0];
            datosDiv.innerHTML = `
                <h2>${cultivoData.planta}</h2>
                <dl>
                    <dt>Ciclo:</dt>
                    <dd>${cultivoData.ciclo}</dd>
                    <dt>Época:</dt>
                    <dd>${cultivoData.epoca}</dd>
                    <dt>Distancia entre plantas:</dt>
                    <dd>${cultivoData.distancia}</dd>
                    <dt>Suelos recomendados:</dt>
                    <dd>${cultivoData.suelos}</dd>
                    <dt>pH mínimo:</dt>
                    <dd>${cultivoData.phmin}</dd>
                    <dt>pH máximo:</dt>
                    <dd>${cultivoData.phmax}</dd>
                    <dt>Temperatura mínima:</dt>
                    <dd>${cultivoData.tmin} °C</dd>
                    <dt>Temperatura máxima:</dt>
                    <dd>${cultivoData.tmax} °C</dd>
                </dl>
            `;

        } else {
            datosDiv.innerHTML = `<p>No se encontraron datos para este cultivo.</p>`;
        }
    } catch (error) {
        console.error("Error al cargar los datos del cultivo:", error);
        document.getElementById('datosCultivo').innerHTML = `<p>Hubo un problema al cargar los datos. Intenta más tarde.</p>`;
    }
});

