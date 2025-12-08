#include "uart_handler.h"

#ifdef ARDUINO
#include "firebase_handler.h"
#endif

// ===== BIẾN TOÀN CỤC CHO STATE MACHINE =====
static UartState currentState = WAITING_START;
static UartCommand currentCommand;
static uint8_t payloadLength = 0;
static uint8_t payloadBuffer[UART_MAX_PAYLOAD_SIZE];
static uint8_t payloadIndex = 0;
static uint8_t receivedChecksum = 0;
static unsigned long lastByteTime = 0;

// Timeout để reset state machine nếu không nhận được dữ liệu liên tục
#define UART_TIMEOUT_MS 1000

// Khai báo Serial2 cho ESP32
HardwareSerial Serial2(2);

/**
 * Khởi tạo UART handler
 */
void initUartHandler() {
    // Khởi tạo Serial2 với các pin RX/TX
    Serial2.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    
    Serial.println("UART Handler đã khởi tạo");
    Serial.printf("UART Config: RX=%d, TX=%d, Baud=%d\n", 
                  UART_RX_PIN, UART_TX_PIN, UART_BAUD_RATE);
    
    // Reset state machine
    currentState = WAITING_START;
    payloadIndex = 0;
    lastByteTime = millis();
}

/**
 * Reset state machine về trạng thái ban đầu
 */
static void resetStateMachine() {
    currentState = WAITING_START;
    payloadIndex = 0;
    payloadLength = 0;
    receivedChecksum = 0;
}

/**
 * Đọc stream UART không chặn (non-blocking)
 * Sử dụng state machine để xử lý từng byte
 */
void readUartStream() {
    // Kiểm tra timeout - reset nếu quá lâu không nhận byte
    if (currentState != WAITING_START && 
        (millis() - lastByteTime) > UART_TIMEOUT_MS) {
        Serial.println("UART timeout - reset state machine");
        resetStateMachine();
    }
    
    // Đọc tất cả các byte có sẵn trong buffer
    while (Serial2.available() > 0) {
        uint8_t incomingByte = Serial2.read();
        lastByteTime = millis();
        
        switch (currentState) {
            case WAITING_START:
                // Chờ byte bắt đầu
                if (incomingByte == UART_START_BYTE) {
                    currentState = READING_COMMAND;
                    // Serial.println("-> Phát hiện START byte");
                }
                // Bỏ qua các byte khác (noise)
                break;
                
            case READING_COMMAND:
                // Đọc byte lệnh
                if (incomingByte >= SEND_TO_FIREBASE && incomingByte <= SYSTEM_STATUS) {
                    currentCommand = (UartCommand)incomingByte;
                    currentState = READING_LENGTH;
                    // Serial.printf("-> CMD: 0x%02X\n", incomingByte);
                } else {
                    // Lệnh không hợp lệ - reset
                    Serial.printf("Lỗi: CMD không hợp lệ (0x%02X)\n", incomingByte);
                    resetStateMachine();
                }
                break;
                
            case READING_LENGTH:
                // Đọc độ dài payload
                payloadLength = incomingByte;
                if (payloadLength > 0 && payloadLength <= UART_MAX_PAYLOAD_SIZE) {
                    currentState = READING_PAYLOAD;
                    payloadIndex = 0;
                    // Serial.printf("-> LEN: %d bytes\n", payloadLength);
                } else if (payloadLength == 0) {
                    // Payload rỗng - chuyển đến checksum
                    currentState = READING_CHECKSUM;
                } else {
                    // Độ dài không hợp lệ
                    Serial.printf("Lỗi: LEN không hợp lệ (%d)\n", payloadLength);
                    resetStateMachine();
                }
                break;
                
            case READING_PAYLOAD:
                // Đọc từng byte của payload
                payloadBuffer[payloadIndex++] = incomingByte;
                
                if (payloadIndex >= payloadLength) {
                    // Đã đọc đủ payload
                    currentState = READING_CHECKSUM;
                    // Serial.println("-> Đã đọc xong PAYLOAD");
                }
                break;
                
            case READING_CHECKSUM:
                // Đọc checksum
                receivedChecksum = incomingByte;
                currentState = READING_END;
                // Serial.printf("-> CHECKSUM: 0x%02X\n", receivedChecksum);
                break;
                
            case READING_END:
                // Đọc byte kết thúc
                if (incomingByte == UART_END_BYTE) {
                    // Frame hoàn chỉnh - validate checksum
                    uint8_t calculatedChecksum = calculateChecksum(
                        currentCommand, 
                        payloadLength, 
                        payloadBuffer
                    );
                    
                    if (calculatedChecksum == receivedChecksum) {
                        // Checksum hợp lệ - xử lý packet
                        Serial.printf("✓ Frame hợp lệ: CMD=0x%02X, LEN=%d\n", 
                                     currentCommand, payloadLength);
                        
                        // Gọi hàm xử lý packet
                        processValidPacket(payloadBuffer, payloadLength);
                    } else {
                        // Checksum sai
                        Serial.printf("✗ Checksum sai: nhận=0x%02X, tính=0x%02X\n", 
                                     receivedChecksum, calculatedChecksum);
                    }
                } else {
                    // Không phải END byte
                    Serial.printf("Lỗi: END byte sai (0x%02X)\n", incomingByte);
                }
                
                // Reset state machine cho frame tiếp theo
                resetStateMachine();
                break;
        }
    }
}

/**
 * Xử lý gói tin hợp lệ
 * Hàm này được gọi khi nhận được frame UART hợp lệ
 */
void processValidPacket(uint8_t* payload, int len) {
    Serial.println("========== PACKET HỢP LỆ ==========");
    Serial.printf("Độ dài: %d bytes\n", len);
    
    // In payload dạng hex
    Serial.print("Payload (hex): ");
    for (int i = 0; i < len; i++) {
        Serial.printf("%02X ", payload[i]);
        if ((i + 1) % 16 == 0) Serial.println();
    }
    Serial.println();
    
    // Xử lý dữ liệu tùy theo lệnh
    if (len >= sizeof(ESPNowPacketHeader)) {
        ESPNowPacketHeader* header = (ESPNowPacketHeader*)payload;
        
        switch (currentCommand) {
            case SEND_TO_FIREBASE:
                // Gửi dữ liệu lên Firebase
                Serial.println("📤 Lệnh: SEND_TO_FIREBASE");
#ifdef ARDUINO
                uploadToFirebase(payload, len);
#else
                Serial.println("  (Firebase không khả dụng)");
#endif
                break;
                
            case SEND_TO_CLIENT:
                // TODO: Gửi xuống client qua ESP-NOW
                Serial.println("📥 Lệnh: SEND_TO_CLIENT");
                Serial.println("  (Chưa implement ESP-NOW)");
                break;
                
            case SYSTEM_STATUS:
                // TODO: Xử lý trạng thái hệ thống
                Serial.println("ℹ️  Lệnh: SYSTEM_STATUS");
                Serial.println("  (Chưa implement)");
                break;
                
            default:
                Serial.printf("⚠️  Lệnh không xác định: 0x%02X\n", currentCommand);
                break;
        }
    } else {
        Serial.println("⚠️  Payload quá ngắn, không có header ESP-NOW");
    }
    
    Serial.println("===================================");
}

