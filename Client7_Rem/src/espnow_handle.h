#ifndef ESPNOW_HANDLE_H
#define ESPNOW_HANDLE_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_now_config.h" // File config chung (Chứa MAC, ID...)
#include "data_struct.h"    // File struct chung
#include "esp_now.h"

// ===== HÀM =====
void initESPNow();
void sendCurtainStatusToGateway();

// Logic tự động hóa dựa trên Lux nhận được
void processAutoLogic(float lux);

#endif