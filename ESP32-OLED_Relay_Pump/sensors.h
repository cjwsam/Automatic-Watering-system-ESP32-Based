/*
 * Sensor Management for Smart Watering System
 * Author: cjwsam
 * Date: April 30, 2025
 * License: MIT
 *
 * Handles water level measurement using an ultrasonic sensor and calibration logic.
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <EEPROM.h>
#include "config.h"
#include "utils.h"

struct CalibrationPoint {
    float distance; // cm
    float percent;  // %
    bool valid;
};
extern CalibrationPoint calibPoint1;
extern CalibrationPoint calibPoint2;
extern float fullTankDistance;
extern float emptyTankDistance;
extern bool isCalibrated;
extern float waterLevel;
extern bool waterSensorOk;
extern int sensorErrorCount;
extern float waterLevelHistory[SENSOR_SAMPLES];
extern int waterLevelIndex;
extern bool waterLevelInitialized;

float readWaterLevel() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH, 50000);
    float distance = duration * 0.034 / 2;
    float waterLevelPercent = waterLevel; // Default to last reading

    // Validate distance
    if (duration == 0 || duration > 40000 || distance <= 0 || distance >= 400) {
        sensorErrorCount++;
        if (sensorErrorCount >= SENSOR_ERROR_THRESHOLD) {
            waterSensorOk = false;
            addLog("Water sensor err: duration=" + String(duration) + ", distance=" + String(distance, 1));
        }
        return waterLevel; // Return last valid reading
    }

    // Reset error count and mark sensor as OK
    sensorErrorCount = 0;
    waterSensorOk = true;

    // Calculate percentage
    if (isCalibrated) {
        waterLevelPercent = 100.0 * (emptyTankDistance - distance) / (emptyTankDistance - fullTankDistance);
    } else {
        // Uncalibrated: use default range (10–100 cm)
        waterLevelPercent = 100.0 * (emptyTankDistance - distance) / (emptyTankDistance - fullTankDistance);
    }
    waterLevelPercent = constrain(waterLevelPercent, 0.0, 100.0);
    addLog("Distance: " + String(distance, 1) + " cm, Level: " + String(waterLevelPercent, 1) + "%");

    // Moving average filter
    waterLevelHistory[waterLevelIndex] = waterLevelPercent;
    waterLevelIndex = (waterLevelIndex + 1) % SENSOR_SAMPLES;
    if (!waterLevelInitialized && waterLevelIndex == 0) {
        waterLevelInitialized = true;
    }
    float sum = 0.0;
    int count = waterLevelInitialized ? SENSOR_SAMPLES : waterLevelIndex;
    for (int i = 0; i < count; i++) {
        sum += waterLevelHistory[i];
    }
    waterLevel = sum / count; // Update global waterLevel
    return waterLevel;
}

void computeCalibration() {
    if (!calibPoint1.valid || !calibPoint2.valid || calibPoint1.percent == calibPoint2.percent) {
        isCalibrated = false;
        return;
    }
    // Linear interpolation: percent = m * distance + b
    float m = (calibPoint2.percent - calibPoint1.percent) / (calibPoint2.distance - calibPoint1.distance);
    float b = calibPoint1.percent - m * calibPoint1.distance;
    fullTankDistance = (100.0 - b) / m;
    emptyTankDistance = (0.0 - b) / m;
    if (fullTankDistance >= emptyTankDistance || fullTankDistance <= 0 || emptyTankDistance <= 0) {
        isCalibrated = false;
        addLog("Calibration failed: Invalid distances");
        return;
    }
    isCalibrated = true;
    addLog("Calibration computed: Full=" + String(fullTankDistance, 1) + " cm, Empty=" + String(emptyTankDistance, 1) + " cm");
}

void sensorTask(void *pvParameters) {
    while (1) {
        waterLevel = readWaterLevel();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

#endif
