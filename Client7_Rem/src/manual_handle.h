#ifndef MANUAL_HANDLE_H
#define MANUAL_HANDLE_H

#include <Arduino.h>

// ===== CẤU HÌNH CHÂN =====
#define BTN_OPEN_CLOSE   34
#define BTN_MODE         35
#define LED_AUTO         4

// ===== BIẾN TRẠNG THÁI =====
extern bool isManualMode;

// ===== HÀM =====
void initManualControl();
void handleButtons();

#endif