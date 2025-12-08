#include "firebase_handler.h"
#include "firestore_handler.h"

#ifdef ARDUINO

// ===== BIẾN TOÀN CỤC =====
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool firebaseReady = false;
unsigned long sendDataPrevMillis = 0;

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
 */
void uploadToFirebase(uint8_t* payload, int len) {
    if (!isFirebaseReady()) {
        Serial.println("✗ Firebase chưa sẵn sàng, bỏ qua upload");
        return;
    }
    
    // Kiểm tra độ dài tối thiểu (phải có ít nhất header)
    if (len < sizeof(ESPNowPacketHeader)) {
        Serial.println("✗ Payload quá ngắn, không có header");
        return;
    }
    
    // Cast payload thành header để lấy client_id và msg_type
    ESPNowPacketHeader* header = (ESPNowPacketHeader*)payload;
    uint8_t client_id = header->client_id;
    uint8_t msg_type = header->msg_type;
    
    Serial.printf("\n📤 Upload Firebase: Client=%d, Type=%d\n", client_id, msg_type);
    
    // Con trỏ tới phần data (sau header)
    uint8_t* dataPtr = payload + sizeof(ESPNowPacketHeader);
    size_t dataLen = len - sizeof(ESPNowPacketHeader) - sizeof(uint16_t); // Trừ header và checksum
    
    String path = "";
    bool success = false;
    
    // Switch case theo client_id
    switch (client_id) {
        case CLIENT_ID_DOOR: {
            // Client 1: Cửa phòng khách
            if (dataLen >= sizeof(DoorData)) {
                DoorData* data = (DoorData*)dataPtr;
                path = "/living_room/door";
                
                Serial.printf("  🚪 Cửa: %s\n", data->is_open ? "MỞ" : "ĐÓNG");
                
                // Upload is_open
                success = Firebase.RTDB.setBool(&fbdo, path + "/is_open", data->is_open);
                if (success) {
                    Serial.println("  ✓ Upload door status thành công");
                } else {
                    Serial.printf("  ✗ Lỗi: %s\n", fbdo.errorReason().c_str());
                }
            }
            break;
        }
        
        case CLIENT_ID_LIGHT: {
            // Client 2: Đèn phòng khách
            if (dataLen >= sizeof(LightData)) {
                LightData* data = (LightData*)dataPtr;
                path = "/living_room/light";
                
                Serial.printf("  💡 Đèn: Mode=%d\n", data->mode);
                
                success = Firebase.RTDB.setInt(&fbdo, path + "/mode", data->mode);
                if (success) {
                    Serial.println("  ✓ Upload light mode thành công");
                } else {
                    Serial.printf("  ✗ Lỗi: %s\n", fbdo.errorReason().c_str());
                }
            }
            break;
        }
        
        case CLIENT_ID_ENV_LIVING: {
            // Client 4: Cảm biến môi trường phòng khách
            if (dataLen >= sizeof(EnvSensorData)) {
                EnvSensorData* data = (EnvSensorData*)dataPtr;
                path = "/living_room/env";
                
                Serial.printf("  🌡️  Chất lượng không khí: %d PPM\n", data->air_quality);
                
                success = Firebase.RTDB.setInt(&fbdo, path + "/air_quality", data->air_quality);
                if (success) {
                    Serial.println("  ✓ Upload air quality thành công");
                } else {
                    Serial.printf("  ✗ Lỗi: %s\n", fbdo.errorReason().c_str());
                }
            }
            break;
        }
        
        case CLIENT_ID_CURTAIN: {
            // Client 7: Rèm cửa phòng ngủ
            if (dataLen >= sizeof(CurtainData)) {
                CurtainData* data = (CurtainData*)dataPtr;
                path = "/bedroom/curtain";
                
                Serial.printf("  🪟 Rèm: %d%%\n", data->position);
                
                success = Firebase.RTDB.setInt(&fbdo, path + "/position", data->position);
                if (success) {
                    Serial.println("  ✓ Upload curtain position thành công");
                } else {
                    Serial.printf("  ✗ Lỗi: %s\n", fbdo.errorReason().c_str());
                }
            }
            break;
        }
        
        case CLIENT_ID_ENV_BEDROOM: {
            // Client 8: Cảm biến môi trường phòng ngủ
            if (dataLen >= sizeof(BedroomEnvData)) {
                BedroomEnvData* data = (BedroomEnvData*)dataPtr;
                path = "/bedroom/env";
                
                Serial.printf("  🌡️  Nhiệt độ: %.1f°C, Độ ẩm: %.1f%%\n", 
                             data->temp, data->hum);
                
                // Upload temperature
                success = Firebase.RTDB.setFloat(&fbdo, path + "/temperature", data->temp);
                if (success) {
                    // Upload humidity
                    success = Firebase.RTDB.setFloat(&fbdo, path + "/humidity", data->hum);
                }
                
                if (success) {
                    Serial.println("  ✓ Upload temp & humidity thành công");
                } else {
                    Serial.printf("  ✗ Lỗi: %s\n", fbdo.errorReason().c_str());
                }
            }
            break;
        }
        
        case CLIENT_ID_FAN: {
            // Client 9: Quạt phòng ngủ
            if (dataLen >= sizeof(FanData)) {
                FanData* data = (FanData*)dataPtr;
                path = "/bedroom/fan";
                
                Serial.printf("  🌀 Quạt: Mode=%d\n", data->mode);
                
                success = Firebase.RTDB.setInt(&fbdo, path + "/mode", data->mode);
                if (success) {
                    Serial.println("  ✓ Upload fan mode thành công");
                } else {
                    Serial.printf("  ✗ Lỗi: %s\n", fbdo.errorReason().c_str());
                }
            }
            break;
        }
        
        case CLIENT_ID_PURIFIER: {
            // Client 10: Máy lọc không khí
            if (dataLen >= sizeof(PurifierData)) {
                PurifierData* data = (PurifierData*)dataPtr;
                path = "/living_room/purifier";
                
                Serial.printf("  💨 Máy lọc: %s\n", data->state ? "BẬT" : "TẮT");
                
                success = Firebase.RTDB.setBool(&fbdo, path + "/state", data->state);
                if (success) {
                    Serial.println("  ✓ Upload purifier state thành công");
                } else {
                    Serial.printf("  ✗ Lỗi: %s\n", fbdo.errorReason().c_str());
                }
            }
            break;
        }
        
        default:
            Serial.printf("  ⚠️  Client ID không xác định: %d\n", client_id);
            break;
    }
    
    Serial.println();
}

#endif // ARDUINO
