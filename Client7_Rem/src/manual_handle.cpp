#include "manual_handle.h"
#include "motor_handle.h"
#include "espnow_handle.h" // Để gửi báo cáo khi bấm nút

bool isManualMode = false; // Mặc định là Tự động

void initManualControl() {
    pinMode(BTN_OPEN_CLOSE, INPUT_PULLUP); // Hoặc INPUT tùy mạch
    pinMode(BTN_MODE, INPUT_PULLUP);
    pinMode(LED_AUTO, OUTPUT);
    digitalWrite(LED_AUTO, HIGH); // Sáng = Auto
}

void handleButtons() {
    // 1. Xử lý nút Chế độ (Mode)
    static int lastModeBtn = HIGH;
    int currentModeBtn = digitalRead(BTN_MODE);

    if (lastModeBtn == HIGH && currentModeBtn == LOW) {
        isManualMode = !isManualMode;
        digitalWrite(LED_AUTO, !isManualMode); // Auto sáng, Manual tắt
        
        Serial.printf("👉 Mode changed: %s\n", isManualMode ? "MANUAL" : "AUTO");
        
        // Gửi báo cáo về Gateway ngay
        sendCurtainStatusToGateway();
        delay(200); // Debounce
    }
    lastModeBtn = currentModeBtn;

    // 2. Xử lý nút Đóng/Mở (Chỉ hoạt động khi Manual)
    if (isManualMode) {
        static int lastActionBtn = HIGH;
        int currentActionBtn = digitalRead(BTN_OPEN_CLOSE);

        if (lastActionBtn == HIGH && currentActionBtn == LOW) {
            // Logic đơn giản: Đang mở (>50%) thì đóng, đang đóng thì mở
            if (currentPercent > 50) {
                setCurtainPercent(0); // Đóng
            } else {
                setCurtainPercent(100); // Mở
            }
            sendCurtainStatusToGateway();
            delay(200);
        }
        lastActionBtn = currentActionBtn;
    }
}