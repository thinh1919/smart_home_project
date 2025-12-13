#ifndef UART_CONFIG_H
#define UART_CONFIG_H

#include <stdint.h>

// Các byte đánh dấu giao thức UART
#define UART_START_BYTE 0xAA  // Byte bắt đầu khung truyền
#define UART_END_BYTE   0x55  // Byte kết thúc khung truyền

// Kích thước tối đa của dữ liệu payload
#define UART_MAX_PAYLOAD_SIZE 256

// Liệt kê các lệnh UART
enum UartCommand : uint8_t {
    SEND_TO_FIREBASE = 0x01,     // Gửi dữ liệu lên Bridge
    SEND_TO_CLIENT   = 0x02,    // Gửi dữ liệu đến Client
    SYSTEM_STATUS    = 0x03,    // Thông tin trạng thái hệ thống
    TIME_SYNC        = 0x04,    //  Lệnh đồng bộ thời gian
    ENERGY_REPORT    = 0x05,    //  Gateway gửi báo cáo năng lượng lên Bridge
};

// Cấu trúc khung truyền UART
// Định dạng: [START] [CMD] [LEN] [PAYLOAD...] [CHECKSUM] [END]
typedef struct {
    uint8_t start;              // Byte bắt đầu: UART_START_BYTE (0xAA)
    UartCommand command;        // Loại lệnh
    uint8_t length;             // Độ dài payload
    uint8_t payload[UART_MAX_PAYLOAD_SIZE]; // Dữ liệu payload
    uint8_t checksum;           // Checksum đơn giản (XOR của CMD + LEN + PAYLOAD)
    uint8_t end;                // Byte kết thúc: UART_END_BYTE (0x55)
} UartFrame;


// Tính toán checksum bằng phép XOR
inline uint8_t calculateChecksum(UartCommand cmd, uint8_t len, const uint8_t* payload) {
    uint8_t checksum = cmd ^ len;
    for (uint8_t i = 0; i < len; i++) {
        checksum ^= payload[i];
    }
    return checksum;
}

// Kiểm tra tính hợp lệ của checksum
inline bool validateChecksum(const UartFrame* frame) {
    return frame->checksum == calculateChecksum(frame->command, frame->length, frame->payload);
}

#endif // UART_CONFIG_H
