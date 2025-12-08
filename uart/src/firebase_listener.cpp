#ifdef ARDUINO

#include "firebase_listener.h"
#include "firebase_handler.h"
#include "firestore_handler.h"
#include <uart_config.h>
#include <esp_now_config.h>
#include <HardwareSerial.h>

// ===== BIẾN TOÀN CỤC =====
extern HardwareSerial Serial2;
extern FirebaseData fbdo;
static FirebaseData streamData;
static bool listener_ready = false;

// ===== HÀM HỖ TRỢ =====

// Gửi lệnh xuống Gateway qua UART
void sendCommandToGateway(uint8_t client_id, uint8_t cmd_type, int16_t value) {
    Serial.printf("📤 [LISTENER] Gửi lệnh: Client=%d, Cmd=0x%02X, Value=%d\n", 
                  client_id, cmd_type, value);

    // Tạo CommandPayload
    CommandPayload cmd;
    cmd.client_id = client_id;
    cmd.command_type = cmd_type;
    cmd.value = value;

    // Tạo UART Frame
    uint8_t frame[sizeof(CommandPayload) + 5]; // START + CMD + LEN + PAYLOAD + CHECKSUM + END
    int idx = 0;

    frame[idx++] = UART_START_BYTE;
    frame[idx++] = SEND_TO_CLIENT;
    frame[idx++] = sizeof(CommandPayload);
    memcpy(&frame[idx], &cmd, sizeof(CommandPayload));
    idx += sizeof(CommandPayload);
    frame[idx++] = calculateChecksum(SEND_TO_CLIENT, sizeof(CommandPayload), (uint8_t*)&cmd);
    frame[idx++] = UART_END_BYTE;

    // Gửi qua Serial2
    Serial2.write(frame, idx);
    Serial2.flush();

    Serial.println("✅ [LISTENER] Đã gửi frame xuống Gateway");
}

// ===== CALLBACK STREAM =====
void streamCallback(StreamData data) {
    Serial.printf("🔔 [STREAM] Path: %s\n", data.dataPath().c_str());
    Serial.printf("🔔 [STREAM] Type: %s\n", data.dataType().c_str());

    String path = data.dataPath();

    // --- 1. DOOR COMMAND ---
    if (path == "/living_room/door/command") {
        String cmd = data.stringData();
        if (cmd == "OPEN") {
            sendCommandToGateway(CLIENT_ID_DOOR, DOOR_CMD_OPEN, 0);
            Firebase.RTDB.setString(&streamData, "/living_room/door/command", "NONE");
        } else if (cmd == "CLOSE") {
            sendCommandToGateway(CLIENT_ID_DOOR, DOOR_CMD_CLOSE, 0);
            Firebase.RTDB.setString(&streamData, "/living_room/door/command", "NONE");
        } else if (cmd == "LOCK") {
            sendCommandToGateway(CLIENT_ID_DOOR, DOOR_CMD_LOCK, 0);
            Firebase.RTDB.setString(&streamData, "/living_room/door/command", "NONE");
        }
    }

    // --- 2. LIGHT COMMAND ---
    else if (path == "/living_room/light/command") {
        int mode = data.intData();
        if (mode >= 0 && mode <= 3) {
            sendCommandToGateway(CLIENT_ID_LIGHT, mode, 0);
            Firebase.RTDB.setInt(&streamData, "/living_room/light/command", -1);
        }
    }

    // --- 3. PURIFIER COMMAND ---
    else if (path == "/living_room/purifier/command") {
        String cmd = data.stringData();
        if (cmd == "ON") {
            sendCommandToGateway(CLIENT_ID_PURIFIER, PURIFIER_CMD_ON, 0);
            Firebase.RTDB.setString(&streamData, "/living_room/purifier/command", "NONE");
        } else if (cmd == "OFF") {
            sendCommandToGateway(CLIENT_ID_PURIFIER, PURIFIER_CMD_OFF, 0);
            Firebase.RTDB.setString(&streamData, "/living_room/purifier/command", "NONE");
        }
    }

    // --- 4. CURTAIN COMMAND ---
    else if (path == "/bedroom/curtain/target_pos") {
        int pos = data.intData();
        if (pos >= 0 && pos <= 100) {
            sendCommandToGateway(CLIENT_ID_CURTAIN, CURTAIN_CMD_SET_POS, pos);
            Firebase.RTDB.setInt(&streamData, "/bedroom/curtain/target_pos", -1);
        }
    }

    // --- 5. FAN COMMAND ---
    else if (path == "/bedroom/fan/command") {
        int mode = data.intData();
        if (mode >= 0 && mode <= 3) {
            sendCommandToGateway(CLIENT_ID_FAN, mode, 0);
            Firebase.RTDB.setInt(&streamData, "/bedroom/fan/command", -1);
        }
    }

    // --- 6. SCENE CONTROL ---
    else if (path == "/scene_control/type") {
        String sceneType = data.stringData();
        if (sceneType != "NONE") {
            Serial.printf("🎬 [SCENE] Nhận lệnh thực thi: %s\n", sceneType.c_str());
            
            // Set status = RUNNING
            Firebase.RTDB.setString(&streamData, "/scene_control/status", "RUNNING");
            
            // ✅ Gọi hàm thực thi scenario từ Firestore
            executeScenario(sceneType.c_str());
            
            // Reset về NONE
            Firebase.RTDB.setString(&streamData, "/scene_control/type", "NONE");
            Firebase.RTDB.setString(&streamData, "/scene_control/status", "DONE");
        }
    }
}

void streamTimeoutCallback(bool timeout) {
    if (timeout) {
        Serial.println("⚠️ [STREAM] Timeout, đang kết nối lại...");
    }
}

// ===== HÀM CÔNG KHAI =====

void initFirebaseListener() {
    Serial.println("🎧 [LISTENER] Khởi tạo Firebase Stream...");

    if (!isFirebaseReady()) {
        Serial.println("❌ [LISTENER] Firebase chưa sẵn sàng");
        return;
    }

    // Bắt đầu stream tại root để lắng nghe tất cả thay đổi
    if (!Firebase.RTDB.beginStream(&streamData, "/")) {
        Serial.printf("❌ [LISTENER] Không thể bắt đầu stream: %s\n", 
                      streamData.errorReason().c_str());
        return;
    }

    Firebase.RTDB.setStreamCallback(&streamData, streamCallback, streamTimeoutCallback);
    listener_ready = true;
    Serial.println("✅ [LISTENER] Firebase Stream đã sẵn sàng");
}

void handleFirebaseStream() {
    if (!listener_ready) return;
    
    // Firebase client tự động xử lý stream trong background
    // Không cần gọi gì thêm
}

#endif // ARDUINO
