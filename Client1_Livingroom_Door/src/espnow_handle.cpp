#include "espnow_handle.h"
#include "door_handle.h" // Để gọi hàm unlockDoor/lockDoor

// Callback gửi
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("📡 Gửi Gateway: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

// Callback nhận (QUAN TRỌNG: Nhận lệnh từ Gateway)
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    ESPNowPacketHeader* header = (ESPNowPacketHeader*)incomingData;

    // Chỉ xử lý tin nhắn COMMAND từ Gateway
    if (header->msg_type == MSG_TYPE_COMMAND) {
        if (len == sizeof(CommandPacket)) {
            CommandPacket* packet = (CommandPacket*)incomingData;
            
            Serial.printf("📥 Lệnh từ Gateway: Type=%d\n", packet->command_type);

            // Xử lý lệnh
            switch (packet->command_type) {
                case DOOR_CMD_OPEN:
                    Serial.println("-> Thực thi lệnh MỞ CỬA");
                    unlockDoor();
                    break;
                    
                case DOOR_CMD_CLOSE: // Hoặc DOOR_CMD_LOCK
                    Serial.println("-> Thực thi lệnh KHÓA CỬA");
                    lockDoor();
                    break;
                    
                default:
                    Serial.println("-> Lệnh không xác định");
                    break;
            }
        }
    }
}

void initESPNow() {
  WiFi.mode(WIFI_STA);
  Serial.print("📡 Client 1 (Door) MAC: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Lỗi Init ESP-NOW");
    delay(1000); ESP.restart();
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv); // Đăng ký hàm nhận

  // Thêm Gateway Peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, gatewayAddress, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("⚠️ Lỗi thêm Peer Gateway");
  } else {
      Serial.println("✅ Đã kết nối Gateway");
  }
}

void sendDoorStatusToGateway() {
    DoorPacket packet;
    
    // Header
    packet.header = createPacketHeader(CLIENT_ID_DOOR, MSG_TYPE_STATUS_UPDATE, sizeof(DoorData));
    
    // Payload
    packet.data.is_open = isDoorOpen;
    packet.data.command = DOOR_CMD_NONE; // Đây là trạng thái trả về, không phải lệnh
    
    // Checksum
    packet.checksum = calculatePacketChecksum(packet);

    // Gửi đi
    esp_now_send(gatewayAddress, (uint8_t*)&packet, sizeof(packet));
}