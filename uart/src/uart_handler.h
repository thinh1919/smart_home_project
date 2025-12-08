#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include <Arduino.h>
#include <uart_config.h>

// ===== CẤU HÌNH UART =====
#define UART_RX_PIN 16
#define UART_TX_PIN 17
#define UART_BAUD_RATE 115200

// ===== TRẠNG THÁI MÁY TRẠNG THÁI UART =====
enum UartState {
    WAITING_START,      // Đang chờ byte bắt đầu
    READING_COMMAND,    // Đọc byte lệnh
    READING_LENGTH,     // Đọc byte độ dài
    READING_PAYLOAD,    // Đọc dữ liệu payload
    READING_CHECKSUM,   // Đọc checksum
    READING_END         // Đọc byte kết thúc
};

// ===== KHAI BÁO HÀM =====

/**
 * Khởi tạo UART handler
 * Gọi hàm này trong setup()
 */
void initUartHandler();

/**
 * Đọc stream UART không chặn (non-blocking)
 * Gọi hàm này trong loop()
 * Sử dụng state machine để phát hiện và xử lý frame hợp lệ
 */
void readUartStream();

/**
 * Xử lý gói tin hợp lệ sau khi đã validate
 * Hàm này sẽ được gọi bởi readUartStream() khi nhận được frame hợp lệ
 * 
 * @param payload Con trỏ tới dữ liệu payload
 * @param len Độ dài payload
 */
void processValidPacket(uint8_t* payload, int len);

#endif // UART_HANDLER_H
