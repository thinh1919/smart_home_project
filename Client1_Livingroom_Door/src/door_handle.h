#ifndef DOOR_HANDLE_H
#define DOOR_HANDLE_H

#include <Arduino.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// ===== CẤU HÌNH PHẦN CỨNG (Giữ nguyên) =====
#define RELAY_PIN 23
#define BTN_INSIDE_PIN 19
#define LIMIT_SWITCH_PIN 18

const bool RELAY_ACTIVE_LOW = true; 

// ===== BIẾN TOÀN CỤC =====
extern bool isDoorOpen;

// ===== HÀM ĐIỀU KHIỂN =====
void initDoorHardware();
void handleDoorLogic(); // Hàm loop chính của cửa

// Các hàm hành động (được gọi bởi ESP-NOW hoặc Logic nội bộ)
void unlockDoor(); // Mở cửa
void lockDoor();   // Đóng cửa

#endif