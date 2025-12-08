#ifndef FIRESTORE_HANDLER_H
#define FIRESTORE_HANDLER_H

#ifdef ARDUINO
#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <data_struct.h>

// ===== CẤU HÌNH FIRESTORE =====
// Project ID sẽ được định nghĩa trong platformio.ini
#ifndef FIREBASE_PROJECT_ID
#define FIREBASE_PROJECT_ID "your-project-id"
#endif

// ===== KHAI BÁO HÀM =====

/**
 * Khởi tạo Firestore handler
 * Gọi sau khi Firebase đã được khởi tạo
 */
void initFirestoreHandler();

/**
 * Đọc và thực thi kịch bản tự động từ Firestore
 * @param sceneId ID của kịch bản: "VE_NHA" hoặc "ROI_NHA"
 */
void executeScenario(const char* sceneId);

/**
 * Ghi dữ liệu energy vào Firestore (TODO: Implement sau)
 * @param kwh Số kWh tiêu thụ
 * @param deviceName Tên thiết bị
 */
void updateEnergyData(float kwh, const char* deviceName);

#endif // ARDUINO
#endif // FIRESTORE_HANDLER_H
