#include "meter_handle.h"

Adafruit_INA219 ina219; 

void initMeter() {
    Wire.begin(SDA_PIN, SCL_PIN);
    
    if (!ina219.begin()) {
        Serial.println("INA219 không tìm thấy! ");
        while (1) { delay(100); }
    }
    
    ina219.setCalibration_32V_2A();
    
    Serial.println("INA219 Ready (Client 11)");
}

MeterReadings readMeter() {
    MeterReadings r;
    
    r.voltage_V = ina219.getBusVoltage_V() + (ina219.getShuntVoltage_mV() / 1000.0);
    r.current_A = ina219.getCurrent_mA() / 1000.0;
    r.power_W   = ina219.getPower_mW() / 1000.0; // Đổi mW sang W
    
    // Lọc nhiễu
    if (r.power_W < 0) r.power_W = 0;
    if (r.current_A < 0) r.current_A = 0;

    Serial.printf("NGUỒN TỔNG: %.2f V | %.3f A | %.2f W\n", 
                  r.voltage_V, r.current_A, r.power_W);
                  
    return r;
}