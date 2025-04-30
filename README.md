# Smart Watering System

**Author**: cjwsam  
**Date**: April 30, 2025  
**License**: MIT

## Overview

The Smart Watering System is an IoT-based solution for automated garden irrigation, built using ESP32 microcontrollers. It consists of two main components:

1. **ESP32C3_SoilMonitor**: A soil moisture monitoring system that uses an ESP32-C3 to measure soil humidity and publish data via MQTT. It provides a web interface for real-time monitoring.
2. **SmartWatering**: A comprehensive watering control system that uses an ESP32 with a TFT display to monitor water levels, control relays for watering zones, and manage schedules. It integrates with the soil monitor via MQTT and offers a web interface for configuration and diagnostics.

### Features
- **Soil Moisture Monitoring**: Measures soil humidity using analog sensors and displays data on a web page.
- **Water Level Monitoring**: Uses an ultrasonic sensor to measure water tank levels with calibration support.
- **Relay Control**: Manages watering zones via relays, controllable manually or via schedules.
- **TFT Display**: Shows real-time water level, soil moisture, system status, and logs on a TFT screen (e.g., TTGO T-Display).
- **Web Interface**: Provides dashboards for monitoring, calibration, scheduling, diagnostics, and log viewing.
- **MQTT Integration**: Enables communication between devices and remote control/data publishing.
- **OTA Updates**: Supports over-the-air firmware updates for both systems.
- **Captive Portal**: Facilitates WiFi setup for the SmartWatering system.
- **Logging**: Maintains a system log viewable on the TFT display and web interface.

## Hardware Requirements

### ESP32C3_SoilMonitor
- ESP32-C3 board (e.g., ESP32-C3-DevKitM-1)
- Analog soil moisture sensor
- LED (onboard LED on GPIO 2 used by default)
- WiFi network access
- MQTT broker (e.g., Mosquitto running on a local server)

### SmartWatering
- ESP32 board with TFT display (e.g., TTGO T-Display)
- Ultrasonic sensor (HC-SR04)
- Relays for controlling watering zones (2 zones supported)
- LED for testing (GPIO 2 by default)
- Push buttons for navigation (GPIO 0 and 35 on TTGO)
- WiFi network access
- MQTT broker (shared with ESP32C3_SoilMonitor)

## Software Dependencies

- **Arduino IDE** or compatible platform (e.g., PlatformIO)
- **Libraries**:
  - `ESPAsyncWebServer`
  - `AsyncWebSocket`
  - `PubSubClient`
  - `ArduinoJson`
  - `ArduinoOTA`
  - `WiFi`
  - `TFT_eSPI`
  - `EEPROM`
  - `SPIFFS`
  - `DNSServer`

Install these libraries via the Arduino Library Manager or PlatformIO.

## Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/cjwsam/smart-watering-system.git
   ```

2. **Configure Settings**:
   - For `ESP32C3_SoilMonitor.ino`:
     - Update `ssid`, `password`, `mqttBroker`, `mqttUser`, and `mqttPass`.
   - For `SmartWatering.ino`:
     - Update `mqttBroker`, `mqttUser`, `mqttPass`.
     - Adjust `otaPassword` in `config.h`.
     - Modify pin definitions in `config.h` if hardware differs.

3. **Set Up Hardware**:
   - Soil moisture sensor → GPIO 4 (ESP32C3_SoilMonitor)
   - Ultrasonic sensor → GPIO 27 (trigger), GPIO 15 (echo) (SmartWatering)
   - Relays → GPIO 25 (Zone 3), GPIO 26 (Zone 4)
   - TFT display/buttons → per `config.h`

4. **Upload the Sketches**:
   - Use Arduino IDE or PlatformIO.
   - Board: ESP32-C3 for SoilMonitor, ESP32 Dev Module for SmartWatering.
   - Upload `ESP32C3_SoilMonitor.ino` and `SmartWatering.ino`.

5. **WiFi Setup (SmartWatering)**:
   - Boots in AP mode (`SmartWateringAP`, password: `watering123`) if no WiFi.
   - Enter WiFi credentials via captive portal.

## Usage

### ESP32C3_SoilMonitor
- Web interface: Displays soil moisture (via WebSocket).
- MQTT:
  - Publishes: `sensors/soilmonitor`
  - Subscribes: `control/soilmonitor`
- LED on GPIO 2 lights up if soil is dry.
- OTA via hostname: `ESP32C3_SoilMonitor_0`

### SmartWatering
- TFT Display Pages:
  - Water Level
  - Soil & Relays
  - System Status
  - Logs (navigate with GPIO 35)
- Web Interface:
  - **Dashboard**: Current water level and relay states.
  - **Calibration**: Set 50% and 100% sensor points.
  - **Schedules**: Configure watering times per zone.
  - **Diagnostics**: Reboot, heap, RSSI, uptime.
  - **Logs**: View system logs.
- MQTT:
  - Publishes: `sensors/environment`
  - Subscribes: `sensors/soil/zone3`, `sensors/soil/zone4`, `control/relay/zone3`
- OTA via hostname: `SmartWatering`

## File Structure

### ESP32C3_SoilMonitor
- `ESP32C3_SoilMonitor.ino`: Main sketch
  - Soil reading (GPIO 4)
  - WebSocket + web page
  - MQTT publishing
  - OTA

### SmartWatering
- `SmartWatering.ino`: Main sketch
  - Setup tasks/services
  - WiFi portal
- `config.h`: Pins, MQTT, EEPROM, constants
- `utils.h`: TFT gradients, logging
- `sensors.h`: Ultrasonic calibration
- `mqtt.h`: Pub/Sub handlers
- `display.h`: All TFT display rendering
- `webserver.h`: Web routes
- `index.h`: HTML/JS for dashboard
- `diagnostics.h`: HTML/JS for diagnostics
- `schedule.h`: HTML/JS for schedules
- `log.h`: HTML/JS for log viewer

## Notes
- Ensure MQTT broker is running and reachable.
- TFT display must be compatible with `TFT_eSPI` (e.g., TTGO T-Display).
- Use web interface for sensor calibration.
- Scheduling currently stores time data but does not trigger relays — you can integrate logic to handle that.
- Adjust hysteresis values in `config.h`:
  ```cpp
  #define RELAY_HYSTERESIS_ON  30
  #define RELAY_HYSTERESIS_OFF 70
  ```

## License

This project is licensed under the MIT License.
