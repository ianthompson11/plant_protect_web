const express = require('express');
const bodyParser = require('body-parser');
const http = require('http');
const WebSocket = require('ws');

const app = express();
const PORT = 3000;

// Middleware para analizar el cuerpo de las solicitudes JSON
app.use(bodyParser.json());
app.use(express.static('public')); // Sirve archivos estáticos desde la carpeta 'public'

// Inicializar un servidor HTTP
const server = http.createServer(app);

// Inicializar WebSocket
const wss = new WebSocket.Server({ server });

let lastData = {}; // Variable para almacenar los últimos datos recibidos

// Ruta para manejar las solicitudes POST a /data
app.post('/data', (req, res) => {
    console.log('Datos recibidos:', req.body);
    lastData = req.body; // Actualizar los últimos datos

    // Enviar los datos a todos los clientes conectados por WebSocket
    wss.clients.forEach(client => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(JSON.stringify(lastData));
        }
    });

    // Responder al ESP32
    res.status(200).send('Datos recibidos correctamente');
});

// Ruta para manejar la solicitud GET a /data
app.get('/data', (req, res) => {
    res.json(lastData); // Devuelve los últimos datos recibidos en formato JSON
});

// Ruta para manejar la solicitud GET a la raíz
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/public/index.html'); // Servir el archivo HTML
});

// Iniciar el servidor
server.listen(PORT, () => {
    console.log(`Servidor escuchando en http://localhost:${PORT}`);
});


