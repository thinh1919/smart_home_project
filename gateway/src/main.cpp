#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
// Sửa lại đường dẫn include cho đúng với cấu trúc thư mục của bạn
#include "../../shared/uart_config.h" 
#include "../../shared/ESP_NOW_CONFIG.h"

// ===== CẤU HÌNH UART =====
#define UART_RX_PIN 18
#define UART_TX_PIN 17
#define UART_BAUD_RATE 115200

// LƯU Ý: Không khai báo HardwareSerial Serial2(2) ở đây nữa để tránh xung đột
// ESP32-S3 tự hiểu Serial2 nếu khai báo pins đúng trong begin()

// ===== BUFFER UART =====
static uint8_t uart_buffer[UART_MAX_PAYLOAD_SIZE + 10];
static int uart_buffer_index = 0;

// ===== KHAI BÁO HÀM =====
void forwardPacketToBridge(const uint8_t *data, int len);
void readUartFromBridge();
void sendCommandToClient(uint8_t client_id, uint8_t cmd_type, int16_t value);
void initESPNow();
void onDataRecv(const uint8_t *mac, const uint8_t *data, int len);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("========================================");
  Serial.println("Gateway Hub (ESP32-S3) khởi động...");
  Serial.println("========================================");
  
  // Khởi tạo WiFi ở chế độ Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.print("Gateway MAC: ");
  Serial.println(WiFi.macAddress());
  
  // Khởi tạo ESP-NOW
  initESPNow();
  
  // Khởi tạo UART nối sang WiFi Bridge
  // SERIAL_8N1: 8 data bits, No parity, 1 stop bit (Chuẩn phổ biến nhất)
  Serial2.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  
  Serial.println("✅ Gateway sẵn sàng!");
  Serial.println("========================================");
}

void loop() {
  // Đọc lệnh từ WiFi Bridge qua UART
  readUartFromBridge();
  
  // ESP-NOW callbacks chạy trong interrupt nên không cần xử lý thêm ở đây
}

// ===== HÀM XỬ LÝ QUAN TRỌNG =====

void forwardPacketToBridge(const uint8_t *data, int len) {
  // 1. Kiểm tra an toàn: ESP-NOW max 250 bytes, uint8_t max 255
  if (len <= 0 || len > 250) { 
    Serial.printf("[ERR] Packet length invalid: %d\n", len);
    return;
  }
  
  // 2. Tạo buffer tĩnh để tránh lỗi tràn RAM (Stack Overflow)
  // Kích thước = Start(1) + Cmd(1) + Len(1) + MaxPayload(250) + Checksum(1) + End(1) = 255
  uint8_t frame[256]; 
  int idx = 0;
  
  // --- ĐÓNG GÓI (PACKING) ---
  
  frame[idx++] = UART_START_BYTE;           // 0xAA
  frame[idx++] = SEND_TO_FIREBASE;          // CMD
  frame[idx++] = (uint8_t)len;              // LENGTH
  
  memcpy(&frame[idx], data, len);           // PAYLOAD
  idx += len;
  
  // Tính checksum
  // Lưu ý: data ở đây chính là payload
  frame[idx++] = calculateChecksum(SEND_TO_FIREBASE, (uint8_t)len, data); 
  
  frame[idx++] = UART_END_BYTE;             // 0x55
  
  // --- GỬI ĐI ---
  
  Serial2.write(frame, idx);
  // Serial2.flush() không bắt buộc trên ESP32 nhưng dùng để chắc chắn gửi hết
  // Serial2.flush(); 
  
  Serial.printf(">> Forwarded UART: %d bytes\n", idx);
}

// ===== ĐỌC UART TỪ WIFI BRIDGE =====

void readUartFromBridge() {
  // Đọc từng byte từ Serial2
  while (Serial2.available()) {
    uint8_t byte = Serial2.read();
    
    // Tìm byte bắt đầu
    if (uart_buffer_index == 0) {
      if (byte == UART_START_BYTE) {
        uart_buffer[uart_buffer_index++] = byte;
      }
      continue;
    }
    
    // Lưu vào buffer
    uart_buffer[uart_buffer_index++] = byte;
    
    // Tránh tràn buffer
    if (uart_buffer_index >= sizeof(uart_buffer)) {
      Serial.println("[UART] Buffer overflow, reset");
      uart_buffer_index = 0;
      continue;
    }
    
    // Kiểm tra xem đã nhận đủ frame chưa
    if (uart_buffer_index >= 3) {
      uint8_t cmd = uart_buffer[1];
      uint8_t len = uart_buffer[2];
      int expected_len = 1 + 1 + 1 + len + 1 + 1; // START + CMD + LEN + PAYLOAD + CHECKSUM + END
      
      if (uart_buffer_index >= expected_len) {
        // Kiểm tra byte kết thúc
        if (uart_buffer[expected_len - 1] == UART_END_BYTE) {
          // Xác thực checksum
          uint8_t received_checksum = uart_buffer[expected_len - 2];
          uint8_t calculated_checksum = calculateChecksum((UartCommand)cmd, len, &uart_buffer[3]);
          
          if (received_checksum == calculated_checksum) {
            // Frame hợp lệ, xử lý lệnh
            if (cmd == SEND_TO_CLIENT) {
              // Parse CommandPayload
              if (len == sizeof(CommandPayload)) {
                CommandPayload* payload = (CommandPayload*)&uart_buffer[3];
                Serial.printf("<< [UART] Command for Client %d: Cmd=0x%02X, Value=%d\n", 
                              payload->client_id, payload->command_type, payload->value);
                
                // Gửi xuống client qua ESP-NOW
                sendCommandToClient(payload->client_id, payload->command_type, payload->value);
              } else {
                Serial.printf("[UART] Invalid payload length: %d (expected %d)\n", len, sizeof(CommandPayload));
              }
            }
          } else {
            Serial.printf("[UART] Checksum mismatch: %02X != %02X\n", received_checksum, calculated_checksum);
          }
        }
        
        // Reset buffer để nhận frame tiếp theo
        uart_buffer_index = 0;
      }
    }
  }
}

// ===== GỬI LỆNH XUỐNG CLIENT QUA ESP-NOW =====

void sendCommandToClient(uint8_t client_id, uint8_t cmd_type, int16_t value) {
  Serial.printf("📤 [ESP-NOW] Sending command to Client %d: Type=0x%02X, Value=%d\n", 
                client_id, cmd_type, value);
  
  // Kiểm tra client ID hợp lệ
  if (!isValidClientId(client_id)) {
    Serial.printf("❌ [ESP-NOW] Invalid client ID: %d\n", client_id);
    return;
  }
  
  // Lấy địa chỉ MAC của client
  uint8_t* clientMac = getClientMacAddress(client_id);
  if (clientMac == nullptr) {
    Serial.printf("❌ [ESP-NOW] No MAC address for client %d\n", client_id);
    return;
  }
  
  // Tạo gói tin command
  // Structure: [Header][CommandPayload][Checksum]
  struct CommandPacket {
    ESPNowPacketHeader header;
    CommandPayload payload;
    uint16_t checksum;
  } __attribute__((packed));
  
  CommandPacket packet;
  
  // Tạo header
  packet.header = createPacketHeader(client_id, MSG_TYPE_COMMAND, sizeof(CommandPayload));
  
  // Điền payload
  packet.payload.client_id = client_id;
  packet.payload.command_type = cmd_type;
  packet.payload.value = value;
  
  // Tính checksum
  packet.checksum = calculatePacketChecksum(packet);
  
  // Gửi qua ESP-NOW
  esp_err_t result = esp_now_send(clientMac, (uint8_t*)&packet, sizeof(packet));
  
  if (result == ESP_OK) {
    Serial.println("✅ [ESP-NOW] Command sent successfully");
  } else {
    Serial.printf("❌ [ESP-NOW] Send failed: %d\n", result);
  }
}

// ===== KHỞI TẠO ESP-NOW =====

void initESPNow() {
  Serial.println("🔧 [ESP-NOW] Initializing...");
  
  // Khởi tạo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ [ESP-NOW] Init failed!");
    return;
  }
  
  Serial.println("✅ [ESP-NOW] Initialized successfully");
  
  // Đăng ký callback nhận dữ liệu
  esp_now_register_recv_cb(onDataRecv);
  
  // Thêm các client vào peer list
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;
  
  // Danh sách các client ID hợp lệ
  uint8_t validClients[] = {
    CLIENT_ID_DOOR,
    CLIENT_ID_LIGHT,
    CLIENT_ID_ENV_LIVING,
    CLIENT_ID_CURTAIN,
    CLIENT_ID_ENV_BEDROOM,
    CLIENT_ID_FAN,
    CLIENT_ID_PURIFIER,
    CLIENT_ID_GATE
  };
  
  for (int i = 0; i < sizeof(validClients); i++) {
    uint8_t clientId = validClients[i];
    uint8_t* mac = getClientMacAddress(clientId);
    
    if (mac != nullptr) {
      memcpy(peerInfo.peer_addr, mac, 6);
      
      if (esp_now_add_peer(&peerInfo) == ESP_OK) {
        Serial.printf("✅ [ESP-NOW] Added Client %d: %02X:%02X:%02X:%02X:%02X:%02X\n",
                     clientId, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      } else {
        Serial.printf("❌ [ESP-NOW] Failed to add Client %d\n", clientId);
      }
    }
  }
}

// ===== CALLBACK NHẬN DỮ LIỆU TỪ CLIENT =====

void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  Serial.printf("📥 [ESP-NOW] Received %d bytes from %02X:%02X:%02X:%02X:%02X:%02X\n",
                len, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                
  if (len >= sizeof(ESPNowPacketHeader) + sizeof(uint16_t)) {
        ESPNowPacketHeader* header = (ESPNowPacketHeader*)data;
        
        // Tính checksum (cần biết loại packet để cast đúng)
        // Hoặc dùng generic:
        uint16_t calculated = 0;
        for (int i = 0; i < len - 2; i++) {
            calculated += data[i];
        }
        uint16_t received = *(uint16_t*)(data + len - 2);
        
        if (calculated != received) {
            Serial.println("❌ Checksum sai, bỏ qua packet");
            return;
        }
    }
  // Forward data lên WiFi Bridge qua UART
  forwardPacketToBridge(data, len);
}