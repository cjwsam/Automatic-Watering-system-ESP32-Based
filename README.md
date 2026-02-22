# Automatic Watering System - ESP32 Based

**Author**: cjwsam
**Date**: April 30, 2025
**License**: MIT

## Overview

The Automatic Watering System is an IoT-based smart irrigation solution built on ESP32 microcontrollers. It automates garden and container watering by monitoring soil moisture levels, tracking water tank levels, and controlling solenoid valves or pumps through relay modules. The system communicates over MQTT and provides web-based dashboards for monitoring, manual control, calibration, and scheduling.

The project consists of two cooperating devices:

1. **ESP32-C3 Soil Monitor** (`ESP32-C3_SoilMonitor.ino`) -- A remote soil moisture sensor node that reads analog soil moisture levels and publishes them over MQTT. It also serves a lightweight web page showing real-time moisture via WebSocket.

2. **ESP32 Smart Watering Controller** (`ESP32-OLED_Relay_Pump/`) -- The main controller that receives soil data over MQTT, measures water tank level with an ultrasonic sensor, drives relay-controlled watering zones, and provides a full web interface with dashboards, diagnostics, scheduling, calibration, and log viewing. It uses a TFT display (e.g., TTGO T-Display) for local status display.

## Features

- Real-time soil moisture monitoring with configurable sensor averaging
- Water tank level measurement using HC-SR04 ultrasonic sensor with two-point calibration
- Two independently controllable watering zones (Zone 3 and Zone 4) via relay modules
- MQTT integration for inter-device communication and remote control of both zones
- Web-based dashboard with relay toggle, water level display, and system status
- Web-based water level calibration (two-point linear interpolation)
- Watering schedule configuration via web interface (zone, start time, duration)
- TFT display with four pages: Water Level, Soil & Relays, System Status, Logs
- Animated wave visualization on the water level TFT page
- System diagnostics page (heap memory, uptime, WiFi RSSI)
- Log viewer accessible via TFT display and web interface
- OTA (Over-The-Air) firmware updates for both devices
- Captive portal for initial WiFi configuration (AP mode fallback)
- EEPROM persistence for WiFi credentials and calibration data

## Hardware Requirements

### ESP32-C3 Soil Monitor

| Component | Description |
|---|---|
| ESP32-C3 board | e.g., ESP32-C3-DevKitM-1 |
| Soil moisture sensor | Capacitive or resistive, analog output |
| LED | Onboard LED on GPIO 2 (lights when soil is dry) |

### ESP32 Smart Watering Controller

| Component | Description |
|---|---|
| ESP32 with TFT display | e.g., TTGO T-Display (ST7789, 240x135) |
| Ultrasonic sensor | HC-SR04 for water tank level measurement |
| 2-channel relay module | 5V relay module for controlling solenoid valves or pumps |
| Solenoid valves or pumps | One per watering zone (2 zones supported) |
| Push buttons | Two buttons for TFT page navigation (GPIO 0 and GPIO 35 on TTGO) |
| Test LED | GPIO 2 (optional, for diagnostics) |

### Shared Infrastructure

| Component | Description |
|---|---|
| MQTT broker | e.g., Mosquitto running on a Raspberry Pi or local server |
| WiFi network | 2.4 GHz WiFi accessible by both devices |
| Power supply | 5V for ESP32 boards; appropriate supply for solenoid valves |

## Pin Assignments

### ESP32-C3 Soil Monitor

| Pin | Function |
|---|---|
| GPIO 4 | Soil moisture sensor (ADC1_CHANNEL_3) |
| GPIO 2 | Onboard LED indicator |

### ESP32 Smart Watering Controller

| Pin | Function |
|---|---|
| GPIO 25 | Relay -- Zone 3 |
| GPIO 26 | Relay -- Zone 4 |
| GPIO 27 | Ultrasonic sensor trigger (HC-SR04) |
| GPIO 15 | Ultrasonic sensor echo (HC-SR04) |
| GPIO 2 | Test LED |
| GPIO 0 | Button 1 (TTGO built-in) |
| GPIO 35 | Button 2 / Page navigation (TTGO built-in) |

## MQTT Topic Structure

### Published by ESP32-C3 Soil Monitor

| Topic | Payload | Description |
|---|---|---|
| `sensors/soilmonitor` | `{"soilHumidity": 45.2}` | Soil moisture percentage (published every 10s) |

### Subscribed by ESP32-C3 Soil Monitor

| Topic | Description |
|---|---|
| `control/soilmonitor` | Reserved for remote commands to the soil sensor |

### Published by Smart Watering Controller

| Topic | Payload | Description |
|---|---|---|
| `sensors/environment` | `{"waterLevel": 72.5, "relayZ3": "ON", "relayZ4": "OFF", "wifiConnected": true, "soilDataReceived": true}` | System status (published every 10s) |

### Subscribed by Smart Watering Controller

| Topic | Payload | Description |
|---|---|---|
| `sensors/soil/zone3` | `{"soilHumidity": 45.2}` | Soil moisture data for Zone 3 |
| `sensors/soil/zone4` | `{"soilHumidity": 38.7}` | Soil moisture data for Zone 4 |
| `control/relay/zone3` | `{"relayZ3": "ON"}` or `{"relayZ3": "OFF"}` | Remote on/off control for Zone 3 relay |
| `control/relay/zone4` | `{"relayZ4": "ON"}` or `{"relayZ4": "OFF"}` | Remote on/off control for Zone 4 relay |

## Web Interface Endpoints

| Path | Method | Description |
|---|---|---|
| `/` | GET | Main dashboard (water level, relay status, system status) |
| `/calibrate` | GET | Water level sensor calibration page |
| `/schedule` | GET | Watering schedule management page |
| `/diagnostics` | GET | System diagnostics (heap, uptime, RSSI) |
| `/logs` | GET | System log viewer |
| `/data` | GET | JSON API for current sensor and relay data |
| `/getLogs` | GET | JSON API for log entries |
| `/clearLogs` | POST | Clear all log entries |
| `/toggleRelay` | POST | Toggle relay state (JSON body with `relayZ3` and/or `relayZ4`) |
| `/saveCalibration` | POST | Save a calibration data point |
| `/resetCalibration` | POST | Reset calibration to defaults |
| `/addSchedule` | POST | Add a new watering schedule entry |
| `/getSchedules` | GET | JSON API for current schedule entries |
| `/reboot` | POST | Reboot the ESP32 |
| `/toggleLed` | POST | Toggle the test LED |
| `/save` | POST | Save WiFi credentials (captive portal) |

## Configuration

### Step 1: Install Dependencies

Install the following libraries via the Arduino Library Manager or PlatformIO:

- `ESPAsyncWebServer`
- `AsyncWebSocket` (usually bundled with ESPAsyncWebServer)
- `PubSubClient`
- `ArduinoJson`
- `ArduinoOTA` (built-in with ESP32 Arduino core)
- `TFT_eSPI`
- `WiFi` (built-in with ESP32 Arduino core)
- `EEPROM` (built-in with ESP32 Arduino core)
- `SPIFFS` (built-in with ESP32 Arduino core)
- `DNSServer` (built-in with ESP32 Arduino core)

For `TFT_eSPI`, configure the library for your display. For the TTGO T-Display, select `Setup25_TTGO_T_Display.h` in the TFT_eSPI `User_Setup_Select.h` file.

### Step 2: Update Credentials

**ESP32-C3 Soil Monitor** (`ESP32-C3_SoilMonitor.ino`):
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqttBroker = "YOUR_MQTT_BROKER";  // e.g., "192.168.1.100"
const char* mqttUser = "YOUR_MQTT_USER";
const char* mqttPass = "YOUR_MQTT_PASSWORD";
```

**Smart Watering Controller** (`ESP32-OLED_Relay_Pump/ESP32-OLED_Relay_Pump.ino` and `config.h`):
```cpp
// In ESP32-OLED_Relay_Pump.ino:
const char* mqttBroker = "YOUR_MQTT_BROKER";

// In config.h:
const char* mqttUser = "YOUR_MQTT_USER";
const char* mqttPass = "YOUR_MQTT_PASSWORD";
const char* otaPassword = "YOUR_OTA_PASSWORD";
```

Alternatively, the Smart Watering Controller supports WiFi setup via captive portal. On first boot (or if stored credentials fail), it creates an access point:
- **SSID**: `SmartWateringAP`
- **Password**: `watering123`

Connect to this AP and enter your WiFi credentials through the portal page.

### Step 3: Upload Firmware

1. **ESP32-C3 Soil Monitor**: Select board "ESP32-C3 Dev Module" in Arduino IDE and upload `ESP32-C3_SoilMonitor.ino`.
2. **Smart Watering Controller**: Select board "ESP32 Dev Module" in Arduino IDE and upload the sketch from the `ESP32-OLED_Relay_Pump/` folder.

After initial upload, subsequent updates can be done over-the-air:
- Soil Monitor OTA hostname: `ESP32C3_SoilMonitor_0`
- Controller OTA hostname: `SmartWatering`

## Setting Up Watering Schedules

1. Open the web interface at `http://<controller-ip>/schedule`.
2. Select the target zone (Zone 3 or Zone 4).
3. Enter a start time (HH:MM format).
4. Enter the watering duration in minutes (1--60).
5. Click "Add Schedule".

**Note**: The current implementation stores schedule entries in memory and displays them on the web interface. Automatic schedule execution (time-based relay activation) is not yet implemented. See the TODO comments in `webserver.h` for guidance on adding a FreeRTOS task that checks current time via NTP and triggers relays according to the schedule.

## Water Level Calibration

The ultrasonic water level sensor requires calibration for accurate percentage readings:

1. Navigate to `http://<controller-ip>/calibrate`.
2. Set the water to a known level (e.g., half full = 50%).
3. Enter the percentage and click "Save Point".
4. Change the water level significantly (e.g., full = 100%) and wait 10 seconds.
5. Enter the new percentage and click "Save Point" again.
6. Calibration is computed using linear interpolation between the two points and is persisted to EEPROM.

Use "Reset Calibration" to start over. Without calibration, the system uses a default range of 10--100 cm.

## File Structure

```
Automatic-Watering-system-ESP32-Based/
|-- ESP32-C3_SoilMonitor.ino     # Soil moisture sensor node (standalone sketch)
|-- ESP32-OLED_Relay_Pump/       # Smart Watering Controller
|   |-- ESP32-OLED_Relay_Pump.ino  # Main sketch (setup, tasks, WiFi, OTA)
|   |-- config.h                   # Pin definitions, constants, data structures
|   |-- utils.h                    # Utility functions (logging, TFT gradient)
|   |-- sensors.h                  # Ultrasonic sensor reading and calibration
|   |-- mqtt.h                     # MQTT callback, subscriptions, publishing task
|   |-- display.h                  # TFT display rendering (4 pages)
|   |-- webserver.h                # Web server route handlers
|   |-- index.h                    # HTML/JS for main dashboard and calibration
|   |-- log.h                      # HTML/JS for log viewer page
|   |-- diagnostics.h             # HTML/JS for diagnostics page
|   |-- schedule.h                 # HTML/JS for schedule management page
|-- README.md
```

## Architecture

```
+---------------------+        MQTT         +------------------------+
| ESP32-C3            | ------------------> | ESP32 Smart Watering   |
| Soil Monitor        |  sensors/soil/zone3 | Controller             |
|                     |  sensors/soil/zone4 |                        |
| - Reads soil sensor |                     | - Reads water level    |
| - Serves web page   | <------------------ | - Controls 2 relays    |
| - Publishes MQTT    |  control/relay/zone3| - TFT display (4 pages)|
|                     |  control/relay/zone4| - Web dashboard        |
+---------------------+                     | - Calibration          |
                                            | - Scheduling           |
                                            | - Diagnostics & logs   |
                                            +------------------------+
                                                      |
                                                      | GPIO 25, 26
                                                      v
                                            +------------------------+
                                            | Relay Module           |
                                            | Zone 3 | Zone 4       |
                                            +--------+---------------+
                                                      |
                                                      v
                                            Solenoid Valves / Pumps
```

## Tuning Parameters

The following constants in `config.h` can be adjusted:

| Constant | Default | Description |
|---|---|---|
| `SENSOR_SAMPLES` | 5 | Number of readings averaged for water level |
| `SENSOR_ERROR_THRESHOLD` | 3 | Consecutive sensor errors before marking sensor as failed |
| `RELAY_HYSTERESIS_ON` | 30 | Soil moisture % below which relay turns ON (dry) |
| `RELAY_HYSTERESIS_OFF` | 70 | Soil moisture % above which relay turns OFF (wet) |
| `RELAY_MIN_DURATION_MS` | 30000 | Minimum relay on/off duration (30 seconds) |
| `MIN_DISTANCE_CHANGE` | 1.0 | Minimum distance change (cm) required between calibration points |
| `LOG_BUFFER_SIZE` | 10 | Number of log entries kept in circular buffer |

## Known Limitations

- **Schedule execution is not yet implemented.** Schedules can be created and viewed via the web interface, but automatic relay activation based on schedule times requires an NTP time sync and a background task (see TODO in `webserver.h`).
- **Schedules are not persisted.** They are stored in RAM and lost on reboot. Persistence via SPIFFS or EEPROM should be added.
- **Hysteresis-based auto-watering is defined but not wired into the relay logic.** The `RELAY_HYSTERESIS_ON` and `RELAY_HYSTERESIS_OFF` constants are defined but not used in any automatic relay control loop.
- The web dashboard (`index.h`) primarily shows Zone 3 controls. Zone 4 can be controlled via MQTT or the `/toggleRelay` HTTP endpoint with a `relayZ4` field.

## License

This project is licensed under the MIT License.
