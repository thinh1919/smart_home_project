#ifndef METER_HANDLE_H
#define METER_HANDLE_H

#include <Arduino.h>
#include <Adafruit_INA219.h>
#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

// Biến lưu trữ kết quả đọc
struct MeterReadings {
    float voltage_V;
    float current_A;
    float power_W;
};

void initMeter();
MeterReadings readMeter();

#endif