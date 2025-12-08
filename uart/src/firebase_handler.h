#ifndef FIREBASE_HANDLER_H
#define FIREBASE_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>

#ifdef ARDUINO
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#endif

#include <firebase_config.h>
#include <esp_now_config.h>
#include <data_struct.h>

// ===== KHAI BÁO HÀM =====

/**
 * Khởi tạo Firebase connection
 * Gọi hàm này trong setup() sau khi kết nối WiFi
 */
void initFirebase();

/**
 * Xử lý các task của Firebase (cần gọi trong loop)
 */
void handleFirebase();

/**
 * Upload dữ liệu từ các client lên Firebase RTDB
 * Hàm này được gọi từ processValidPacket()
 * 
 * @param payload Con trỏ tới dữ liệu payload (bao gồm ESPNowPacketHeader)
 * @param len Độ dài payload
 */
void uploadToFirebase(uint8_t* payload, int len);

/**
 * Kiểm tra trạng thái kết nối Firebase
 */
bool isFirebaseReady();

#endif // FIREBASE_HANDLER_H
