/*
 * Web Server Handlers for Smart Watering System
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * Defines web server routes and handlers for the web interface, including dashboard,
 * calibration, diagnostics, logs, and scheduling.
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "utils.h"
#include "sensors.h"
#include "mqtt.h"
#include "index.h"
#include "log.h"
#include "diagnostics.h"
#include "schedule.h"

extern AsyncWebServer webServer;
extern DNSServer dnsServer;
extern bool inFallbackAP;
extern String storedSsid;
extern String storedPass;
extern bool relayZone3State;
extern bool relayZone4State;
extern unsigned long relayZone3LastChange;
extern unsigned long relayZone4LastChange;
extern bool ledState;
extern Schedule schedules[10];
extern int scheduleCount;

void handleCaptivePortal(AsyncWebServerRequest *request) {
    String html = "<html><body style='font-family:Arial; text-align:center;'>";
    html += "<h1>Smart Watering Setup</h1>";
    html += "<form action='/save' method='POST'>";
    html += "<p>WiFi SSID: <input type='text' name='ssid' maxlength='32' required></p>";
    html += "<p>Password: <input type='password' name='pass' maxlength='64' required></p>";
    html += "<p><input type='submit' value='Save'></p>";
    html += "</form></body></html>";
    request->send(200, "text/html", html);
}

void handleSaveCredentials(AsyncWebServerRequest *request) {
    String ssid = request->arg("ssid");
    String pass = request->arg("pass");
    if (ssid.length() > 0 && pass.length() > 0) {
        EEPROM.writeString(SSID_ADDR, ssid);
        EEPROM.writeString(PASS_ADDR, pass);
        EEPROM.commit();
        addLog("Credentials saved");
        request->send(200, "text/html", "<html><body><h1>Credentials Saved</h1><p>Rebooting...</p></body></html>");
        delay(1000);
        ESP.restart();
    } else {
        request->send(400, "text/html", "<html><body><h1>Error</h1><p>Invalid SSID or Password</p></body></html>");
    }
}

void handleRoot(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
}

void handleLogs(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", logs_html);
}

void handleGetLogs(AsyncWebServerRequest *request) {
    StaticJsonDocument<512> doc;
    JsonArray logs = doc.createNestedArray("logs");
    if (xSemaphoreTake(logSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
        for (int i = 0; i < logCount; i++) {
            int idx = (logHead - logCount + i + LOG_BUFFER_SIZE) % LOG_BUFFER_SIZE;
            logs.add(logBuffer[idx]);
        }
        xSemaphoreGive(logSemaphore);
    }
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
}

void handleClearLogs(AsyncWebServerRequest *request) {
    if (xSemaphoreTake(logSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
        logHead = 0;
        logCount = 0;
        for (int i = 0; i < LOG_BUFFER_SIZE; i++) {
            logBuffer[i] = "";
        }
        xSemaphoreGive(logSemaphore);
    }
    addLog("Logs cleared");
    StaticJsonDocument<200> response;
    response["status"] = "success";
    String json;
    serializeJson(response, json);
    request->send(200, "application/json", json);
}

void handleDiagnosticsPage(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", diagnostics_html);
}

void handleDiagnostics(AsyncWebServerRequest *request) {
    StaticJsonDocument<200> doc;
    doc["heap"] = ESP.getFreeHeap();
    doc["uptime"] = millis();
    doc["rssi"] = WiFi.RSSI();
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
}

void handleReboot(AsyncWebServerRequest *request) {
    addLog("Reboot requested via web");
    StaticJsonDocument<200> response;
    response["status"] = "success";
    String json;
    serializeJson(response, json);
    request->send(200, "application/json", json);
    delay(1000); // Allow response to be sent
    ESP.restart();
}

void handleToggleLed(AsyncWebServerRequest *request) {
    ledState = !ledState;
    digitalWrite(ledPin, ledState ? HIGH : LOW);
    addLog("Test LED " + String(ledState ? "ON" : "OFF"));
    StaticJsonDocument<200> response;
    response["status"] = "success";
    response["ledState"] = ledState ? "ON" : "OFF";
    String json;
    serializeJson(response, json);
    request->send(200, "application/json", json);
}

void handleData(AsyncWebServerRequest *request) {
    StaticJsonDocument<200> doc;
    float waterLevelToSend = waterSensorOk ? waterLevel : -1;
    doc["waterLevel"] = round(waterLevelToSend * 100) / 100.0; // Round to 2 decimal places
    doc["relayZ3"] = relayZone3State ? "ON" : "OFF";
    doc["relayZ4"] = relayZone4State ? "ON" : "OFF";
    doc["wifiConnected"] = wifiConnected;
    doc["soilDataReceived"] = soilDataReceived;
    doc["calibrationStatus"] = isCalibrated ? "Calibrated" : (calibPoint1.valid ? "Need 1 more point" : "Need 2 points");
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
    addLog("Served data: " + json);
}

void handleToggleRelay(AsyncWebServerRequest *request, String body) {
    addLog("Received toggle relay request at /toggleRelay");
    addLog("Request body: " + body);
    
    if (body.length() == 0) {
        addLog("Toggle relay: Empty body received");
        request->send(400, "application/json", "{\"error\":\"Empty body received\"}");
        return;
    }

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);
    if (error) {
        addLog("Toggle relay: Invalid JSON - " + String(error.c_str()));
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    if (!doc.containsKey("relayZ3")) {
        addLog("Toggle relay: Missing relayZ3 field");
        request->send(400, "application/json", "{\"error\":\"Missing relayZ3 field\"}");
        return;
    }

    String relayCommand = doc["relayZ3"];
    addLog("Relay command: " + relayCommand);
    if (relayCommand == "ON" && !relayZone3State) {
        digitalWrite(relayZone3, HIGH);
        relayZone3State = true;
        relayZone3LastChange = millis();
        addLog("Z3 ON via HTTP");
    } else if (relayCommand == "OFF" && relayZone3State) {
        digitalWrite(relayZone3, LOW);
        relayZone3State = false;
        relayZone3LastChange = millis();
        addLog("Z3 OFF via HTTP");
    } else {
        addLog("Toggle relay: No state change needed");
    }

    StaticJsonDocument<200> response;
    response["relayZ3"] = relayZone3State ? "ON" : "OFF";
    String json;
    serializeJson(response, json);
    addLog("Sending toggle response: " + json);
    request->send(200, "application/json", json);
}

void handleCalibrate(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html); // Serve same page, JavaScript handles /calibrate
}

void handleSaveCalibration(AsyncWebServerRequest *request) {
    addLog("Received calibration request");
    String body = request->arg("plain");
    addLog("Calibration body: " + body);
    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, body)) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        addLog("Calibration: Invalid JSON");
        return;
    }
    if (!doc.containsKey("waterLevelPercent")) {
        request->send(400, "application/json", "{\"error\":\"Missing waterLevelPercent\"}");
        addLog("Calibration: Missing waterLevelPercent");
        return;
    }
    float percent = doc["waterLevelPercent"];
    if (percent < 0 || percent > 100) {
        StaticJsonDocument<200> response;
        response["status"] = "error";
        response["message"] = "Invalid percentage: must be 0 to 100";
        response["calibrationStatus"] = isCalibrated ? "Calibrated" : (calibPoint1.valid ? "Need 1 more point" : "Need 2 points");
        String json;
        serializeJson(response, json);
        request->send(400, "application/json", json);
        addLog("Calibration: Invalid percentage");
        return;
    }
    // Measure current distance
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH, 50000);
    if (duration == 0 || duration > 40000) {
        StaticJsonDocument<200> response;
        response["status"] = "error";
        response["message"] = "Sensor error: unable to measure distance";
        response["calibrationStatus"] = isCalibrated ? "Calibrated" : (calibPoint1.valid ? "Need 1 more point" : "Need 2 points");
        String json;
        serializeJson(response, json);
        request->send(400, "application/json", json);
        addLog("Calibration: Sensor error");
        return;
    }
    float distance = duration * 0.034 / 2;
    if (distance <= 0 || distance >= 400) {
        StaticJsonDocument<200> response;
        response["status"] = "error";
        response["message"] = "Invalid distance: must be between 0 and 400 cm";
        response["calibrationStatus"] = isCalibrated ? "Calibrated" : (calibPoint1.valid ? "Need 1 more point" : "Need 2 points");
        String json;
        serializeJson(response, json);
        request->send(400, "application/json", json);
        addLog("Calibration: Invalid distance");
        return;
    }
    // Store calibration point
    if (!calibPoint1.valid) {
        calibPoint1.distance = distance;
        calibPoint1.percent = percent;
        calibPoint1.valid = true;
        addLog("Saved Point 1: " + String(percent, 1) + "% at " + String(distance, 1) + " cm");
    } else if (!calibPoint2.valid) {
        if (percent == calibPoint1.percent) {
            StaticJsonDocument<200> response;
            response["status"] = "error";
            response["message"] = "Second point must have different percentage";
            response["calibrationStatus"] = "Need 1 more point";
            String json;
            serializeJson(response, json);
            request->send(400, "application/json", json);
            addLog("Calibration: Same percentage");
            return;
        }
        if (abs(distance - calibPoint1.distance) < MIN_DISTANCE_CHANGE) {
            StaticJsonDocument<200> response;
            response["status"] = "error";
            response["message"] = "Water level must change significantly for second point";
            response["calibrationStatus"] = "Need 1 more point";
            String json;
            serializeJson(response, json);
            request->send(400, "application/json", json);
            addLog("Calibration: Insufficient distance change");
            return;
        }
        calibPoint2.distance = distance;
        calibPoint2.percent = percent;
        calibPoint2.valid = true;
        addLog("Saved Point 2: " + String(percent, 1) + "% at " + String(distance, 1) + " cm");
        // Compute calibration
        computeCalibration();
        if (isCalibrated) {
            EEPROM.put(CALIB_POINT1_ADDR, calibPoint1);
            EEPROM.put(CALIB_POINT2_ADDR, calibPoint2);
            EEPROM.commit();
        }
    } else {
        // Reset and start over
        calibPoint1.distance = distance;
        calibPoint1.percent = percent;
        calibPoint1.valid = true;
        calibPoint2.valid = false;
        isCalibrated = false;
        addLog("Reset and saved Point 1: " + String(percent, 1) + "% at " + String(distance, 1) + " cm");
    }
    StaticJsonDocument<200> response;
    response["status"] = "success";
    response["message"] = calibPoint2.valid ? "Calibration complete! Please verify water level." : "Saved point. Change the water level, wait 10 seconds, then add another.";
    response["calibrationStatus"] = isCalibrated ? "Calibrated" : (calibPoint1.valid ? "Need 1 more point" : "Need 2 points");
    String json;
    serializeJson(response, json);
    request->send(200, "application/json", json);
}

void handleResetCalibration(AsyncWebServerRequest *request) {
    calibPoint1.valid = false;
    calibPoint2.valid = false;
    isCalibrated = false;
    EEPROM.put(CALIB_POINT1_ADDR, calibPoint1);
    EEPROM.put(CALIB_POINT2_ADDR, calibPoint2);
    EEPROM.commit();
    addLog("Calibration reset");
    StaticJsonDocument<200> response;
    response["status"] = "success";
    response["message"] = "Calibration reset";
    response["calibrationStatus"] = "Need 2 points";
    String json;
    serializeJson(response, json);
    request->send(200, "application/json", json);
}

void handleSchedulePage(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", schedule_html);
}

void handleGetSchedules(AsyncWebServerRequest *request) {
    StaticJsonDocument<512> doc;
    JsonArray schedulesArray = doc.createNestedArray("schedules");
    for (int i = 0; i < scheduleCount; i++) {
        JsonObject schedule = schedulesArray.createNestedObject();
        schedule["zone"] = schedules[i].zone;
        schedule["startTime"] = schedules[i].startTime;
        schedule["duration"] = schedules[i].duration;
    }
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
}

void handleAddSchedule(AsyncWebServerRequest *request) {
    String body = request->arg("plain");
    addLog("Received add schedule request: " + body);
    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, body)) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        addLog("Add schedule: Invalid JSON");
        return;
    }
    if (!doc.containsKey("zone") || !doc.containsKey("startTime") || !doc.containsKey("duration")) {
        request->send(400, "application/json", "{\"error\":\"Missing fields\"}");
        addLog("Add schedule: Missing fields");
        return;
    }
    if (scheduleCount >= 10) {
        request->send(400, "application/json", "{\"error\":\"Schedule limit reached\"}");
        addLog("Add schedule: Limit reached");
        return;
    }
    String zone = doc["zone"];
    String startTime = doc["startTime"];
    int duration = doc["duration"];
    schedules[scheduleCount].zone = zone;
    schedules[scheduleCount].startTime = startTime;
    schedules[scheduleCount].duration = duration;
    scheduleCount++;
    addLog("Added schedule: " + zone + " at " + startTime + " for " + String(duration) + " min");
    StaticJsonDocument<200> response;
    response["status"] = "success";
    String json;
    serializeJson(response, json);
    request->send(200, "application/json", json);
}

#endif
