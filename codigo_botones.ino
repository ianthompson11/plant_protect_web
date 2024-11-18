#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

// Configuración Wi-Fi
const char* ssid = "Google Wifi";
const char* password = ":B#hWe6u$8:J/e}<mEBhigwGU";

// Pines para los relés
#define relay1_pin 13
#define relay2_pin 12
#define relay3_pin 14
#define relay4_pin 27

// Pin para verificar la señal
#define signal_pin 26

// Estados iniciales de los relés
bool relay1_state = false;
bool relay2_state = false;
bool relay3_state = false;
bool relay4_state = false;

// Control de prioridad para el relay 4
bool relay4_manual_control = false;  // Control manual (desde el botón) o automático (desde pin 26)

// Crear el servidor web en el puerto 80
WebServer server(80);

// Función para manejar la página web principal
void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Control de Relés</title>
    <style>
      body { font-family: Arial; text-align: center; margin: 0; padding: 0; }
      button { margin: 10px; padding: 10px 20px; font-size: 16px; }
    </style>
  </head>
  <body>
    <h1>Control de Relés</h1>
    <button onclick="toggleRelay(1)">Relay 1: %RELAY1%</button>
    <button onclick="toggleRelay(2)">Relay 2: %RELAY2%</button>
    <button onclick="toggleRelay(3)">Relay 3: %RELAY3%</button>
    <button onclick="toggleRelay(4)">Relay 4: %RELAY4%</button>
    <br><br>
    <button onclick="toggleMode()">Modo: %MODE%</button>
    <br><br>
    <p>Estado de la entrada en pin 26: %SIGNAL%</p> <!-- Mostrar el estado del pin 26 -->
    <script>
      function toggleRelay(relay) {
        fetch('/toggle?relay=' + relay)
          .then(() => location.reload());
      }

      function toggleMode() {
        fetch('/toggleMode')
          .then(response => {
            if (response.ok) {
              location.reload(); // Recarga la página después de la solicitud exitosa
            }
          });
      }
    </script>
  </body>
  </html>
  )rawliteral";

  // Obtener el estado del pin de señal
  String signalStatus = (digitalRead(signal_pin) == HIGH) ? "ALTA" : "BAJA"; // Leer el estado del pin

  // Reemplazar los marcadores con el estado actual de los relés y el pin
  html.replace("%RELAY1%", relay1_state ? "ON" : "OFF");
  html.replace("%RELAY2%", relay2_state ? "ON" : "OFF");
  html.replace("%RELAY3%", relay3_state ? "ON" : "OFF");
  html.replace("%RELAY4%", relay4_state ? "ON" : "OFF");
  html.replace("%MODE%", relay4_manual_control ? "Manual" : "Automático");
  html.replace("%SIGNAL%", signalStatus); // Reemplazar el marcador con el estado del pin

  server.send(200, "text/html", html);
}


// Función para configurar OTA
void setupOTA() {
  ArduinoOTA.setPassword("123456789");
  ArduinoOTA.onStart([]() {
    String type = ArduinoOTA.getCommand() == U_FLASH ? "Sketch" : "Filesystem";
    Serial.println("Inicio de actualización OTA: " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nActualización OTA completada.");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progreso: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Error de autenticación");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Error al comenzar");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Error de conexión");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Error de recepción");
    else if (error == OTA_END_ERROR) Serial.println("Error al finalizar");
  });
  ArduinoOTA.begin();
  Serial.println("OTA listo para recibir actualizaciones.");
}

// Función para manejar el cambio de estado de los relés
void handleToggle() {
  if (server.hasArg("relay")) {
    int relay = server.arg("relay").toInt();
    switch (relay) {
      case 1:
        relay1_state = !relay1_state;
        digitalWrite(relay1_pin, relay1_state ? LOW : HIGH);
        break;
      case 2:
        relay2_state = !relay2_state;
        digitalWrite(relay2_pin, relay2_state ? LOW : HIGH);
        break;
      case 3:
        relay3_state = !relay3_state;
        digitalWrite(relay3_pin, relay3_state ? LOW : HIGH);
        break;
      case 4:
        relay4_state = !relay4_state;
        digitalWrite(relay4_pin, relay4_state ? LOW : HIGH);
        break;
    }
  }
  server.send(204, "text/plain", ""); // Respuesta vacía
}

// Función para alternar entre modo manual y automático
void handleToggleMode() {
  relay4_manual_control = !relay4_manual_control;  // Cambia el modo
  String mode = relay4_manual_control ? "Manual" : "Automático";
  Serial.println("Modo cambiado a: " + mode); // Para depuración
  server.send(204, "text/plain", ""); // Respuesta vacía
}

void setup() {
  // Configurar pines de relés
  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);
  pinMode(relay3_pin, OUTPUT);
  pinMode(relay4_pin, OUTPUT);

  // Configurar pin de señal como entrada
  pinMode(signal_pin, INPUT);

  // Apagar todos los relés al inicio
  digitalWrite(relay1_pin, HIGH);
  digitalWrite(relay2_pin, HIGH);
  digitalWrite(relay3_pin, HIGH);
  digitalWrite(relay4_pin, HIGH);

  // Conectar a WiFi
  Serial.begin(115200);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado. Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Configurar manejadores de rutas
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.on("/toggleMode", handleToggleMode);

  // Configurar OTA
  setupOTA();

  // Iniciar servidor
  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  // Manejar las solicitudes del servidor web
  server.handleClient();
  ArduinoOTA.handle(); // Manejar actualizaciones OTA

  // Verificar el estado del pin 26 si no se controla manualmente
  if (!relay4_manual_control) {
    if (digitalRead(signal_pin) == HIGH) {
      // Enciende el relay 4 si el pin 26 recibe señal alta
      if (!relay4_state) {
        relay4_state = true;
        digitalWrite(relay4_pin, LOW);  // Enciende el relay (baja el pin)
        Serial.print("Low");
      }
    } else {
      // Apaga el relay 4 si el pin 26 no recibe señal
      if (relay4_state) {
        relay4_state = false;
        digitalWrite(relay4_pin, HIGH);  // Apaga el relay (sube el pin)
        Serial.print("High");
      }
    }
  }
}
