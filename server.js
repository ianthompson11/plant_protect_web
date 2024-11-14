const express = require('express');
const bodyParser = require('body-parser');
const http = require('http');
const WebSocket = require('ws');
const fs = require('fs');
const pool = require('./db'); // Asegúrate de que el archivo db.js esté en la misma carpeta

const app = express();
const PORT = 3000;

app.use(bodyParser.json());
app.use(express.static('public'));

const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

let lastData = {}; // Variable para almacenar los últimos datos recibidos
const dataFilePath = './data.json'; // Ruta del archivo JSON donde guardaremos los datos

// Función para obtener los datos del cultivo
async function obtenerDatosCultivo(cultivo) {
    const query = `
        SELECT planta, ciclo, epoca, distancia, suelos, phmin, phmax, tmin, tmax
        FROM cultivos
        WHERE planta ILIKE $1
    `;
    const res = await pool.query(query, [cultivo]);
    return res.rows; // Retorna los resultados de la consulta
}


// Ruta para manejar las solicitudes POST a /data
app.post('/data', (req, res) => {
    console.log('Datos recibidos:', req.body);
    lastData = req.body;

    // Guarda los datos en el archivo JSON
    let jsonData = [];
    if (fs.existsSync(dataFilePath)) {
        const fileContent = fs.readFileSync(dataFilePath, 'utf8');
        if (fileContent) {
            jsonData = JSON.parse(fileContent);
        }
    }

    
    // Agrega el nuevo dato al array
        jsonData.push(lastData);

        // Función para guardar los datos en JSON y en la base de datos
    async function guardarDatos(datos) {
        // Insertar en la base de datos con los nombres correctos
        const query = `
            INSERT INTO datos_recibidos (distance, temperatura, humedad, luminosidad, humedad_suelo, contador)
            VALUES ($1, $2, $3, $4, $5, $6)
        `;
        const valores = [
            datos.distance,         // Mapea correctamente el campo distance
            datos.temperatura,      // Mapea correctamente el campo temperatura
            datos.humedad,          // Mapea correctamente el campo humedad
            datos.luminosidad,      // Mapea correctamente el campo luminosidad
            datos.humedadSuelo,     // Mapea correctamente el campo humedadSuelo (con "S" mayúscula)
            datos.contador          // Mapea correctamente el campo contador
        ];

        // Verifica los valores antes de ejecutarlos
        console.log('Valores a insertar:', valores);
        console.log(datos.distance);

        try {
            await pool.query(query, valores);
            console.log('Datos guardados en la base de datos');
        } catch (error) {
            console.error('Error al guardar en la base de datos:', error);
        }
    }


    //console.log(lastData);




    // Llamada a la función para guardar en ambos lugares
    guardarDatos(lastData);

    // Enviar los datos a todos los clientes conectados por WebSocket
    wss.clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(JSON.stringify(lastData));
        }
    });

    // Responder al ESP32
    res.status(200).send('Datos recibidos correctamente');
});

// Ruta para obtener los datos del cultivo
app.get('/api/cultivo/:cultivo', async (req, res) => {
    const cultivo = req.params.cultivo; // Obtiene el nombre del cultivo de la URL
    console.log('Cultivo solicitado:', cultivo);  // Agrega este log para verificar

    try {
        const datosCultivo = await obtenerDatosCultivo(cultivo); // Llama a la función para obtener los datos
        console.log('Datos del cultivo:', datosCultivo);  // Verifica lo que se obtiene de la base de datos
        if (datosCultivo.length > 0) {
            res.json(datosCultivo); // Devuelve los datos en formato JSON
        } else {
            res.status(404).json({ message: 'Cultivo no encontrado' });
        }
    } catch (error) {
        console.error('Error al obtener los datos del cultivo:', error);
        res.status(500).json({ message: 'Error al obtener los datos del cultivo' });
    }
});


// Ruta para manejar la solicitud GET a /data
app.get('/data', (req, res) => {
    res.json(lastData); // Devuelve los últimos datos recibidos en formato JSON
});

// Ruta para manejar la solicitud GET a la raíz
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/public/index.html');
});

// Iniciar el servidor
server.listen(PORT, () => {
    console.log(`Servidor escuchando en http://localhost:${PORT}`);
});
