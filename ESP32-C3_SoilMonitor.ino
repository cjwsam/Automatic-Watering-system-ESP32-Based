/*
 * Soil Moisture Monitor using ESP32-C3
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * This sketch implements a soil moisture monitoring system using an ESP32-C3.
 * Features:
 * - Web interface with real-time moisture updates via WebSocket
 * - MQTT integration for remote monitoring
 * - OTA updates
 * - Onboard LED indicator
 *
 * Hardware requirements:
 * - ESP32-C3 board
 * - Soil moisture sensor (analog output)
 * - LED (onboard LED on GPIO 2 used by default)
 *
 * Dependencies:
 * - ESPAsyncWebServer
 * - AsyncWebSocket
 * - PubSubClient
 * - ArduinoJson
 * - ArduinoOTA
 * - WiFi
 *
 * Instructions:
 * 1. Install the required libraries via Arduino Library Manager
 * 2. Update WiFi and MQTT configurations below
 * 3. Connect the soil moisture sensor to GPIO 4
 * 4. Upload the sketch and access the web interface at the device's IP
 */

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

// Pin definitions
#define SOIL_SENSOR_PIN 4  // GPIO 4 (ADC1_CHANNEL_3)
#define LED_PIN 2          // Onboard LED (GPIO 2)

// Network and MQTT configuration
const char* ssid = "YOUR_WIFI_SSID";          // Replace with your WiFi SSID
const char* password = "YOUR_WIFI_PASSWORD";   // Replace with your WiFi password
const char* mqttBroker = "YOUR_MQTT_BROKER";  // Replace with your MQTT broker IP
const int mqttPort = 1883;
const char* mqttUser = "YOUR_MQTT_USER";      // Replace with your MQTT username
const char* mqttPass = "YOUR_MQTT_PASSWORD";  // Replace with your MQTT password
const char* deviceName = "ESP32C3_SoilMonitor_0";

// HTML for the webpage
const char* index_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Soil Moisture Monitor</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <style>
        body {
            background: linear-gradient(to bottom right, #15803d, #86efac);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            font-family: 'Inter', sans-serif;
        }
        .card {
            background: white;
            border-radius: 1rem;
            box-shadow: 0 4px 20px rgba(0, 0, 0, 0.1);
            padding: 2rem;
            width: 100%;
            max-width: 400px;
            text-align: center;
        }
        .moisture-value {
            font-size: 2.5rem;
            font-weight: bold;
            color: #15803d;
        }
    </style>
</head>
<body>
    <div class="card">
        <h1 class="text-2xl font-bold text-gray-800 mb-4">Soil Moisture Monitor</h1>
        <div class="p-4 bg-gray-100 rounded-lg">
            <span class="text-gray-700 font-medium">Moisture Level</span>
            <div id="soilMoisture" class="moisture-value">-- %</div>
        </div>
        <p class="text-gray-500 text-sm mt-4">Updated every 10 seconds</p>
    </div>
    <script>
        const ws = new WebSocket(`ws://${window.location.hostname}/ws`);
        ws.onmessage = function(event) {
            const data = JSON.parse(event.data);
            document.getElementById('soilMoisture').textContent = data.soilHumidity.toFixed(1) + ' %';
        };
        ws.onclose = function() {
            setTimeout(() => location.reload(), 5000);
        };
    </script>
</body>
</html>
)rawliteral";

// Global objects
AsyncWebServer webServer(80);
AsyncWebSocket ws("/ws");
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Sensor data
float soilHumidity = 0.0;

// Status
bool mqttConnected = false;
bool wifiConnected = false;

// Timing
unsigned long lastPublish = 0;
const unsigned long publishInterval = 10000;  // 10 seconds

// Logging
String logBuffer = "";

void addLog(String message) {
  Serial.println(message);
  logBuffer += message + "\n";
  if (logBuffer.length() > 1000) logBuffer = logBuffer.substring(logBuffer.length() - 1000);
}

float readSoilHumidity() {
  const int numReadings = 5;
  long total = 0;
  for (int i = 0; i < numReadings; i++) {
    total += analogRead(SOIL_SENSOR_PIN);
    delay(50);
  }
  int avgRawValue = total / numReadings;

  // Map ADC (0-4095) to percentage (0-100%)
  const int dryValue = 4095;  // Dry (in air)
  const int wetValue = 1000;  // Wet (in water)
  float humidity = map(avgRawValue, dryValue, wetValue, 0, 100);
  return constrain(humidity, 0, 100);
}

void notifyClients() {
  StaticJsonDocument<100> doc;
  doc["soilHumidity"] = round(soilHumidity * 10) / 10.0;
  char json[100];
  serializeJson(doc, json);
  ws.textAll(json);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    // Handle incoming WebSocket messages if needed
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      addLog("WebSocket client #" + String(client->id()) + " connected");
      notifyClients();
      break;
    case WS_EVT_DISCONNECT:
      addLog("WebSocket client #" + String(client->id()) + " disconnected");
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  addLog("Connecting to WiFi: " + String(ssid));

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    addLog("WiFi connected, IP: " + WiFi.localIP().toString());
  } else {
    addLog("WiFi connection failed. Retrying in loop...");
  }
}

void setupWebServer() {
  ws.onEvent(onEvent);
  webServer.addHandler(&ws);
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });
  webServer.begin();
  addLog("Web server started");
}

void setupMQTT() {
  mqttClient.setServer(mqttBroker, mqttPort);
  mqttClient.setCallback([](char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
    addLog("MQTT received on " + String(topic) + ": " + message);
  });
}

void setupOTA() {
  ArduinoOTA.setHostname(deviceName);
  ArduinoOTA.onStart([]() { addLog("OTA Start"); });
  ArduinoOTA.onEnd([]() { addLog("OTA End"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.begin();
  addLog("OTA setup complete");
}

void reconnectMQTT() {
  if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
    addLog("Connecting to MQTT...");
    if (mqttClient.connect("ESP32C3Client", mqttUser, mqttPass)) {
      mqttClient.subscribe("control/soilmonitor");
      mqttConnected = true;
      addLog("MQTT connected");
      notifyClients();
    } else {
      addLog("MQTT connect failed, rc=" + String(mqttClient.state()));
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // Setup indicator
  pinMode(SOIL_SENSOR_PIN, INPUT);
  analogReadResolution(12);  // 12-bit ADC

  setupWiFi();
  setupWebServer();
  setupMQTT();
  setupOTA();

  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // Handle OTA
  ArduinoOTA.handle();

  // Maintain WiFi
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  if (!wifiConnected) {
    WiFi.reconnect();
    addLog("WiFi reconnecting");
    delay(2000);
    return;
  }

  // Maintain MQTT
  if (!mqttClient.connected()) {
    reconnectMQTT();
  } else {
    mqttClient.loop();
  }

  // Clean up WebSocket clients
  ws.cleanupClients();

  // Read soil moisture
  soilHumidity = readSoilHumidity();
  digitalWrite(LED_PIN, soilHumidity > 30 ? HIGH : LOW);  // LED ON if > 30%

  // Publish to MQTT and update webpage every 10 seconds
  if (millis() - lastPublish > publishInterval) {
    if (mqttConnected) {
      StaticJsonDocument<100> doc;
      doc["soilHumidity"] = round(soilHumidity * 10) / 10.0;
      char json[100];
      serializeJson(doc, json);
      bool published = mqttClient.publish("sensors/soilmonitor", json);
      addLog("Published to sensors/soilmonitor: " + String(json) + ", Success: " + String(published));
    }
    notifyClients();
    lastPublish = millis();
  }

  delay(1000);  // Update every second
}
