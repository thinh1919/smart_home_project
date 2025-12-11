#ifndef ESPNOW_HANDLE_H
#define ESPNOW_HANDLE_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_now_config.h" // File cấu hình chung
#include "data_struct.h"    // File struct chung

void initESPNow();
void sendDoorStatusToGateway();

#endif