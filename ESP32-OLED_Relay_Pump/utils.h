/*
 * Utility Functions for Smart Watering System
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * Provides utility functions for drawing gradients and managing logs.
 */

#ifndef UTILS_H
#define UTILS_H

#include <TFT_eSPI.h>
#include "config.h"

extern TFT_eSprite spr;
extern String logBuffer[LOG_BUFFER_SIZE];
extern int logHead;
extern int logCount;
extern SemaphoreHandle_t logSemaphore;

void drawGradientBackground(uint16_t startColor, uint16_t endColor) {
    for (int y = 0; y < spr.height(); y++) {
        uint16_t color = spr.alphaBlend(y * 255 / spr.height(), startColor, endColor);
        spr.drawFastHLine(0, y, spr.width(), color);
    }
}

void addLog(String message) {
    if (xSemaphoreTake(logSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
        logBuffer[logHead] = message.substring(0, 50);
        logHead = (logHead + 1) % LOG_BUFFER_SIZE;
        if (logCount < LOG_BUFFER_SIZE) logCount++;
        xSemaphoreGive(logSemaphore);
    }
    Serial.println(message); // Send to serial monitor
}

#endif
