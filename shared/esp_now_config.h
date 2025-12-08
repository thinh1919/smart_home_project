#ifndef ESP_NOW_CONFIG_H
#define ESP_NOW_CONFIG_H

#include <cstdint>
#include "data_struct.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

// ===== ĐỊA CHỈ MAC CỦA GATEWAY =====
// Thay đổi địa chỉ MAC này theo Gateway thực tế của bạn
// Để lấy MAC address: WiFi.macAddress() hoặc esp_wifi_get_mac()
uint8_t gatewayAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ===== ĐỊA CHỈ MAC CỦA CÁC CLIENT =====
// Thay đổi địa chỉ MAC theo thiết bị thực tế
// Lưu ý: Phải chạy sketch lấy MAC trên từng ESP32 trước
uint8_t clientMacAddress[10][6] = {
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01},  // [1] - Client 1: Cửa phòng khách
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x02},  // [2] - Client 2: Đèn phòng khách
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // [3] - Không dùng (đã xóa cổng)
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x04},  // [4] - Client 4: Cảm biến môi trường phòng khách
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x07},  // [7] - Client 7: Rèm cửa phòng ngủ
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x08},  // [8] - Client 8: Cảm biến môi trường phòng ngủ
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x09},  // [9] - Client 9: Quạt phòng ngủ
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x0A}   // [10] - Client 10: Máy lọc không khí 
};
// Lưu ý: Client 10 (Máy lọc không khí) sẽ thêm vào mảng khi cần mở rộng

// ===== CẤU HÌNH WIFI CHANNEL =====
// ESP-NOW hoạt động tốt nhất khi tất cả thiết bị cùng channel
#define WIFI_CHANNEL 1

// ===== THÔNG TIN CLIENT ID =====
// Định nghĩa ID cho từng client trong hệ thống
#define CLIENT_ID_DOOR          1   // Cửa phòng khách
#define CLIENT_ID_LIGHT         2   // Đèn phòng khách
#define CLIENT_ID_GATE          3   // Cổng
#define CLIENT_ID_ENV_LIVING    4   // Cảm biến môi trường phòng khách
#define CLIENT_ID_CURTAIN       7   // Rèm cửa phòng ngủ
#define CLIENT_ID_ENV_BEDROOM   8   // Cảm biến môi trường phòng ngủ
#define CLIENT_ID_FAN           9   // Quạt phòng ngủ
#define CLIENT_ID_PURIFIER      10  // Máy lọc không khí

// Số lượng client tối đa
#define MAX_CLIENTS             10

// ===== CẤU HÌNH ESP-NOW =====
#define ESP_NOW_MAX_DATA_LEN    250     // Kích thước tối đa của gói tin ESP-NOW
#define ESP_NOW_SEND_TIMEOUT    1000    // Timeout khi gửi (ms)
#define ESP_NOW_MAX_RETRY       3       // Số lần thử lại khi gửi thất bại

// ===== LOẠI TIN NHẮN =====
#define MSG_TYPE_STATUS_UPDATE  0       // Cập nhật trạng thái từ client lên gateway
#define MSG_TYPE_COMMAND        1       // Lệnh từ gateway xuống client
#define MSG_TYPE_SENSOR_DATA    2       // Dữ liệu cảm biến
#define MSG_TYPE_HEARTBEAT      3       // Tin nhắn heartbeat để kiểm tra kết nối
#define MSG_TYPE_ACK            4       // Xác nhận đã nhận

// ===== CẤU HÌNH HEARTBEAT =====
#define HEARTBEAT_INTERVAL      30000   // Gửi heartbeat mỗi 30 giây
#define CLIENT_TIMEOUT          90000   // Client bị coi là mất kết nối sau 90 giây

// ===== GÓI TIN ESP-NOW =====

// Header chung cho tất cả các gói tin
struct ESPNowPacketHeader {
    uint8_t client_id;      // ID của client gửi/nhận (1-10)
    uint8_t msg_type;       // Loại tin nhắn (MSG_TYPE_xxx)
    uint8_t seq_number;     // Số thứ tự gói tin (để phát hiện mất gói)
    uint32_t timestamp;     // Timestamp khi gửi
    uint16_t data_len;      // Độ dài dữ liệu payload
} __attribute__((packed));

// ===== GÓI TIN TỪ CLIENT LÊN GATEWAY =====

// Client 1: Cửa phòng khách
struct DoorPacket {
    ESPNowPacketHeader header;
    DoorData data;
    uint16_t checksum;
} __attribute__((packed));

// Client 2: Đèn phòng khách
struct LightPacket {
    ESPNowPacketHeader header;
    LightData data;
    uint16_t checksum;
} __attribute__((packed));

// Client 3: Cổng
struct GatePacket {
    ESPNowPacketHeader header;
    GateData data;
    uint16_t checksum;
} __attribute__((packed));

// Client 4: Cảm biến môi trường phòng khách
struct EnvSensorPacket {
    ESPNowPacketHeader header;
    EnvSensorData data;
    uint16_t checksum;
} __attribute__((packed));

// Client 7: Rèm cửa phòng ngủ
struct CurtainPacket {
    ESPNowPacketHeader header;
    CurtainData data;
    uint16_t checksum;
} __attribute__((packed));

// Client 8: Cảm biến môi trường phòng ngủ
struct BedroomEnvPacket {
    ESPNowPacketHeader header;
    BedroomEnvData data;
    uint16_t checksum;
} __attribute__((packed));

// Client 9: Quạt phòng ngủ
struct FanPacket {
    ESPNowPacketHeader header;
    FanData data;
    uint16_t checksum;
} __attribute__((packed));

// Client 10: Máy lọc không khí
struct PurifierPacket {
    ESPNowPacketHeader header;
    PurifierData data;
    uint16_t checksum;
} __attribute__((packed));

// ===== GÓI TIN HEARTBEAT =====
struct HeartbeatPacket {
    ESPNowPacketHeader header;
    uint8_t battery_level;  // % pin (nếu dùng pin)
    int8_t rssi;            // Cường độ tín hiệu WiFi
    uint16_t checksum;
} __attribute__((packed));

// ===== GÓI TIN ACK =====
struct AckPacket {
    ESPNowPacketHeader header;
    uint8_t ack_seq_number; // Số thứ tự gói tin được xác nhận
    uint8_t status;         // 0=OK, 1=ERROR
    uint16_t checksum;
} __attribute__((packed));

// ===== GÓI TIN LỆNH TỪ GATEWAY XUỐNG CLIENT =====
struct CommandPacket {
    ESPNowPacketHeader header;
    uint8_t command_type;   // Loại lệnh (tùy theo từng client)
    int16_t value;          // Giá trị lệnh
    uint16_t checksum;
} __attribute__((packed));

// ===== HÀM TRỢ GIÚP =====

// Tính checksum cho gói tin
template<typename T>
uint16_t calculatePacketChecksum(const T& packet) {
    const uint8_t* data = (const uint8_t*)&packet;
    size_t len = sizeof(T) - sizeof(uint16_t); // Trừ đi phần checksum
    uint16_t sum = 0;
    for(size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

// Xác thực checksum
template<typename T>
bool verifyPacketChecksum(const T& packet) {
    uint16_t calculated = calculatePacketChecksum(packet);
    return (calculated == packet.checksum);
}

// Tạo header cho gói tin
// ===== CÁC HÀM TIỆN ÍCH (CHỈ KHI DÙNG VỚI ARDUINO) =====
#ifdef ARDUINO

inline ESPNowPacketHeader createPacketHeader(uint8_t client_id, uint8_t msg_type, uint16_t data_len) {
    static uint8_t seq_counter = 0;
    ESPNowPacketHeader header;
    header.client_id = client_id;
    header.msg_type = msg_type;
    header.seq_number = seq_counter++;
    header.timestamp = millis();
    header.data_len = data_len;
    return header;
}

// In thông tin gói tin (debug)
inline void printPacketHeader(const ESPNowPacketHeader& header) {
    Serial.printf("Client ID: %d | Type: %d | Seq: %d | Time: %lu | Len: %d\n",
                  header.client_id, header.msg_type, header.seq_number, 
                  header.timestamp, header.data_len);
}

// Lấy địa chỉ MAC của client theo ID
inline uint8_t* getClientMacAddress(uint8_t client_id) {
    if (client_id >= 1 && client_id < MAX_CLIENTS) {
        return clientMacAddress[client_id];
    }
    return nullptr;
}

// Kiểm tra client ID có hợp lệ không
inline bool isValidClientId(uint8_t client_id) {
    return (client_id == CLIENT_ID_DOOR || 
            client_id == CLIENT_ID_LIGHT ||
            client_id == CLIENT_ID_ENV_LIVING ||
            client_id == CLIENT_ID_CURTAIN ||
            client_id == CLIENT_ID_ENV_BEDROOM ||
            client_id == CLIENT_ID_FAN ||
            client_id == CLIENT_ID_PURIFIER);
}

#endif // ARDUINO

#endif // ESP_NOW_CONFIG_H