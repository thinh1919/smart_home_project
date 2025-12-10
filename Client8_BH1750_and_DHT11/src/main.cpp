#include<Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>

#include "esp_now_config.h"
#include "data_struct.h"

// ===== CẤU HÌNH PHẦN CỨNG =====
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 10000; // Gửi dữ liệu mỗi 10 giây

// ===== DANH SÁCH ĐỊA CHỈ MAC (Lấy từ device_config.h của bạn) =====
// 1. Gateway
//uint8_t gatewayAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Thay MAC Gateway thật vào đây
// 2. Client 9 (Quạt)
uint8_t fanAddress[] = {0x20, 0xE7, 0xC8, 0x68, 0xA4, 0xFC};     
// 3. Client 7 (Rèm) - MỚI THÊM
uint8_t curtainAddress[] = {0x6C, 0xC8, 0x40, 0x86, 0xF7, 0x98}; //

// ===== CALLBACK GỬI DATA =====
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // In debug gọn nhẹ
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "✅ Gửi OK: " : "❌ Gửi LỖI: ");
  Serial.printf("%02X\n", mac_addr[5]); // Chỉ in byte cuối MAC cho gọn
}

// ===== SETUP ESP-NOW =====
void initESPNow() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) return;
  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;

  // 1. Add Gateway
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  esp_now_add_peer(&peerInfo);

  // 2. Add Quạt
  memcpy(peerInfo.peer_addr, fanAddress, 6);
  esp_now_add_peer(&peerInfo);

  // 3. Add Rèm (QUAN TRỌNG)
  memcpy(peerInfo.peer_addr, curtainAddress, 6);
  esp_now_add_peer(&peerInfo);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin();
  lightMeter.begin();
  initESPNow();
}

void loop() {
  if (millis() - lastSendTime > SEND_INTERVAL) {
    lastSendTime = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    // float lux = lightMeter.readLightLevel(); 
    // Lưu ý: Nếu muốn Rèm đóng theo ánh sáng, bạn nhớ thêm field `lux` vào struct BedroomEnvData nhé!

    if (isnan(h) || isnan(t)) return;

    // --- ĐÓNG GÓI ---
    BedroomEnvPacket packet;
    packet.header = createPacketHeader(CLIENT_ID_ENV_BEDROOM, MSG_TYPE_SENSOR_DATA, sizeof(BedroomEnvData));
    packet.data.temp = t;
    packet.data.hum = h;
    packet.data.lux = lightMeter.readLightLevel();
    packet.checksum = calculatePacketChecksum(packet);

    // --- GỬI ĐA ĐIỂM (Broadcast có địa chỉ) ---
    // 1. Gửi Gateway
    esp_now_send(gatewayAddress, (uint8_t*)&packet, sizeof(packet));
    delay(10); // Delay nhỏ để tránh nghẽn sóng
    
    // 2. Gửi Quạt (Tự động bật mát)
    esp_now_send(fanAddress, (uint8_t*)&packet, sizeof(packet));
    delay(10);

    // 3. Gửi Rèm (Tự động đóng nếu nóng/nắng)
    esp_now_send(curtainAddress, (uint8_t*)&packet, sizeof(packet));
  }
}