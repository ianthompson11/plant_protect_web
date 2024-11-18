#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

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
//int relay4 = 21;

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
const float MAX_JUMP = 30; // Máxima diferencia permitida entre lecturas consecutivas

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
  /*if (distance >= MIN_DIST && distance <= MAX_DIST) {
    // Validar si el salto en la distancia es mayor que MAX_JUMP
    if (abs(distance - lastDistance) <= MAX_JUMP) {
      // Aplicar el suavizado solo si el cambio está dentro del límite permitido
      lastDistance = lastDistance * (1 - smoothingFactor) + distance * smoothingFactor;
    } else {
      // Si el salto es demasiado grande, no se actualiza la distancia
      Serial.println("Salto grande detectado, ignorando lectura.");
    }
  }*/

  Serial.print("La distancia es: ");
  Serial.println(distance);

    // Verificar si la distancia está dentro del rango
  if (distance >= MIN_DIST && distance <= MAX_DIST) {

    if(delayCounter ==  5 ){
      Serial.println("entro");
      lastDistance = distance ;

    }else if(delayCounter >  5 ){

      if (abs(distance - lastDistance) <= MAX_JUMP) {
        // Aplicar el suavizado solo si el cambio está dentro del límite permitido
        lastDistance = lastDistance * (1 - smoothingFactor) + distance * smoothingFactor;
      } else {
        // Si el salto es demasiado grande, no se actualiza la distancia
        Serial.println("Salto grande detectado, ignorando lectura.");
      }

    }



  }
  
  // Retorna la última distancia válida (ya sea la nueva distancia válida o la última que pasó el filtro)
  return lastDistance;
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
  //pinMode(relay4, OUTPUT);

  // Asegúrate de que los relés estén apagados al inicio
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  //digitalWrite(relay4, LOW);

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

  // Inicialización del tiempo de calentamiento
  startMillis = millis();
}

void loop() {
  // Verifica si aún está en tiempo de calentamiento
  /*if (millis() - startMillis < warmupTime * 1000) {
    Serial.println("Calentando el sensor...");
    delay(1000); // Espera un segundo durante el tiempo de calentamiento
    return; // Salir de loop() hasta que termine el tiempo de calentamiento
  }*/


  int estadoPin = digitalRead(PIN_ENTRADA); // Lee el estado del pin

  if (estadoPin == HIGH) {
    // Acción cuando el pin recibe una señal (por ejemplo, una señal HIGH)
    Serial.println("¡Señal recibida!");  // Imprime un mensaje en el terminal
    //digitalWrite(relay4, HIGH);
    // Aquí puedes añadir cualquier acción que desees realizar
    // por ejemplo, encender un LED, activar un motor, etc.
    
  }else if(estadoPin == LOW){
    
    //digitalWrite(relay4, LOW);


  }

  // Medir distancia con filtro de rango
  float distance = measureDistance(); 

  // Imprime valores para depuración
  Serial.print("Filtered Distance (lastDistance): ");
  Serial.println(lastDistance);

  // Control de LEDs según la distancia filtrada (lastDistance)
  if (lastDistance <= UMBRAL_LLENO) { 
    //De 5 cm a 10.66 cm
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_NARANJA, LOW);
    digitalWrite(LED_ROJO, LOW);
  } else if (lastDistance <= UMBRAL_MEDIO_ALTO) {
    //entre 17.32 y 10.66
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARILLO, HIGH);
    digitalWrite(LED_NARANJA, LOW);
    digitalWrite(LED_ROJO, LOW);
  } else if (lastDistance <= UMBRAL_MEDIO_BAJO) {
    //entre 24 y 17.32
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_NARANJA, HIGH);
    digitalWrite(LED_ROJO, LOW);
  } else {
    //entre 30.65 y 24
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARILLO, LOW);
    digitalWrite(LED_NARANJA, LOW);
    digitalWrite(LED_ROJO, HIGH);
  }

  // Lectura de sensores adicionales
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();
  int ldrValor = analogRead(ldrPin);
  float luminosidad = map(ldrValor, 0, 4095, 100, 0);
  int humedadSueloValor = analogRead(humedadSueloPin);
  float humedadSuelo = map(humedadSueloValor, 2800, 4095, 100, 0);
  humedadSuelo = constrain(humedadSuelo, 0, 100);

  // Validación de datos para evitar NaN y establecer 8888 en caso de error
  if (isnan(temperatura)) temperatura = 888;
  if (isnan(humedad)) humedad = 888;
  if (isnan(luminosidad)) luminosidad = 888;
  if (isnan(humedadSuelo)) humedadSuelo = 888;

  // Enviar datos al servidor Node.js
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    // Crear el JSON con los datos del sensor
  String postData = "{\"distance\": " + String(lastDistance) +
                    ", \"temperatura\": " + String(temperatura) +
                    ", \"humedad\": " + String(humedad) +
                    ", \"luminosidad\": " + String(luminosidad) +
                    ", \"humedadSuelo\": " + String(humedadSuelo) + 
                    ", \"contador\": " + String(delayCounter) + "}";


    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("Respuesta del servidor: ");
      Serial.println(response);
    } else {
      Serial.print("Error en la solicitud POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Error: No conectado a Wi-Fi");
  }

  delayCounter++;
  Serial.println("El contador :");
  Serial.println(delayCounter);

  delay(1000); // Esperar 1 segundo antes de la siguiente lectura
}