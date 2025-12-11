#include "espnow_handle.h"

// Callback khi gửi xong
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // In byte cuối của MAC để biết gửi cho ai
    Serial.printf(status == ESP_NOW_SEND_SUCCESS ? ">> Gửi OK (...%02X)\n" : ">> Gửi LỖI (...%02X)\n", mac_addr[5]);
}

void initESPNowNetwork() {
    WiFi.mode(WIFI_STA);
    Serial.print("📡 Client 4 MAC: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK) {
        Serial.println("❌ ESP-NOW Init Failed");
        delay(1000); ESP.restart();
    }
    esp_now_register_send_cb(OnDataSent);

    // Thêm các Peer cần thiết (Dùng hàm getClientMacAddress từ config chung)
    esp_now_peer_info_t peerInfo = {};
    peerInfo.channel = WIFI_CHANNEL;
    peerInfo.encrypt = false;

    // 1. Gateway
    memcpy(peerInfo.peer_addr, gatewayAddress, 6);
    esp_now_add_peer(&peerInfo);

    // 2. Máy lọc (Client 5)
    const uint8_t* purifierMac = getClientMacAddress(CLIENT_ID_PURIFIER);
    memcpy(peerInfo.peer_addr, purifierMac, 6);
    esp_now_add_peer(&peerInfo);

    // 3. Đèn (Client 6)
    const uint8_t* lightMac = getClientMacAddress(CLIENT_ID_LIGHT);
    memcpy(peerInfo.peer_addr, lightMac, 6);
    esp_now_add_peer(&peerInfo);
}

void sendSensorDataToHybrid(EnvSensorData data) {
    // 1. Đóng gói Packet chuẩn
    EnvSensorPacket packet;
    packet.header = createPacketHeader(CLIENT_ID_ENV_LIVING, MSG_TYPE_SENSOR_DATA, sizeof(EnvSensorData));
    packet.data = data; // Copy dữ liệu vào payload
    packet.checksum = calculatePacketChecksum(packet);

    // 2. Gửi Gateway (Lưu log)
    esp_now_send(gatewayAddress, (uint8_t*)&packet, sizeof(packet));
    delay(10); 

    // 3. Gửi Máy lọc (Tự động hóa P2P)
    const uint8_t* purifierMac = getClientMacAddress(CLIENT_ID_PURIFIER);
    esp_now_send(purifierMac, (uint8_t*)&packet, sizeof(packet));
    delay(10);

    // 4. Gửi Đèn (Tự động hóa P2P)
    const uint8_t* lightMac = getClientMacAddress(CLIENT_ID_LIGHT);
    esp_now_send(lightMac, (uint8_t*)&packet, sizeof(packet));
}