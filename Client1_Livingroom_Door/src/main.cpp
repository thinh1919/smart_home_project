#include <Arduino.h>
#include "door_handle.h"
#include "espnow_handle.h"

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  
  // 1. Khởi tạo Phần cứng Cửa (LCD, Keypad, Relay...)
  initDoorHardware();

  // 2. Khởi tạo Mạng ESP-NOW
  initESPNow();
  
  Serial.println("🚀 Client 1 (Door) STARTED");
}

// ================== LOOP ==================
void loop() {
  // Chạy logic chính của cửa (quét phím, check công tắc hành trình...)
  handleDoorLogic();
}