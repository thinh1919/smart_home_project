#include <Arduino.h>
#include "uart_handler.h"

#ifdef ARDUINO
#include "firebase_handler.h"
#include "firebase_listener.h"
#endif

// Timer cho batch upload (60 giây)
unsigned long lastSyncMillis = 0;
const unsigned long SYNC_INTERVAL = 60000; // 60 seconds

void setup() {
  // Khởi tạo Serial cho debug
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("========================================");
  Serial.println("WiFi Bridge (ESP32) đã khởi động");
  Serial.println("========================================");
  
#ifdef ARDUINO
  // Khởi tạo Firebase (bao gồm WiFi)
  initFirebase();
  
  // Khởi tạo Firebase listener
  initFirebaseListener();
#endif
  
  // Khởi tạo UART handler
  initUartHandler();
  
  Serial.println("Sẵn sàng nhận dữ liệu từ Gateway...");
  Serial.printf("Batch upload interval: %lu giây\n", SYNC_INTERVAL / 1000);
}

void loop() {
#ifdef ARDUINO
  // Xử lý Firebase tasks (token refresh, etc.)
  handleFirebase();
  
  // Xử lý Firebase stream
  handleFirebaseStream();
  
  // Batch upload: Đồng bộ dữ liệu mỗi 60 giây
  unsigned long currentMillis = millis();
  if (currentMillis - lastSyncMillis >= SYNC_INTERVAL) {
    lastSyncMillis = currentMillis;
    syncDataToFirebase();
  }
#endif
  
  // Đọc và xử lý UART stream (non-blocking)
  readUartStream();
  
  // Có thể thêm các task khác ở đây mà không bị block
}