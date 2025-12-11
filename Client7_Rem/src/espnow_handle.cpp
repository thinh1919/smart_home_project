#include "espnow_handle.h"
#include "motor_handle.h"
#include "manual_handle.h"
#include "esp_now.h"

// Biến chống spam khi Auto
unsigned long lastAutoActionTime = 0;

// Callback khi nhận dữ liệu
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    ESPNowPacketHeader* header = (ESPNowPacketHeader*)incomingData;

    // -----------------------------------------------------------
    // TRƯỜNG HỢP 1: NHẬN LỆNH TỪ GATEWAY (APP ĐIỀU KHIỂN)
    // -----------------------------------------------------------
    if (header->msg_type == MSG_TYPE_COMMAND) { 
        // Lưu ý: ID gateway thường là chưa định nghĩa trong header gửi xuống, 
        // hoặc check msg_type = COMMAND
        
        if (len == sizeof(CommandPacket)) {
            CommandPacket* packet = (CommandPacket*)incomingData;
            if(!verifyPacketChecksum(*packet)) {
                Serial.println("❌ Lệnh từ Gateway bị lỗi checksum");
                return;
            }
            // App điều khiển -> Chuyển sang Manual Mode để tránh Auto ghi đè
            isManualMode = true; 
            digitalWrite(LED_AUTO, LOW);

            Serial.printf("📥 Lệnh từ Gateway: Val=%d\n", packet->value);

            // Xử lý lệnh
            // Giả sử packet->value gửi 0-100 là % mở
            // Nếu value = -1 -> Lệnh dừng (ví dụ)
            if (packet->command_type == CURTAIN_CMD_SET_POS) {
                setCurtainPercent(packet->value);
            }
            
            // Báo cáo lại trạng thái
            sendCurtainStatusToGateway();
        }
    }

    // -----------------------------------------------------------
    // TRƯỜNG HỢP 2: NHẬN DỮ LIỆU CẢM BIẾN TỪ CLIENT 8 (HYBRID P2P)
    // -----------------------------------------------------------
    else if (header->client_id == CLIENT_ID_ENV_BEDROOM && header->msg_type == MSG_TYPE_SENSOR_DATA) {
        BedroomEnvPacket* packet = (BedroomEnvPacket*)incomingData;
        
        if (verifyPacketChecksum(*packet)) {
            float lux = packet->data.lux;
            Serial.printf("🌤 Nhận Lux từ Client 8: %.1f\n", lux);
            
            // Gọi hàm xử lý logic tự động
            processAutoLogic(lux);
        }
    }
}

void initESPNow() {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("❌ ESP-NOW Init Failed");
        ESP.restart();
    }
    
    // Đăng ký hàm nhận
    esp_now_register_recv_cb(OnDataRecv);

    // Thêm Gateway Peer (Để gửi báo cáo status)
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, gatewayAddress, 6);
    peerInfo.channel = WIFI_CHANNEL;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
}

void sendCurtainStatusToGateway() {
    CurtainPacket packet;
    
    // Header
    packet.header = createPacketHeader(CLIENT_ID_CURTAIN, MSG_TYPE_STATUS_UPDATE, sizeof(CurtainData));
    
    // Payload (Mapping struct CurtainData mới)
    packet.data.isOpen = (currentPercent > 0);
    packet.data.position = currentPercent;
    packet.data.curtainPercent = -1; // -1 nghĩa là đang ở trạng thái nghỉ, ko có target pending
    packet.data.manualMode = isManualMode;
    
    // Checksum
    packet.checksum = calculatePacketChecksum(packet);

    // Gửi đi
    esp_now_send(gatewayAddress, (uint8_t*)&packet, sizeof(packet));
    Serial.println("📤 Đã gửi Status về Gateway");
}

void processAutoLogic(float lux) {
    // Nếu đang Manual Mode thì bỏ qua Auto
    if (isManualMode) return;

    // Chống spam lệnh liên tục (mỗi 5s mới check 1 lần)
    if (millis() - lastAutoActionTime < 5000) return;

    // LOGIC:
    // Sáng quá (Nắng) -> Đóng rèm
    if (lux > (LUX_TARGET + LUX_TOLERANCE)) {
        if (currentPercent > 0) { // Nếu đang mở
            Serial.println("☀️ Nắng quá -> Auto ĐÓNG");
            setCurtainPercent(0); 
            lastAutoActionTime = millis();
            sendCurtainStatusToGateway();
        }
    }
    // Tối (Hoặc sáng dịu) -> Mở rèm (Tùy logic bạn muốn)
    else if (lux < (LUX_TARGET - LUX_TOLERANCE)) {
        if (currentPercent < 100) { // Nếu đang đóng
            Serial.println("☁️ Trời râm/Tối -> Auto MỞ");
            setCurtainPercent(100);
            lastAutoActionTime = millis();
            sendCurtainStatusToGateway();
        }
    }
}