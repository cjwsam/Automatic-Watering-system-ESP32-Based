/*
 * Display Management for Smart Watering System
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * Handles TFT display rendering for water level, soil moisture, system status, and logs.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include "config.h"
#include "utils.h"
#include "sensors.h"
#include "mqtt.h"

extern TFT_eSPI tft;
extern TFT_eSprite spr;
extern volatile bool displayOn;
extern volatile int currentPage;
extern float wavePhase;
extern bool wifiConnected;

void displayWaterPage() {
    drawGradientBackground(TFT_BLUE, TFT_NAVY);
    spr.setTextDatum(MC_DATUM);
    spr.setFreeFont(&FreeMonoBold12pt7b);
    spr.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
    spr.drawString("Water Level", spr.width() / 2, 10);

    int tankX = 70, tankY = 30, tankWidth = 100, tankHeight = 80;
    spr.drawRoundRect(tankX, tankY, tankWidth, tankHeight, 10, TFT_WHITE);
    for (int level = 20; level <= 80; level += 20) {
        int y = tankY + tankHeight - (level / 100.0 * tankHeight);
        spr.drawFastHLine(tankX + 1, y, tankWidth - 2, TFT_LIGHTGREY);
    }
    if (waterSensorOk) {
        int fillHeight = (waterLevel / 100.0) * tankHeight;
        uint16_t fillColor = (waterLevel > 50) ? TFT_BLUE : (waterLevel > 20) ? TFT_YELLOW : TFT_RED;
        spr.fillRect(tankX + 1, tankY + tankHeight - fillHeight, tankWidth - 2, fillHeight, fillColor);
        if (waterLevel > 0) {
            int waterSurfaceY = tankY + tankHeight - fillHeight;
            for (int i = 0; i < tankWidth - 1; i++) {
                int waveY1 = waterSurfaceY + 4 * sin((i * 0.2) + wavePhase);
                int waveY2 = waterSurfaceY + 4 * sin(((i + 1) * 0.2) + wavePhase);
                waveY1 = constrain(waveY1, tankY, tankY + tankHeight - 1);
                waveY2 = constrain(waveY2, tankY, tankY + tankHeight - 1);
                spr.drawLine(tankX + i, waveY1, tankX + i + 1, waveY2, TFT_WHITE);
            }
        }
        spr.setFreeFont(&FreeMono9pt7b);
        spr.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
        spr.drawString(String(waterLevel, 1) + "%", spr.width() / 2, tankY + tankHeight + 10);
    } else {
        spr.setFreeFont(&FreeMono9pt7b);
        spr.setTextColor(TFT_RED, TFT_TRANSPARENT);
        spr.drawString("Sensor Error", spr.width() / 2, tankY + tankHeight / 2);
    }
    spr.pushSprite(0, 0);
}

void displaySoilPage() {
    drawGradientBackground(TFT_DARKGREEN, TFT_BLACK);
    spr.drawRect(5, 5, spr.width() - 10, spr.height() - 10, TFT_MAGENTA);
    spr.setTextDatum(MC_DATUM);
    spr.setFreeFont(&FreeMonoBold12pt7b);
    spr.setTextColor(TFT_MAGENTA, TFT_TRANSPARENT);
    spr.drawString("Soil & Relays", spr.width() / 2, 20);
    spr.setFreeFont(&FreeMono9pt7b);
    spr.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
    if (!soilDataReceived || !mqttConnected) {
        spr.drawString("No Soil Data", spr.width() / 2, 60);
    } else {
        spr.drawString("Z3: " + String(soilHumidityZone3, 1) + "%", spr.width() / 2, 60);
        spr.drawString("Z4: " + String(soilHumidityZone4, 1) + "%", spr.width() / 2, 80);
        spr.drawString("Relays: " + String(relayZone3State ? "ON" : "OFF") + "/" + String(relayZone4State ? "ON" : "OFF"), spr.width() / 2, 100);
    }
    spr.pushSprite(0, 0);
}

void displayStatusPage() {
    drawGradientBackground(TFT_PURPLE, TFT_BLACK);
    spr.drawRect(5, 5, spr.width() - 10, spr.height() - 10, TFT_YELLOW);
    spr.setTextDatum(MC_DATUM);
    spr.setFreeFont(&FreeMonoBold12pt7b);
    spr.setTextColor(TFT_YELLOW, TFT_TRANSPARENT);
    spr.drawString("System Status", spr.width() / 2, 20);
    spr.setFreeFont(&FreeMono9pt7b);
    spr.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
    spr.drawString("WiFi: " + String(wifiConnected ? "Connected" : "Disconnected"), spr.width() / 2, 50);
    spr.drawString("MQTT: " + String(mqttConnected ? "Connected" : "Disconnected"), spr.width() / 2, 70);
    spr.drawString("Soil: " + String(soilDataReceived && mqttConnected ? "OK" : "No Data"), spr.width() / 2, 90);
    spr.drawString("Water: " + String(waterSensorOk ? "OK" : "Error"), spr.width() / 2, 110);
    spr.pushSprite(0, 0);
}

void displayLogPage() {
    drawGradientBackground(TFT_DARKGREY, TFT_BLACK);
    spr.setTextDatum(TL_DATUM);
    spr.setFreeFont(&FreeMonoBold9pt7b);
    spr.setTextColor(TFT_YELLOW, TFT_BLACK);
    spr.drawString("System Log", 5, 5);
    spr.setFreeFont(&FreeMono9pt7b);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    int y = 30;
    for (int i = 0; i < logCount; i++) {
        int idx = (logHead - logCount + i + LOG_BUFFER_SIZE) % LOG_BUFFER_SIZE;
        String msg = logBuffer[idx];
        if (msg.length() > 20) msg = msg.substring(0, 17) + "...";
        spr.drawString(msg, 3, y);
        y += 20;
        if (y > 110) break;
    }
    spr.pushSprite(0, 0);
}

void displayTask(void *pvParameters) {
    while (1) {
        if (!displayOn) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        if (digitalRead(BUTTON_PIN_NEXT) == LOW) {
            currentPage = (currentPage + 1) % 4;
            delay(50);
            while (digitalRead(BUTTON_PIN_NEXT) == LOW) vTaskDelay(pdMS_TO_TICKS(10));
        }
        wavePhase += 0.1;
        if (wavePhase > 2 * PI) wavePhase -= 2 * PI;
        if (currentPage == WATER_PAGE) displayWaterPage();
        else if (currentPage == SOIL_PAGE) displaySoilPage();
        else if (currentPage == STATUS_PAGE) displayStatusPage();
        else if (currentPage == LOG_PAGE) displayLogPage();
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

#endif
