#include "sensor_handle.h"

BH1750 lightMeter;

bool initSensors() {
    Wire.begin(); // Khởi tạo I2C
    if (lightMeter.begin()) {
        Serial.println("✅ BH1750 OK");
    } else {
        Serial.println("❌ BH1750 Error");
        return false;
    }
    pinMode(MQ135_PIN, INPUT);
    return true;
}

SensorResult readEnvSensors() {
    SensorResult res;

    // 1. Đọc Lux
    res.lux = lightMeter.readLightLevel();

    // 2. Đọc & Xử lý MQ135 (Theo công thức của bạn)
    int raw = analogRead(MQ135_PIN);

    // --- Chống bão hòa & Lỗi ---
    if (raw <= 0) raw = 1;
    if (raw >= 4090) raw = 4090; // Clamp xác giá trị max

    res.mq135_raw = raw;

    // --- Tính Điện Áp (Vout) ---
    float Vout = (raw * VCC_ADC) / ADC_MAX;

    // --- Tính Trở kháng cảm biến (Rs) ---
    // Công thức: Rs = (Vc - Vout) * RL / Vout
    float Rs = (VCC_ADC - Vout) * MQ135_RL / Vout;
    if (Rs < 0.1) Rs = 0.1; // Tránh chia cho 0 hoặc âm vô lý
    
    res.mq135_rs = Rs;

    // --- Tính PPM (Tham khảo - Có thể bỏ qua nếu chỉ dùng Raw) ---
    // float co2_ppm = 116.602 * pow(Rs / R0, -2.769);

    // --- Phân loại trạng thái (Logic Status) ---
    if (raw < RAW_CLEAN)         res.air_status = 0; // Sạch
    else if (raw < RAW_LIGHT)    res.air_status = 1; // Nhẹ
    else if (raw < RAW_POLLUTED) res.air_status = 2; // Kém
    else                         res.air_status = 3; // Nguy hiểm

    // --- Log Debug ---
    Serial.println("\n===== [SENSOR DEBUG] =====");
    Serial.printf("Lux: %.1f | Raw: %d | Vout: %.2fV | Rs: %.2f kOhm\n", 
                  res.lux, raw, Vout, Rs);
    Serial.printf("Status: %d\n", res.air_status);

    return res;
}