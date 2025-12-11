#include "firebase_handler.h"
#include "firestore_handler.h"

#ifdef ARDUINO

// ===== BIẾN TOÀN CỤC =====
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool firebaseReady = false;
unsigned long sendDataPrevMillis = 0;

// Buffer để lưu trạng thái hiện tại của toàn bộ nhà
HomeStatus currentHomeState = {0};

/**
 * Khởi tạo Firebase connection
 */
void initFirebase() {
    Serial.println("========================================");
    Serial.println("Đang kết nối WiFi...");
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.println("WiFi đã kết nối!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    // Cấu hình Firebase
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    
    // Đăng nhập với email và password
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    
    // Gán callback cho token generation
    config.token_status_callback = tokenStatusCallback;
    
    // Khởi động Firebase
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    
    // Thiết lập buffer size (tùy chọn)
    fbdo.setResponseSize(4096);
    
    Serial.println("Firebase đã được khởi tạo!");
    Serial.println("========================================");
}

/**
 * Xử lý Firebase tasks
 */
void handleFirebase() {
    // Firebase tự động xử lý token refresh
    // Chỉ cần kiểm tra trạng thái ready
    if (Firebase.ready()) {
        if (!firebaseReady) {
            firebaseReady = true;
            Serial.println("✓ Firebase sẵn sàng!");
        }
    } else {
        if (firebaseReady) {
            firebaseReady = false;
            Serial.println("✗ Firebase mất kết nối!");
        }
    }
}

/**
 * Kiểm tra trạng thái Firebase
 */
bool isFirebaseReady() {
    return Firebase.ready();
}

/**
 * Upload dữ liệu lên Firebase theo client_id
 * (Đã được tối ưu: chỉ cập nhật buffer, không upload ngay)
 */
void uploadToFirebase(uint8_t* payload, int len) {
    // Kiểm tra độ dài tối thiểu (phải có ít nhất header)
    if (len < sizeof(ESPNowPacketHeader)) {
        Serial.println("✗ Payload quá ngắn, không có header");
        return;
    }
    
    // Cast payload thành header để lấy client_id và msg_type
    ESPNowPacketHeader* header = (ESPNowPacketHeader*)payload;
    uint8_t client_id = header->client_id;
    uint8_t msg_type = header->msg_type;
    
    Serial.printf("\n📦 Buffer Update: Client=%d, Type=%d\n", client_id, msg_type);
    
    // Con trỏ tới phần data (sau header)
    uint8_t* dataPtr = payload + sizeof(ESPNowPacketHeader);
    size_t dataLen = len - sizeof(ESPNowPacketHeader) - sizeof(uint16_t); // Trừ header và checksum
    
    // Cập nhật buffer currentHomeState theo client_id
    switch (client_id) {
        case CLIENT_ID_DOOR: {
            // Client 1: Cửa phòng khách
            if (dataLen >= sizeof(DoorData)) {
                DoorData* data = (DoorData*)dataPtr;
                currentHomeState.living_room_door.is_open = data->is_open;
                currentHomeState.living_room_door.command = data->command;
                Serial.printf("  🚪 Cửa: %s (buffered)\n", data->is_open ? "MỞ" : "ĐÓNG");
            }
            break;
        }
        
        case CLIENT_ID_LIGHT: {
            // Client 2: Đèn phòng khách
            if (dataLen >= sizeof(LightData)) {
                LightData* data = (LightData*)dataPtr;
                currentHomeState.living_room_light.mode = data->mode;
                currentHomeState.living_room_light.command = data->command;
                Serial.printf("  💡 Đèn: Mode=%d (buffered)\n", data->mode);
            }
            break;
        }
        
        case CLIENT_ID_ENV_LIVING: {
            // Client 4: Cảm biến môi trường phòng khách
            if (dataLen >= sizeof(EnvSensorData)) {
                EnvSensorData* data = (EnvSensorData*)dataPtr;
                currentHomeState.living_room_env.mq135 = data->mq135;                        // ✅
                currentHomeState.living_room_env.air_quality_status = data->air_quality_status; // ✅
                currentHomeState.living_room_env.lux = data->lux;                            // ✅
                Serial.printf("  🌡️  AQI: %d, Status: %d, Lux: %.1f (buffered)\n", 
                     data->mq135, data->air_quality_status, data->lux);
            }
            break;
        }
        
        case CLIENT_ID_CURTAIN: {
            // Client 7: Rèm cửa phòng ngủ
            if (dataLen >= sizeof(CurtainData)) {
                CurtainData* data = (CurtainData*)dataPtr;
                currentHomeState.bedroom_curtain.position = data->curtainPosition;
                currentHomeState.bedroom_curtain.target_pos = data->curtainPercent;
                currentHomeState.bedroom_curtain.state= data->isOpen;
                currentHomeState.bedroom_curtain.mode = data->manualMode;
                Serial.printf("  🪟 Rèm: %d%% (buffered)\n", data->curtainPosition);
            }
            break;
        }
        
        case CLIENT_ID_ENV_BEDROOM: {
            // Client 8: Cảm biến môi trường phòng ngủ
            if (dataLen >= sizeof(BedroomEnvData)) {
                BedroomEnvData* data = (BedroomEnvData*)dataPtr;
                currentHomeState.bedroom_env.temp = data->temp;
                currentHomeState.bedroom_env.hum = data->hum;
                currentHomeState.bedroom_env.lux = data->lux;
                Serial.printf("  🌡️  Nhiệt độ: %.1f°C, Độ ẩm: %.1f%%, Độ sáng: %.1f lux (buffered)\n", 
                             data->temp, data->hum, data->lux);
            }
            break;
        }
        
        case CLIENT_ID_FAN: {
            // Client 9: Quạt phòng ngủ
            if (dataLen >= sizeof(FanData)) {
                FanData* data = (FanData*)dataPtr;
                currentHomeState.bedroom_fan.mode = data->mode;
                currentHomeState.bedroom_fan.command = data->command;
                Serial.printf("  🌀 Quạt: Mode=%d (buffered)\n", data->mode);
            }
            break;
        }
        
        case CLIENT_ID_PURIFIER: {
            // Client 10: Máy lọc không khí
            if (dataLen >= sizeof(PurifierData)) {
                PurifierData* data = (PurifierData*)dataPtr;
                currentHomeState.living_room_purifier.state = data->state;
                currentHomeState.living_room_purifier.command = data->command;
                Serial.printf("  💨 Máy lọc: %s (buffered)\n", data->state ? "BẬT" : "TẮT");
            }
            break;
        }
        
        default:
            Serial.printf("  ⚠️  Client ID không xác định: %d\n", client_id);
            break;
    }
    
    // Cập nhật timestamp
    currentHomeState.last_updated = millis();
    Serial.println();
}

/**
 * Đồng bộ toàn bộ dữ liệu buffer lên Firebase trong một lần gọi
 * Gọi hàm này theo định kỳ (ví dụ: mỗi 60 giây)
 */
void syncDataToFirebase() {
    if (!isFirebaseReady()) {
        Serial.println("✗ Firebase chưa sẵn sàng, bỏ qua sync");
        return;
    }
    
    Serial.println("\n🔄 Bắt đầu đồng bộ dữ liệu lên Firebase...");
    
    // Tạo FirebaseJson object để chứa toàn bộ dữ liệu
    FirebaseJson json;
    
    // ===== PHÒNG KHÁCH =====
    // Door
    json.set("living_room/door/is_open", currentHomeState.living_room_door.is_open);
    json.set("living_room/door/command", (int)currentHomeState.living_room_door.command);
    
    // Light
    json.set("living_room/light/mode", (int)currentHomeState.living_room_light.mode);
    json.set("living_room/light/command", (int)currentHomeState.living_room_light.command);
    
    // Purifier
    json.set("living_room/purifier/state", currentHomeState.living_room_purifier.state);
    json.set("living_room/purifier/command", (int)currentHomeState.living_room_purifier.command);
    
    // Environment sensor
    json.set("living_room/env/air_quality", (int)currentHomeState.living_room_env.air_quality);
    
    // ===== PHÒNG NGỦ =====
    // Curtain
    json.set("bedroom/curtain/position", (int)currentHomeState.bedroom_curtain.position);
    json.set("bedroom/curtain/target_pos", (int)currentHomeState.bedroom_curtain.target_pos);
    
    // Fan
    json.set("bedroom/fan/mode", (int)currentHomeState.bedroom_fan.mode);
    json.set("bedroom/fan/command", (int)currentHomeState.bedroom_fan.command);
    
    // Environment sensor
    json.set("bedroom/env/temperature", currentHomeState.bedroom_env.temp);
    json.set("bedroom/env/humidity", currentHomeState.bedroom_env.hum);
    
    // ===== SCENE CONTROL =====
    json.set("scene_control/type", (int)currentHomeState.scene_control.type);
    json.set("scene_control/status", (int)currentHomeState.scene_control.status);
    
    // Gửi toàn bộ JSON trong một lần updateNode
    if (Firebase.RTDB.updateNode(&fbdo, "/", &json)) {
        Serial.println("✓ Đồng bộ dữ liệu thành công!");
        Serial.printf("  📊 Thời gian: %lu ms\n", millis() - currentHomeState.last_updated);
    } else {
        Serial.println("✗ Đồng bộ dữ liệu thất bại!");
        Serial.printf("  Lỗi: %s\n", fbdo.errorReason().c_str());
    }
    
    Serial.println();
}

#endif // ARDUINO
