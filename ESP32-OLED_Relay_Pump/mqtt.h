/*
 * MQTT Integration for Smart Watering System
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * Manages MQTT communication for receiving soil moisture data and controlling relays.
 */

#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include "config.h"
#include "utils.h"

extern AsyncWebServer webServer;
extern WiFiClient espClient;
extern PubSubClient mqttClient;
extern const char* mqttBroker;
extern const int mqttPort;
extern float soilHumidityZone3;
extern float soilHumidityZone4;
extern bool soilDataReceived;
extern bool relayZone3State;
extern bool relayZone4State;
extern unsigned long relayZone3LastChange;
extern unsigned long relayZone4LastChange;
extern bool mqttConnected;
extern bool wifiConnected;
extern bool inFallbackAP;
extern DNSServer dnsServer;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Convert payload to string
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.printf("MQTT received on %s: %s\n", topic, message.c_str());

  // Parse JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.printf("JSON parse error: %s\n", error.c_str());
    return;
  }

  if (strcmp(topic, "control/relay/zone3") == 0) {
    const char* relayZ3 = doc["relayZ3"];
    if (relayZ3) {
      String command = String(relayZ3);
      command.toUpperCase(); // Normalize to uppercase
      bool currentState = digitalRead(relayZone3); // Assuming HIGH = ON
      Serial.printf("Z3 Command: %s, Current State: %s\n", command.c_str(), currentState ? "ON" : "OFF");

      if (command == "ON") {
        if (!currentState) {
          digitalWrite(relayZone3, HIGH);
          relayZone3State = true;
          Serial.println("Z3 ON via MQTT");
        } else {
          Serial.println("Z3 already ON, ignoring ON command");
        }
      } else if (command == "OFF") {
        if (currentState) {
          digitalWrite(relayZone3, LOW);
          relayZone3State = false;
          Serial.println("Z3 OFF via MQTT");
        } else {
          Serial.println("Z3 already OFF, ignoring OFF command");
        }
      } else {
        Serial.printf("Invalid Z3 command: %s\n", command.c_str());
      }
    }
  }

  if (strcmp(topic, "control/relay/zone4") == 0) {
    const char* relayZ4 = doc["relayZ4"];
    if (relayZ4) {
      String command = String(relayZ4);
      command.toUpperCase(); // Normalize to uppercase
      bool currentState = digitalRead(relayZone4); // Assuming HIGH = ON
      Serial.printf("Z4 Command: %s, Current State: %s\n", command.c_str(), currentState ? "ON" : "OFF");

      if (command == "ON") {
        if (!currentState) {
          digitalWrite(relayZone4, HIGH);
          relayZone4State = true;
          Serial.println("Z4 ON via MQTT");
        } else {
          Serial.println("Z4 already ON, ignoring ON command");
        }
      } else if (command == "OFF") {
        if (currentState) {
          digitalWrite(relayZone4, LOW);
          relayZone4State = false;
          Serial.println("Z4 OFF via MQTT");
        } else {
          Serial.println("Z4 already OFF, ignoring OFF command");
        }
      } else {
        Serial.printf("Invalid Z4 command: %s\n", command.c_str());
      }
    }
  }

  // Handle incoming soil moisture data from remote sensors
  if (strcmp(topic, "sensors/soil/zone3") == 0) {
    if (doc.containsKey("soilHumidity")) {
      soilHumidityZone3 = doc["soilHumidity"];
      soilDataReceived = true;
      Serial.printf("Soil Z3 humidity: %.1f%%\n", soilHumidityZone3);
    }
  }

  if (strcmp(topic, "sensors/soil/zone4") == 0) {
    if (doc.containsKey("soilHumidity")) {
      soilHumidityZone4 = doc["soilHumidity"];
      soilDataReceived = true;
      Serial.printf("Soil Z4 humidity: %.1f%%\n", soilHumidityZone4);
    }
  }
}

void networkTask(void *pvParameters) {
    while (1) {
        if (inFallbackAP) {
            dnsServer.processNextRequest();
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }
        wifiConnected = (WiFi.status() == WL_CONNECTED);
        if (!wifiConnected) {
            WiFi.reconnect();
            addLog("WiFi reconnecting");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }
        ArduinoOTA.handle();
        if (!mqttClient.connected()) {
            addLog("MQTT attempting connect as " + String(mqttUser));
            if (mqttClient.connect("ESP32Client", mqttUser, mqttPass)) {
                mqttClient.subscribe("sensors/soil/zone3");
                mqttClient.subscribe("sensors/soil/zone4");
                mqttClient.subscribe("control/relay/zone3");
                mqttClient.subscribe("control/relay/zone4");
                mqttConnected = true;
                addLog("MQTT connected and subscribed to topics");
            } else {
                addLog("MQTT connect failed, state: " + String(mqttClient.state()));
            }
        } else {
            mqttClient.loop();
            static unsigned long lastPublish = 0;
            if (millis() - lastPublish > 10000) {
                StaticJsonDocument<200> doc;
                float waterLevelToSend = waterSensorOk ? waterLevel : -1;
                doc["waterLevel"] = round(waterLevelToSend * 100) / 100.0;
                doc["relayZ3"] = relayZone3State ? "ON" : "OFF";
                doc["relayZ4"] = relayZone4State ? "ON" : "OFF";
                doc["wifiConnected"] = wifiConnected;
                doc["soilDataReceived"] = soilDataReceived;
                char json[200];
                serializeJson(doc, json);
                bool published = mqttClient.publish("sensors/environment", json);
                addLog("Published to sensors/environment: " + String(json) + ", Success: " + String(published));
                lastPublish = millis();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#endif
