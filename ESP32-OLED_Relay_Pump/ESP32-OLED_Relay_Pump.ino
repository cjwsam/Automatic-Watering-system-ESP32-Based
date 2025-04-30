/*
 * Smart Watering System for ESP32
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * This sketch implements a smart watering system using an ESP32 with a TFT display.
 * Features:
 * - Monitors water level using an ultrasonic sensor
 * - Controls relays for watering zones based on soil moisture data
 * - Displays real-time data on a TFT screen (water level, soil moisture, system status)
 * - Web interface for monitoring, calibration, and scheduling
 * - MQTT integration for remote control and data publishing
 * - OTA updates and captive portal for WiFi setup
 * - Logging system with web-based log viewer
 *
 * Hardware requirements:
 * - ESP32 board with TFT display (e.g., TTGO T-Display)
 * - Ultrasonic sensor (HC-SR04)
 * - Soil moisture sensors (connected to separate ESP32-C3 via MQTT)
 * - Relays for controlling watering zones
 * - LED for testing
 *
 * Dependencies:
 * - TFT_eSPI
 * - ESPAsyncWebServer
 * - PubSubClient
 * - ArduinoJson
 * - ArduinoOTA
 * - WiFi
 * - EEPROM
 * - SPIFFS
 * - DNSServer
 *
 * Instructions:
 * 1. Install the required libraries via Arduino Library Manager
 * 2. Configure pins and settings in config.h
 * 3. Update MQTT broker details in the sketch
 * 4. Upload the sketch and access the web interface at the device's IP
 * 5. Use the captive portal for initial WiFi setup if needed
 */

#include <FS.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "utils.h"
#include "sensors.h"
#include "mqtt.h"
#include "display.h"
#include "webserver.h"

// Global variables
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
AsyncWebServer webServer(80);
DNSServer dnsServer;
bool inFallbackAP = false;
String storedSsid, storedPass;
float soilHumidityZone3 = 0.0, soilHumidityZone4 = 0.0;
bool soilDataReceived = false;
float waterLevel = 0.0;
bool waterSensorOk = false;
int sensorErrorCount = 0;
bool relayZone3State = false, relayZone4State = false;
unsigned long relayZone3LastChange = 0, relayZone4LastChange = 0;
bool wifiConnected = false;
bool mqttConnected = false;
bool timeSynced = false;
volatile bool displayOn = true;
volatile int currentPage = 0;
String logBuffer[LOG_BUFFER_SIZE];
int logHead = 0, logCount = 0;
SemaphoreHandle_t logSemaphore;
float wavePhase = 0.0;
float waterLevelHistory[SENSOR_SAMPLES];
int waterLevelIndex = 0;
bool waterLevelInitialized = false;
bool ledState = false;
CalibrationPoint calibPoint1 = {0.0, 0.0, false};
CalibrationPoint calibPoint2 = {0.0, 0.0, false};
float fullTankDistance = 10.0;
float emptyTankDistance = 100.0;
bool isCalibrated = false;
Schedule schedules[10];
int scheduleCount = 0;

// Define MQTT broker and port
const char* mqttBroker = "YOUR_MQTT_BROKER";
const int mqttPort = 1883;

// Task declarations
extern void displayTask(void *pvParameters);
extern void networkTask(void *pvParameters);
extern void sensorTask(void *pvParameters);

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUTTON_PIN_NEXT, INPUT_PULLUP);
    pinMode(relayZone3, OUTPUT);
    pinMode(relayZone4, OUTPUT);
    pinMode(ledPin, OUTPUT);
    digitalWrite(relayZone3, LOW);
    digitalWrite(relayZone4, LOW);
    digitalWrite(ledPin, LOW);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    // Initialize water level history
    for (int i = 0; i < SENSOR_SAMPLES; i++) {
        waterLevelHistory[i] = 0.0;
    }

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    tft.init();
    tft.setRotation(1);
    spr.createSprite(240, 135);
    spr.setTextWrap(true);
    logSemaphore = xSemaphoreCreateMutex();

    EEPROM.begin(EEPROM_SIZE);
    storedSsid = EEPROM.readString(SSID_ADDR);
    storedPass = EEPROM.readString(PASS_ADDR);
    // Load calibration points
    EEPROM.get(CALIB_POINT1_ADDR, calibPoint1);
    EEPROM.get(CALIB_POINT2_ADDR, calibPoint2);
    // Validate loaded values
    if (calibPoint1.valid && calibPoint2.valid && calibPoint1.percent != calibPoint2.percent) {
        computeCalibration();
    } else {
        isCalibrated = false;
    }
    addLog("Calibration: " + String(isCalibrated ? "Calibrated" : "Uncalibrated"));

    // Manual relay test to confirm hardware functionality
    addLog("Starting manual relay test for Zone 3");
    for (int i = 0; i < 3; i++) {
        digitalWrite(relayZone3, HIGH);
        addLog("Relay Z3 ON (manual test)");
        delay(1000);
        digitalWrite(relayZone3, LOW);
        addLog("Relay Z3 OFF (manual test)");
        delay(1000);
    }

    if (storedSsid.length() > 0 && storedPass.length() > 0) {
        WiFi.begin(storedSsid.c_str(), storedPass.c_str());
        unsigned long to = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - to < 10000) delay(500);
        wifiConnected = (WiFi.status() == WL_CONNECTED);
    }
    if (!wifiConnected) {
        WiFi.softAP(apSSID, apPassword);
        inFallbackAP = true;
        dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
        webServer.on("/", HTTP_GET, handleCaptivePortal);
        webServer.on("/save", HTTP_POST, handleSaveCredentials);
        webServer.begin();
        addLog("AP mode");
    } else {
        addLog("Connected to WiFi, IP: " + WiFi.localIP().toString());
        webServer.on("/", HTTP_GET, handleRoot);
        webServer.on("/logs", HTTP_GET, handleLogs);
        webServer.on("/getLogs", HTTP_GET, handleGetLogs);
        webServer.on("/clearLogs", HTTP_POST, handleClearLogs);
        webServer.on("/diagnostics", HTTP_GET, handleDiagnosticsPage);
        webServer.on("/diagnostics", HTTP_GET, handleDiagnostics);
        webServer.on("/reboot", HTTP_POST, handleReboot);
        webServer.on("/toggleLed", HTTP_POST, handleToggleLed);
        webServer.on("/schedule", HTTP_GET, handleSchedulePage);
        webServer.on("/getSchedules", HTTP_GET, handleGetSchedules);
        webServer.on("/addSchedule", HTTP_POST, handleAddSchedule);
        webServer.on("/save", HTTP_POST, handleSaveCredentials);
        webServer.on("/data", HTTP_GET, handleData);
        // Explicitly handle POST for /toggleRelay with a body callback
        webServer.on("/toggleRelay", HTTP_POST, [](AsyncWebServerRequest *request) {
            // Placeholder for the initial handler
        }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String body = "";
            for (size_t i = 0; i < len; i++) {
                body += (char)data[i];
            }
            handleToggleRelay(request, body);
        });
        webServer.on("/calibrate", HTTP_GET, handleCalibrate);
        webServer.on("/saveCalibration", HTTP_POST, handleSaveCalibration);
        webServer.on("/resetCalibration", HTTP_POST, handleResetCalibration);
        webServer.begin();
    }

    ArduinoOTA.setHostname(otaHostname);
    ArduinoOTA.setPassword(otaPassword);
    ArduinoOTA.begin();

    mqttClient.setServer(mqttBroker, mqttPort);
    mqttClient.setCallback(mqttCallback);

    xTaskCreate(displayTask, "DisplayTask", 4096, nullptr, 1, nullptr);
    xTaskCreate(networkTask, "NetworkTask", 4096, nullptr, 2, nullptr);
    xTaskCreate(sensorTask, "SensorTask", 2048, nullptr, 1, nullptr);

    // Relay test
    addLog("Testing relays");
    digitalWrite(relayZone3, HIGH);
    digitalWrite(relayZone4, HIGH);
    delay(3000);
    digitalWrite(relayZone3, LOW);
    digitalWrite(relayZone4, LOW);
    addLog("Relay test complete");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
