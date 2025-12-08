#include <Arduino.h>
#include "uart_handler.h"

#ifdef ARDUINO
#include "firebase_handler.h"
#include "firebase_listener.h"
#endif

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
}

void loop() {
#ifdef ARDUINO
  // Xử lý Firebase tasks (token refresh, etc.)
  handleFirebase();
  
  // Xử lý Firebase stream
  handleFirebaseStream();
#endif
  
  // Đọc và xử lý UART stream (non-blocking)
  readUartStream();
  
  // Có thể thêm các task khác ở đây mà không bị block
}