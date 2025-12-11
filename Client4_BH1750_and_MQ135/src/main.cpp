#include <Arduino.h>
#include "sensor_handle.h"
#include "espnow_handle.h"
#include "data_struct.h"

unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 10000; // 10 giây

void setup() {
    Serial.begin(115200);
    
    // 1. Khởi tạo Cảm biến
    initSensors();

    // 2. Khởi tạo Mạng
    initESPNowNetwork();
}

void loop() {
    if (millis() - lastSendTime > SEND_INTERVAL) {
        lastSendTime = millis();

        // BƯỚC 1: Đọc cảm biến (Logic MQ135 phức tạp nằm trong này)
        SensorResult res = readEnvSensors();

        // BƯỚC 2: Chuyển đổi sang Struct chuẩn để gửi
        EnvSensorData dataToSend;
        dataToSend.mq135 = res.mq135_raw;          // Giá trị Raw
        dataToSend.air_quality_status = res.air_status; // Trạng thái 0-3
        dataToSend.lux = res.lux;                  // Ánh sáng

        // BƯỚC 3: Gửi đi (Logic Hybrid nằm trong này)
        sendSensorDataToHybrid(dataToSend);
    }
}