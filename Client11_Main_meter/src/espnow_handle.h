#ifndef ESPNOW_HANDLE_H
#define ESPNOW_HANDLE_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_now_config.h"
#include "meter_handle.h" // Để lấy struct MeterReadings

void initESPNow();
void sendTotalPowerToGateway(MeterReadings readings);

#endif