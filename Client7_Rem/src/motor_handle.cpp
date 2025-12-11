#include "motor_handle.h"

// Khởi tạo AccelStepper (Full 4 wire)
AccelStepper curtainMotor(AccelStepper::FULL4WIRE, IN1, IN3, IN2, IN4);

long maxSteps = 4096; // Giả sử 1 vòng hoặc hành trình cụ thể (Cần calibrate thực tế)
uint8_t currentPercent = 0;

void initMotor() {
    curtainMotor.setMaxSpeed(1000.0);
    curtainMotor.setAcceleration(500.0);
    // Giả sử khởi động là đang đóng hoàn toàn (0)
    curtainMotor.setCurrentPosition(0); 
    Serial.println("✅ Motor Init OK");
}

void runMotorLoop() {
    if (curtainMotor.distanceToGo() != 0) {
        curtainMotor.run();
    }
    
    // Cập nhật % hiện tại theo vị trí thực
    long pos = curtainMotor.currentPosition();
    // Constrain để tránh bug
    if (pos < 0) pos = 0;
    if (pos > maxSteps) pos = maxSteps;
    
    currentPercent = map(pos, 0, maxSteps, 0, 100);
}

void setCurtainPercent(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    long targetStep = map(percent, 0, 100, 0, maxSteps);
    curtainMotor.moveTo(targetStep);
    
    Serial.printf("🎬 Motor Target: %d%% (Step: %ld)\n", percent, targetStep);
}

void stopMotor() {
    curtainMotor.stop();
    // Cập nhật lại vị trí dừng làm mốc mới để tránh trôi
    curtainMotor.runToPosition(); 
}

bool isMotorRunning() {
    return curtainMotor.distanceToGo() != 0;
}