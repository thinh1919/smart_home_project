#include "door_handle.h"
#include "espnow_handle.h" // Để gửi báo cáo trạng thái

// ===== KHAI BÁO KEYPAD & LCD =====
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {13, 32, 14, 27};
byte colPins[COLS] = {26, 25, 33};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== BIẾN LOGIC =====
String input = "";
const uint8_t PASS_LEN = 4;
char password[PASS_LEN + 1];
bool isDoorOpen = false;

// EEPROM
const int EEPROM_SIZE = 512;
const int EEPROM_ADDR_PASS = 0;

// ===== HÀM NỘI BỘ (Private) =====
void relayWrite(bool on) {
  if (RELAY_ACTIVE_LOW) digitalWrite(RELAY_PIN, on ? LOW : HIGH);
  else digitalWrite(RELAY_PIN, on ? HIGH : LOW);
}

void loadPassword() {
  EEPROM.begin(EEPROM_SIZE);
  bool empty = false;
  for (int i = 0; i < PASS_LEN; i++) {
    byte b = EEPROM.read(EEPROM_ADDR_PASS + i);
    if (b == 0xFF || b == 0) empty = true;
    password[i] = (empty) ? '1' + i : (char)b;
  }
  password[PASS_LEN] = '\0';
}

void savePassword(const char* p) {
  for (int i = 0; i < PASS_LEN; i++)
    EEPROM.write(EEPROM_ADDR_PASS + i, p[i]);
  EEPROM.commit();
}

void showMaskedInput() {
  lcd.setCursor(0, 1);
  for (uint8_t i = 0; i < input.length(); i++) lcd.print('*');
  for (uint8_t i = input.length(); i < PASS_LEN; i++) lcd.print(' ');
}

void changePassword() {
  lcd.clear(); lcd.print("New password:");
  String pass1 = "";
  while (pass1.length() < PASS_LEN) {
    char k = keypad.getKey();
    if (k && isdigit(k)) {
      pass1 += k;
      lcd.setCursor(pass1.length() - 1, 1); lcd.print('*');
    }
    // Giữ cho ESP-NOW vẫn nhận dữ liệu nền (nếu cần)
    delay(10); 
  }
  
  delay(500);
  lcd.clear(); lcd.print("Confirm again:");
  String pass2 = "";
  while (pass2.length() < PASS_LEN) {
    char k = keypad.getKey();
    if (k && isdigit(k)) {
      pass2 += k;
      lcd.setCursor(pass2.length() - 1, 1); lcd.print('*');
    }
    delay(10);
  }
  
  if (pass1 == pass2) {
    savePassword(pass1.c_str());
    strcpy(password, pass1.c_str());
    lcd.clear(); lcd.print("Pass changed!");
  } else {
    lcd.clear(); lcd.print("Not match!");
  }
  delay(1500);
  lockDoor();
}

// ===== HÀM PUBLIC (GIAO TIẾP RA NGOÀI) =====

void initDoorHardware() {
  lcd.init();
  lcd.backlight();
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BTN_INSIDE_PIN, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
  
  relayWrite(false); // Mặc định khóa
  loadPassword();
  
  lcd.clear();
  lcd.print("Enter password:");
  lcd.setCursor(0, 1);
  lcd.print("----");
}

void unlockDoor() {
  relayWrite(true);
  isDoorOpen = true;
  lcd.clear(); lcd.print("Door: OPENED");
  input = ""; // Reset input
  
  // Gửi báo cáo về Gateway
  sendDoorStatusToGateway();
}

void lockDoor() {
  relayWrite(false);
  isDoorOpen = false;
  lcd.clear(); lcd.print("Door: LOCKED");
  input = "";
  lcd.setCursor(0, 1); lcd.print("----"); // Reset màn hình nhập pass
  
  // Gửi báo cáo về Gateway
  sendDoorStatusToGateway();
}

void handleDoorLogic() {
  char key = keypad.getKey();
  
  static bool lastLimitState = (digitalRead(LIMIT_SWITCH_PIN) == LOW); 

  // 1. Nút mở bên trong
  if (digitalRead(BTN_INSIDE_PIN) == LOW) {
    unlockDoor();
    delay(300);
  }

  // 2. Xử lý công tắc hành trình (Tự động khóa khi đóng cửa)
  bool currentLimit = (digitalRead(LIMIT_SWITCH_PIN) == LOW);
  
  // Nếu cửa đang mở (Logic phần mềm) và công tắc hành trình vừa chuyển từ "nhả" -> "kích" (Cửa đóng thật)
  if (isDoorOpen && !lastLimitState && currentLimit) {
    lockDoor();
    delay(200);
  }
  lastLimitState = currentLimit;

  // 3. Xử lý Keypad '#'
  if (key == '#') {
    if (input == "1111") {
      changePassword();
      return;
    }
    
    if (input == String(password)) {
      unlockDoor();
    } else {
      lcd.clear(); lcd.print("Wrong password!");
      delay(1000);
      lockDoor(); // Reset về màn hình khóa
    }
    input = ""; // Reset sau khi check
  }
  
  // 4. Xử lý nhập số
  if (key && isdigit(key)) {
    if (input.length() < PASS_LEN) {
      input += key;
      showMaskedInput();
    }
  }
}