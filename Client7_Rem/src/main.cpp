#include <Arduino.h>
#include "motor_handle.h"
#include "manual_handle.h"
#include "espnow_handle.h"
#include "esp_now.h"

// Timer gửi heartbeat hoặc update định kỳ
unsigned long lastUpdate = 0;

void setup() {
    Serial.begin(115200);
    
    // 1. Khởi tạo Motor
    initMotor();
    
    // 2. Khởi tạo Nút bấm
    initManualControl();
    
    // 3. Khởi tạo Mạng
    initESPNow();

    Serial.println("🚀 Client 7 (Curtain) START");
}

void loop() {
    // 1. Luôn chạy vòng lặp motor (AccelStepper cần gọi liên tục)
    runMotorLoop();

    // 2. Kiểm tra nút bấm vật lý
    handleButtons();
    
    // 3. (Tùy chọn) Gửi cập nhật định kỳ mỗi 30s để App đồng bộ chắc chắn
    if (millis() - lastUpdate > 30000) {
        lastUpdate = millis();
        // Chỉ gửi nếu motor đang đứng yên để tránh spam khi đang chạy
        if (!isMotorRunning()) {
            sendCurtainStatusToGateway();
        }
    }
}