#ifndef SENSOR_HANDLE_H
#define SENSOR_HANDLE_H

#include <Arduino.h>
#include <BH1750.h>
#include <Wire.h>

// ===== CẤU HÌNH MQ135 (User Config) =====
#define MQ135_PIN   34
#define MQ135_RL    10.0      // kΩ (Trở tải)
#define R0          10.0      // kΩ (Giá trị R0 sau khi hiệu chuẩn)
#define ADC_MAX     4095.0
#define VCC_ADC     3.3       // ESP32 dùng 3.3V

// Ngưỡng phân loại không khí (Theo giá trị Raw)
#define RAW_CLEAN       800
#define RAW_LIGHT       1500
#define RAW_POLLUTED    2500

// Struct lưu kết quả đọc tạm thời
struct SensorResult {
    float lux;
    uint16_t mq135_raw;
    float mq135_rs;           // Trở kháng tính toán (để debug)
    int8_t air_status;        // 0=Sạch, 1=Nhẹ, 2=Kém, 3=Nguy hiểm
};

// Hàm khởi tạo và đọc
bool initSensors();
SensorResult readEnvSensors();

#endif