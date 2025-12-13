#include <Arduino.h>
#include "meter_handle.h"
#include "espnow_handle.h"

unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL = 10000; // Gửi mỗi 10 giây lên gateway

void setup() {
    Serial.begin(115200);
    
    initMeter();
    initESPNow();
    
    Serial.println("Client 11 (Total Meter) STARTED");
}

void loop() {
    if (millis() - lastSend > SEND_INTERVAL) {
        lastSend = millis();
        
        // 1. Đọc cảm biến
        MeterReadings data = readMeter();
        
        // 2. Gửi về Gateway
        //sendTotalPowerToGateway(data);
    }
}