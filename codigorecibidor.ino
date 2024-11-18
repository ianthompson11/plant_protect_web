#include <WiFi.h>
#include <WiFiUdp.h>

// Configuración de Wi-Fi
const char* ssid = "Google Wifi";
const char* password = ":B#hWe6u$8:J/e}<mEBhigwGU";

// Puerto de escucha
const int listenPort = 12345;

WiFiUDP udp;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Espera a que se conecte a Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado a Wi-Fi");

  // Comienza el UDP en el puerto de escucha
  udp.begin(listenPort);
  Serial.println("Esperando mensaje...");
}

void loop() {
  int packetSize = udp.parsePacket();  // Verifica si hay datos entrantes
  if (packetSize) {
    char incomingPacket[255];  // Buffer para los datos entrantes

    // Lee los datos
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0; // Añadir el terminador null
    }

    // Imprimir el mensaje recibido
    Serial.print("Mensaje recibido: ");
    Serial.println(incomingPacket);
  }

  delay(100); // Espera 100 ms antes de leer nuevamente
}