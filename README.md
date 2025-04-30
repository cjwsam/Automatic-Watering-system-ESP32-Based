Smart Watering System
Author: cjwsamDate: April 30, 2025License: MIT
Overview
The Smart Watering System is an IoT-based solution for automated garden irrigation, built using ESP32 microcontrollers. It consists of two main components:

ESP32C3_SoilMonitor: A soil moisture monitoring system that uses an ESP32-C3 to measure soil humidity and publish data via MQTT. It provides a web interface for real-time monitoring.
SmartWatering: A comprehensive watering control system that uses an ESP32 with a TFT display to monitor water levels, control relays for watering zones, and manage schedules. It integrates with the soil monitor via MQTT and offers a web interface for configuration and diagnostics.

Features

Soil Moisture Monitoring: Measures soil humidity using analog sensors and displays data on a web page.
Water Level Monitoring: Uses an ultrasonic sensor to measure water tank levels with calibration support.
Relay Control: Manages watering zones via relays, controllable manually or via schedules.
TFT Display: Shows real-time water level, soil moisture, system status, and logs on a TFT screen (e.g., TTGO T-Display).
Web Interface: Provides dashboards for monitoring, calibration, scheduling, diagnostics, and log viewing.
MQTT Integration: Enables communication between devices and remote control/data publishing.
OTA Updates: Supports over-the-air firmware updates for both systems.
Captive Portal: Facilitates WiFi setup for the SmartWatering system.
Logging: Maintains a system log viewable on the TFT display and web interface.

Hardware Requirements
ESP32C3_SoilMonitor

ESP32-C3 board (e.g., ESP32-C3-DevKitM-1)
Analog soil moisture sensor
LED (onboard LED on GPIO 2 used by default)
WiFi network access
MQTT broker (e.g., Mosquitto running on a local server)

SmartWatering

ESP32 board with TFT display (e.g., TTGO T-Display)
Ultrasonic sensor (HC-SR04)
Relays for controlling watering zones (2 zones supported)
LED for testing (GPIO 2 by default)
Push buttons for navigation (GPIO 0 and 35 on TTGO)
WiFi network access
MQTT broker (shared with ESP32C3_SoilMonitor)

Software Dependencies

Arduino IDE or compatible platform (e.g., PlatformIO)
Libraries:
ESPAsyncWebServer (for web server functionality)
AsyncWebSocket (for real-time web updates in ESP32C3_SoilMonitor)
PubSubClient (for MQTT communication)
ArduinoJson (for JSON handling)
ArduinoOTA (for over-the-air updates)
WiFi (for network connectivity)
TFT_eSPI (for TFT display in SmartWatering)
EEPROM (for storing WiFi credentials and calibration data)
SPIFFS (for file system in SmartWatering)
DNSServer (for captive portal in SmartWatering)



Install these libraries via the Arduino Library Manager or PlatformIO.
Installation

Clone the Repository:
git clone https://github.com/cjwsam/smart-watering-system.git


Configure Settings:

For ESP32C3_SoilMonitor.ino:
Update ssid, password, mqttBroker, mqttUser, and mqttPass with your WiFi and MQTT credentials.


For SmartWatering.ino:
Update mqttBroker, mqttUser, and mqttPass in the main sketch.
Adjust otaPassword in config.h for OTA security.
Modify pin definitions in config.h if your hardware differs.




Set Up Hardware:

Connect the soil moisture sensor to GPIO 4 (ESP32C3_SoilMonitor).
Connect the ultrasonic sensor to GPIO 27 (trigger) and GPIO 15 (echo) (SmartWatering).
Connect relays to GPIO 25 (Zone 3) and GPIO 26 (Zone 4) (SmartWatering).
Ensure the TFT display and buttons are connected as per config.h (SmartWatering).


Upload the Sketches:

Open ESP32C3_SoilMonitor.ino and SmartWatering.ino in the Arduino IDE.
Select the appropriate board (ESP32-C3 for SoilMonitor, ESP32 Dev Module for SmartWatering).
Upload the sketches to their respective boards.


WiFi Setup (SmartWatering):

If WiFi credentials are not stored, the SmartWatering system will start in AP mode (SmartWateringAP, password: watering123).
Connect to the AP and access the captive portal to enter WiFi credentials.



Usage
ESP32C3_SoilMonitor

Web Interface: Access the device's IP address in a browser to view soil moisture levels, updated every 10 seconds via WebSocket.
MQTT: Publishes soil humidity to sensors/soilmonitor and subscribes to control/soilmonitor for commands.
LED Indicator: The onboard LED (GPIO 2) turns on if soil moisture exceeds 30%.
OTA Updates: Update firmware using the ArduinoOTA library with the hostname ESP32C3_SoilMonitor_0.

SmartWatering

TFT Display: Cycles through four pages (Water Level, Soil & Relays, System Status, Logs) using the next button (GPIO 35).
Web Interface:
Dashboard: View water level, relay status, and system status.
Calibration: Calibrate the water level sensor by setting two points (e.g., 50% and 100%).
Schedules: Add watering schedules for Zone 3 and Zone 4.
Diagnostics: Monitor heap memory, uptime, and network strength; reboot or toggle the test LED.
Logs: View and clear system logs.


MQTT: Publishes water level and relay states to sensors/environment, subscribes to sensors/soil/zone3, sensors/soil/zone4, and control/relay/zone3.
Relays: Controlled via the web interface, MQTT, or schedules. Hysteresis prevents rapid switching.
OTA Updates: Update firmware using the hostname SmartWatering and the configured OTA password.

File Structure
ESP32C3_SoilMonitor

ESP32C3_SoilMonitor.ino: Main sketch for soil moisture monitoring, web server, MQTT, and OTA.
Reads soil moisture from an analog sensor (GPIO 4).
Serves a web page with real-time updates via WebSocket.
Publishes data to MQTT and supports OTA updates.



SmartWatering

SmartWatering.ino: Main sketch for water level monitoring, relay control, TFT display, web server, MQTT, and OTA.
Initializes hardware, tasks, and services (WiFi, MQTT, web server, SPIFFS).
Manages captive portal for WiFi setup.


config.h: Defines pin assignments, MQTT credentials, EEPROM settings, and system constants.
utils.h: Utility functions for drawing gradients on the TFT and managing logs with a semaphore.
sensors.h: Handles ultrasonic sensor readings and water level calibration.
mqtt.h: Manages MQTT connections, subscriptions, and publishing.
display.h: Renders TFT display pages (water level, soil data, status, logs).
webserver.h: Defines web server routes and handlers for the dashboard, calibration, diagnostics, logs, and schedules.
index.h: HTML and JavaScript for the main dashboard and calibration interface.
diagnostics.h: HTML and JavaScript for the diagnostics page.
schedule.h: HTML and JavaScript for the scheduling page.
log.h: HTML and JavaScript for the log viewer.

Notes

Ensure the MQTT broker is running and accessible to both devices.
The SmartWatering system requires a TFT display compatible with the TFT_eSPI library (e.g., TTGO T-Display).
Calibrate the ultrasonic sensor for accurate water level readings using the web interface.
The scheduling feature does not include automatic relay activation based on time; it stores schedules for manual or external triggering.
Adjust the RELAY_HYSTERESIS_ON and RELAY_HYSTERESIS_OFF values in config.h to fine-tune relay behavior.

Contributing
Contributions are welcome! Please submit pull requests or open issues on the GitHub repository. Ensure code changes are well-documented and tested.
License
This project is licensed under the MIT License. See the LICENSE file for details.
Contact
For questions or support, contact the author via GitHub: cjwsam.
