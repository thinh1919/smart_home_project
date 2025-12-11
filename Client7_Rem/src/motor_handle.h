#ifndef MOTOR_MANAGER_H
#define MOTOR_MANAGER_H

#include <Arduino.h>
#include <AccelStepper.h>

// ===== CẤU HÌNH CHÂN MOTOR (ULN2003) =====
#define IN1 26
#define IN2 25
#define IN3 33
#define IN4 32

// ===== CẤU HÌNH AUTO =====
#define LUX_TARGET      150.0  // Ngưỡng ánh sáng mong muốn
#define LUX_TOLERANCE   30.0   // Dung sai

// ===== BIẾN TOÀN CỤC =====
extern AccelStepper curtainMotor;
extern long maxSteps;       // Tổng số bước (Full hành trình)
extern uint8_t currentPercent; // 0-100%

// ===== HÀM =====
void initMotor();
void runMotorLoop();
void setCurtainPercent(int percent); // Điều khiển rèm đến % cụ thể
void stopMotor();
bool isMotorRunning();

#endif