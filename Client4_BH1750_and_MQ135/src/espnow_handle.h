#ifndef ESPNOW_HANDLE_H
#define ESPNOW_HANDLE_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_now_config.h" // File cấu hình chung của cả dự án
#include "data_struct.h"    // File struct chung
#include "esp_now.h"
// Khởi tạo mạng và thêm Peer
void initESPNowNetwork();

// Gửi dữ liệu đi (Gói gọn logic Hybrid vào đây)
void sendSensorDataToHybrid(EnvSensorData data);

#endif