#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <Arduino.h>

// ===== ĐỊNH NGHĨA ENUM =====

// Loại lệnh cho cửa và cổng
enum DoorCommand {
    DOOR_CMD_NONE = 0,      // Không có lệnh
    DOOR_CMD_OPEN = 1,      // Mở
    DOOR_CMD_CLOSE = 2,     // Đóng
};

// Chế độ đèn và quạt
enum DeviceMode {
    MODE_OFF = 0,           // Tắt
    MODE_ECO = 1,           // Chế độ tiết kiệm
    MODE_MEDIUM = 2,        // Chế độ vừa
    MODE_STRONG = 3         // Chế độ mạnh
};

// Lệnh điều khiển máy lọc không khí
enum PurifierCommand {
    PURIFIER_CMD_NONE = 0,  // Không có lệnh
    PURIFIER_CMD_ON = 1,    // Bật
    PURIFIER_CMD_OFF = 2    // Tắt
};

// Loại kịch bản tự động
enum SceneType {
    SCENE_NONE = 0,         // Không có kịch bản
    SCENE_ROI_NHA = 1,      // Rời nhà
    SCENE_VE_NHA = 2        // Về nhà
};

// Trạng thái kịch bản
enum SceneStatus {
    SCENE_STATUS_NONE = 0,      // Không hoạt động
    SCENE_STATUS_RUNNING = 1,   // Đang chạy
    SCENE_STATUS_DONE = 2       // Hoàn thành
};

// ===== CLIENT 1: CỬA (Phòng Khách) =====
struct DoorData {
    bool is_open;           // Trạng thái thật của cửa
    uint8_t command;        // Lệnh: NONE, OPEN, CLOSE, LOCK
};

// ===== CLIENT 2: ĐÈN (Phòng Khách) =====
struct LightData {
    uint8_t mode;           // Chế độ hiện tại: 0-3 (Tắt, Eco, Vừa, Mạnh)
    int8_t command;         // Lệnh từ App: -1 = Không có, 0-3=Lệnh
};

// ===== CLIENT 3: CỔNG =====
/*#define MAX_RFID_CARDS 20
struct GateData {
    bool is_open;           // Trạng thái cổng
    uint8_t command;        // Lệnh: OPEN, CLOSE, NONE
    bool sync_card;         // Cờ báo đồng bộ thẻ RFID
    uint8_t card_count;     // Số lượng thẻ
    char list_card[MAX_RFID_CARDS][9];  // Danh sách thẻ RFID (8 ký tự + null terminator)
};*/

// ===== CLIENT 4: CẢM BIẾN MÔI TRƯỜNG (Phòng Khách) =====
struct EnvSensorData {
    uint16_t air_quality;   // Chất lượng không khí (PPM)
};

// ===== CLIENT 7: RÈM CỬA (Phòng Ngủ) =====
struct CurtainData {
    uint8_t position;       // % độ mở hiện tại (0-100)
    int8_t target_pos;      // Vị trí mục tiêu: -1=Không có, 0-100=Vị trí đích
};

// ===== CLIENT 8: CẢM BIẾN MÔI TRƯỜNG (Phòng Ngủ) =====
struct BedroomEnvData {
    float temp;             // Nhiệt độ (°C)
    float hum;              // Độ ẩm (%)
};

// ===== CLIENT 9: QUẠT (Phòng Ngủ) =====
struct FanData {
    uint8_t mode;           // Chế độ hiện tại: 0-3 (Tắt, Eco, Vừa, Mạnh)
    int8_t command;         // Lệnh từ App: -1=Không có, 0-3=Lệnh
};

// ===== CLIENT 10: MÁY LỌC KHÔNG KHÍ (Phòng Khách) =====
struct PurifierData {
    bool state;             // Trạng thái: Bật/Tắt
    uint8_t command;        // Lệnh: ON, OFF, NONE
};

// ===== ĐIỀU KHIỂN KỊCH BẢN =====
struct SceneControl {
    uint8_t type;           // Loại kịch bản: NONE, ROI_NHA, VE_NHA
    uint8_t status;         // Trạng thái: NONE, RUNNING, DONE
};

// ===== TRẠNG THÁI TOÀN BỘ NHÀ (Gateway) =====
struct HomeStatus {
    unsigned long last_updated;     // Thời gian cập nhật
    
    SceneControl scene_control;     // Kịch bản tự động
    
    // Phòng khách
    DoorData living_room_door;
    LightData living_room_light;
    PurifierData living_room_purifier;
    EnvSensorData living_room_env;
    
    // Phòng ngủ
    CurtainData bedroom_curtain;
    FanData bedroom_fan;
    BedroomEnvData bedroom_env;
    
    // Cổng
    GateData gate;
};

// ===== NHẬT KÝ TRUY CẬP =====
struct AccessLog {
    unsigned long timestamp;    // Thời gian
    char user_name[50];         // Tên người mở
    char action[10];            // Hành động: "OPEN" hoặc "CLOSE"
};

// ===== THÔNG TIN THẺ RFID =====
struct RfidCard {
    char card_id[9];        // Mã thẻ RFID (8 ký tự)
    char name[50];          // Tên người dùng
    bool is_active;         // Trạng thái kích hoạt
};

// ===== DỮ LIỆU NĂNG LƯỢNG =====
#define MAX_DAYS_IN_MONTH 31
struct EnergyData {
    char month[8];          // Tháng: "2025-11"
    uint16_t year;          // Năm: 2025
    
    float total_kwh;        // Tổng điện tiêu thụ cả tháng (kWh)
    uint32_t total_cost;    // Tổng tiền (VND)
    
    // Dữ liệu theo ngày: Index 0 = ngày 1, index 30 = ngày 31
    float daily_data[MAX_DAYS_IN_MONTH];
    
    // Mức tiêu thụ theo thiết bị
    float fan_bed;          // Quạt phòng ngủ
    float purifier;         // Máy lọc không khí
    float light_living;     // Đèn phòng khách
    float curtain;          // Rèm cửa
    float others;           // Thiết bị khác
};

// ===== CẤU TRÚC TIN NHẮN CLIENT =====
// Cấu trúc tin nhắn chung cho giao tiếp client-gateway
struct ClientMessage {
    uint8_t client_id;      // ID Client (1-10)
    uint8_t msg_type;       // Loại tin nhắn: 0=Cập nhật trạng thái, 1=Lệnh, 2=Dữ liệu cảm biến
    uint8_t data_size;      // Kích thước dữ liệu
    uint8_t payload[64];    // Dữ liệu payload
    uint16_t checksum;      // Checksum CRC để xác thực
};

// ===== HÀM TRỢ GIÚP =====

// Chuyển đổi loại kịch bản sang chuỗi
inline const char* sceneTypeToString(uint8_t type) {
    switch(type) {
        case SCENE_NONE: return "NONE";
        case SCENE_ROI_NHA: return "ROI_NHA";
        case SCENE_VE_NHA: return "VE_NHA";
        default: return "UNKNOWN";
    }
}

// Chuyển đổi lệnh cửa sang chuỗi
inline const char* doorCommandToString(uint8_t cmd) {
    switch(cmd) {
        case DOOR_CMD_NONE: return "NONE";
        case DOOR_CMD_OPEN: return "OPEN";
        case DOOR_CMD_CLOSE: return "CLOSE";
        default: return "UNKNOWN";
    }
}

// Chuyển đổi lệnh máy lọc không khí sang chuỗi
inline const char* purifierCommandToString(uint8_t cmd) {
    switch(cmd) {
        case PURIFIER_CMD_NONE: return "NONE";
        case PURIFIER_CMD_ON: return "ON";
        case PURIFIER_CMD_OFF: return "OFF";
        default: return "UNKNOWN";
    }
}

// Tính checksum để xác thực tin nhắn
inline uint16_t calculateChecksum(const uint8_t* data, size_t length) {
    uint16_t sum = 0;
    for(size_t i = 0; i < length; i++) {
        sum += data[i];
    }
    return sum;
}

#endif // DATA_STRUCT_H