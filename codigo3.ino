#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h> // Librería para UDP

// Configuración de Wi-Fi
const char* ssid = "Google Wifi";
const char* password = ":B#hWe6u$8:J/e}<mEBhigwGU";

// URL del servidor Node.js (cambia la IP a la IP de tu PC)
const char* serverUrl = "http://192.168.1.127:3000/data";

int delayCounter = 0;

// Pines del sensor HC-SR04
#define TRIG_PIN 13
#define ECHO_PIN 12

// Pines de los LEDs
#define LED_VERDE 25
#define LED_AMARILLO 26
#define LED_NARANJA 27
#define LED_ROJO 14

#define PIN_ENTRADA  33

// Define los pines GPIO del ESP32 a los que conectaste los relés
int relay1 = 5;
int relay2 = 18;
int relay3 = 19;

// Umbrales de distancia (en cm) para cada nivel del tanque
const int UMBRAL_LLENO = 10.66;
const int UMBRAL_MEDIO_ALTO = 17.32;
const int UMBRAL_MEDIO_BAJO = 24;
const int UMBRAL_VACIO = 30.65;

// Configuración del sensor DHT11, LDR y humedad de suelo
#define DHT_PIN 4
DHT dht(DHT_PIN, DHT11);
const int ldrPin = 35;
const int humedadSueloPin = 34;

// Variables para filtrado de distancia
float lastDistance = 5.55;          // Última distancia válida medida
const float smoothingFactor = 0.1;  // Factor de suavizado (ajustable)
const int warmupTime = 5;        // Tiempo de calentamiento en segundos
unsigned long startMillis;       // Almacena el tiempo de inicio del programa

// Rango de valores válidos de distancia
const float MIN_DIST = 5.00;
const float MAX_DIST = 30.65;
const float MAX_JUMP = 2.5; // Máxima diferencia permitida entre lecturas consecutivas

// Configuración de UDP
WiFiUDP udp;
const char* udpAddress = "192.168.1.138";  // Dirección IP del otro ESP32
const int udpPort = 12345;                 // Puerto UDP para enviar el mensaje

// Función para medir la distancia actual
float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = (duration * 0.034) / 2;

  // Verificar si la distancia está dentro del rango
  if (distance >= MIN_DIST && distance <= MAX_DIST) {
    if (delayCounter == 5) {
      lastDistance = distance;
    } else if (delayCounter > 5) {
      if (abs(distance - lastDistance) <= MAX_JUMP) {
        lastDistance = lastDistance * (1 - smoothingFactor) + distance * smoothingFactor;
      }
    }
  }

  // Retorna la última distancia válida
  return lastDistance;
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

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Configuración de pines
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_NARANJA, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

  pinMode(PIN_ENTRADA, INPUT);

  // Inicializa los pines de los relés como salidas
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);

  // Asegúrate de que los relés estén apagados al inicio
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);

  // Conexión a Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado a Wi-Fi. IP: ");
  Serial.println(WiFi.localIP());

  // Configurar OTA
  setupOTA();
  startMillis = millis();
}

void loop() {
  ArduinoOTA.handle(); // Manejar actualizaciones OTA

  // Medir distancia con filtro de rango
  float distance = measureDistance();

  // Imprime valores para depuración
  Serial.print("Filtered Distance (lastDistance): ");
  Serial.println(lastDistance);

  // Control de LEDs según la distancia filtrada (lastDistance)
  if (lastDistance <= UMBRAL_LLENO) { 
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_NARANJA, LOW);
    digitalWrite(LED_ROJO, LOW);
  } else if (lastDistance <= UMBRAL_MEDIO_ALTO) {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARILLO, HIGH);
    digitalWrite(LED_NARANJA, LOW);
    digitalWrite(LED_ROJO, LOW);
  } else if (lastDistance <= UMBRAL_MEDIO_BAJO) {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_NARANJA, HIGH);
    digitalWrite(LED_ROJO, LOW);
  } else {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_NARANJA, LOW);
    digitalWrite(LED_ROJO, HIGH);
  }

  // Enviar mensaje al otro ESP32
  if (WiFi.status() == WL_CONNECTED) {
    String message = "Aguacate"; // El mensaje que se enviará
    udp.beginPacket(udpAddress, udpPort);
    udp.print(message);
    udp.endPacket();
    Serial.print("Mensaje enviado al ESP32: ");
    Serial.println(message);
  } else {
    Serial.println("Error: No conectado a Wi-Fi");
  }

  delayCounter++;
  Serial.println("El contador :");
  Serial.println(delayCounter);
  Serial.println("Hola");

  delay(1000); // Esperar 1 segundo antes de la siguiente lectura
}