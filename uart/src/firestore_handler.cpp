#ifdef ARDUINO

#include "firestore_handler.h"
#include "firebase_handler.h"
#include "firebase_listener.h"
#include <FirebaseJson.h>

// ===== BIẾN TOÀN CỤC =====
extern FirebaseData fbdo;
static bool firestore_ready = false;

// ===== KHỞI TẠO =====

void initFirestoreHandler() {
    Serial.println("🗄️  [FIRESTORE] Khởi tạo Firestore Handler...");
    
    if (!isFirebaseReady()) {
        Serial.println("❌ [FIRESTORE] Firebase chưa sẵn sàng");
        return;
    }
    
    firestore_ready = true;
    Serial.println("✅ [FIRESTORE] Firestore Handler sẵn sàng");
    Serial.printf("   Project ID: %s\n", FIREBASE_PROJECT_ID);
}

// ===== EXECUTE SCENARIO =====

void executeScenario(const char* sceneId) {
    if (!firestore_ready) {
        Serial.println("❌ [SCENARIO] Firestore chưa sẵn sàng");
        return;
    }
    
    Serial.printf("🎬 [SCENARIO] Bắt đầu thực thi: %s\n", sceneId);
    
    // Đọc document từ Firestore: scenario/{sceneId}
    String docPath = String("scenario/") + sceneId;
    
    if (!Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", docPath.c_str())) {
        Serial.printf("❌ [SCENARIO] Không đọc được: %s\n", fbdo.errorReason().c_str());
        return;
    }
    
    // Parse JSON response
    FirebaseJson json;
    json.setJsonData(fbdo.payload());
    
    // Lấy mảng actions
    FirebaseJsonData actionsData;
    json.get(actionsData, "fields/actions/arrayValue/values");
    
    if (!actionsData.success) {
        Serial.println("❌ [SCENARIO] Không tìm thấy actions");
        return;
    }
    
    // Parse mảng actions
    FirebaseJsonArray actionsArray;
    actionsArray.setJsonArrayData(actionsData.stringValue);
    
    Serial.printf("📋 [SCENARIO] Tìm thấy %d actions\n", actionsArray.size());
    
    // Thực thi từng action
    for (size_t i = 0; i < actionsArray.size(); i++) {
        FirebaseJsonData actionItem;
        actionsArray.get(actionItem, i);
        
        if (!actionItem.success) continue;
        
        // Parse từng action object
        FirebaseJson actionJson;
        actionJson.setJsonData(actionItem.stringValue);
        
        // Lấy các fields: device_path, field, value
        FirebaseJsonData devicePathData, fieldData, valueData;
        
        actionJson.get(devicePathData, "mapValue/fields/device_path/stringValue");
        actionJson.get(fieldData, "mapValue/fields/field/stringValue");
        
        // Value có thể là integer hoặc string
        bool hasIntValue = actionJson.get(valueData, "mapValue/fields/value/integerValue");
        if (!hasIntValue) {
            actionJson.get(valueData, "mapValue/fields/value/stringValue");
        }
        
        if (!devicePathData.success || !fieldData.success || !valueData.success) {
            Serial.printf("⚠️  [SCENARIO] Action %d: Thiếu thông tin\n", i + 1);
            continue;
        }
        
        // Tạo đường dẫn RTDB
        String rtdbPath = "/" + devicePathData.stringValue + "/" + fieldData.stringValue;
        
        // Ghi lệnh vào RTDB (Firebase Stream sẽ bắt và gửi xuống)
        bool success = false;
        
        if (hasIntValue) {
            int intValue = valueData.intValue;
            success = Firebase.RTDB.setInt(&fbdo, rtdbPath.c_str(), intValue);
            Serial.printf("   %d. %s = %d ", i + 1, rtdbPath.c_str(), intValue);
        } else {
            String strValue = valueData.stringValue;
            success = Firebase.RTDB.setString(&fbdo, rtdbPath.c_str(), strValue.c_str());
            Serial.printf("   %d. %s = %s ", i + 1, rtdbPath.c_str(), strValue.c_str());
        }
        
        if (success) {
            Serial.println("✅");
        } else {
            Serial.printf("❌ (%s)\n", fbdo.errorReason().c_str());
        }
        
        // Delay nhỏ giữa các lệnh để tránh quá tải
        delay(200);
    }
    
    Serial.printf("✅ [SCENARIO] Hoàn thành thực thi: %s\n", sceneId);
}

// ===== ENERGY DATA (TODO) =====

void updateEnergyData(float kwh, const char* deviceName) {
    // TODO: Implement energy tracking
    Serial.printf("⚡ [ENERGY] %s: %.2f kWh (chưa implement)\n", deviceName, kwh);
}

#endif // ARDUINO
