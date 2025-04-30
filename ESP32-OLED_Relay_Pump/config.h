/*
 * Configuration Header for Smart Watering System
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * Defines constants, pin assignments, and data structures for the Smart Watering System.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <EEPROM.h>

// Configuration defines
const char* mqttUser = "YOUR_MQTT_USER";      // Replace with your MQTT username
const char* mqttPass = "YOUR_MQTT_PASSWORD";  // Replace with your MQTT password
#define SENSOR_SAMPLES 5
#define SENSOR_ERROR_THRESHOLD 3
#define RELAY_HYSTERESIS_ON 30
#define RELAY_HYSTERESIS_OFF 70
#define RELAY_MIN_DURATION_MS 30000 // 30 seconds
#define MIN_DISTANCE_CHANGE 1.0 // Minimum distance change (cm) for second calibration point

// Pin definitions
const int BUTTON_PIN = 0;       // TTGO button 1
const int BUTTON_PIN_NEXT = 35; // TTGO button 2
const int relayZone3 = 25;      // Zone 3 relay
const int relayZone4 = 26;      // Zone 4 relay
const int trigPin = 27;         // Ultrasonic trigger
const int echoPin = 15;         // Ultrasonic echo
const int ledPin = 2;           // Test LED pin (adjust as needed)

// OTA settings
const char* otaHostname = "SmartWatering";
const char* otaPassword = "YOUR_OTA_PASSWORD"; // Replace with your OTA password

// Captive portal
const byte DNS_PORT = 53;
const char* apSSID = "SmartWateringAP";
const char* apPassword = "watering123";

// EEPROM for credentials and calibration
#define EEPROM_SIZE (128 + 2 * sizeof(CalibrationPoint))
#define SSID_ADDR 0
#define SSID_MAX_LEN 32
#define PASS_ADDR (SSID_ADDR + SSID_MAX_LEN)
#define PASS_MAX_LEN 64
#define CALIB_POINT1_ADDR (PASS_ADDR + PASS_MAX_LEN)
#define CALIB_POINT2_ADDR (CALIB_POINT1_ADDR + sizeof(CalibrationPoint))

// Display state
#define WATER_PAGE 0
#define SOIL_PAGE 1
#define STATUS_PAGE 2
#define LOG_PAGE 3

// Log buffer
#define LOG_BUFFER_SIZE 10

// Schedule structure
struct Schedule {
    String zone;
    String startTime;
    int duration;
};

#endif
