#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <BH1750.h>
#include<esp_now.h>

// Include cấu hình chung (Single Source of Truth)
#include "esp_now_config.h"
#include "data_struct.h"

// ===== CẤU HÌNH PHẦN CỨNG =====
#define MQ135_PIN 34
// Ngưỡng chất lượng không khí (MQ135 Analog Value)
#define GOOD_THRESHOLD 1000
#define MODERATE_THRESHOLD 2000
#define UNHEALTHY_THRESHOLD 3000

// ===== KHAI BÁO ĐỐI TƯỢNG =====
BH1750 lightMeter;

unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 10000; // 10 giây gửi 1 lần

// ===== DANH SÁCH ĐỊA CHỈ MAC (Lấy từ device_config.h của bạn) =====
// 1. Gateway
//uint8_t gatewayAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Thay MAC Gateway thật vào đây
// 2. Máy lọc không khí (Client 5)
uint8_t purifierAddress[] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC};     
// 3. Đèn phòng khách (Client 6)
uint8_t lightAddress[] = {0x30, 0xAE, 0xA4, 0xDD, 0xEE, 0xFF};  

// ===== CALLBACK GỬI DATA =====
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Debug xem gửi cho ai
  // 01: Gateway, 07: Purifier, 08: Light (Ví dụ byte cuối)
  Serial.printf(status == ESP_NOW_SEND_SUCCESS ? "✅ Gửi OK tới ...%02X\n" : "❌ Gửi LỖI tới ...%02X\n", mac_addr[5]);
}

// ===== SETUP ESP-NOW =====
void initESPNow() {
  WiFi.mode(WIFI_STA);
  Serial.print("📡 Client 4 (Env Living) MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Lỗi khởi tạo ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);

  // Helper để thêm peer nhanh
  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;

  // 1. Thêm Gateway (Để báo cáo)
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  esp_now_add_peer(&peerInfo);

  // 2. Thêm Máy lọc không khí (Client 5) - Để tự động lọc khí
  const uint8_t* purifierMac = getClientMacAddress(CLIENT_ID_PURIFIER);
  memcpy(peerInfo.peer_addr, purifierMac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
     Serial.println("⚠️ Chưa config MAC Máy lọc hoặc lỗi thêm Peer");
  }

  // 3. Thêm Đèn phòng khách (Client 6) - Để tự động bật đèn khi tối
  const uint8_t* lightMac = getClientMacAddress(CLIENT_ID_LIGHT);
  memcpy(peerInfo.peer_addr, lightMac, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
     Serial.println("⚠️ Chưa config MAC Đèn hoặc lỗi thêm Peer");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Init BH1750
  if (lightMeter.begin()) {
    Serial.println("✅ BH1750 OK");
  } else {
    Serial.println("❌ BH1750 Error");
  }

  // MQ135 Analog Pin
  pinMode(MQ135_PIN, INPUT);

  initESPNow();
}

void loop() {
  if (millis() - lastSendTime > SEND_INTERVAL) {
    lastSendTime = millis();

    // 1. Đọc cảm biến
    float lux = lightMeter.readLightLevel();
    int mq135_raw = analogRead(MQ135_PIN);

    // Tính toán đơn giản (Logic cũ của bạn)
    // float pollutionPercent = (mq135_raw / 4095.0) * 100.0; 
    
    // In log debug
    Serial.println("\n===== MÔI TRƯỜNG PHÒNG KHÁCH =====");
    Serial.printf("💡 Lux: %.1f\n", lux);
    Serial.printf("💨 MQ135: %d (Raw)\n", mq135_raw);

    // 2. Đóng gói dữ liệu (EnvSensorPacket)
    EnvSensorPacket packet;
    packet.header = createPacketHeader(CLIENT_ID_ENV_LIVING, MSG_TYPE_SENSOR_DATA, sizeof(EnvSensorData));
    
    // Lưu ý: Bạn cần cập nhật struct EnvSensorData trong data_struct.h để chứa đủ các trường này
    packet.data.air_quality = mq135_raw; 
    // packet.data.lux = lux; // <--- Cần thêm vào data_struct.h nếu chưa có
    
    packet.checksum = calculatePacketChecksum(packet);

    // 3. Gửi Đa điểm (Hybrid)
    
    // -> Gửi Gateway (Lưu Log)
    esp_now_send(gatewayAddress, (uint8_t*)&packet, sizeof(packet));
    delay(10); 

    // -> Gửi Máy lọc (Client 5) để nó tự xử lý (Nếu bẩn thì bật)
    const uint8_t* purifierMac = getClientMacAddress(CLIENT_ID_PURIFIER);
    esp_now_send(purifierMac, (uint8_t*)&packet, sizeof(packet));
    delay(10);

    // -> Gửi Đèn (Client 6) để nó tự xử lý (Nếu tối quá thì bật)
    const uint8_t* lightMac = getClientMacAddress(CLIENT_ID_LIGHT);
    esp_now_send(lightMac, (uint8_t*)&packet, sizeof(packet));
  }
}