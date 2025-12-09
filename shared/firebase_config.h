// firmware/shared/firebase_config.h
#ifndef FIREBASE_CONFIG_H
#define FIREBASE_CONFIG_H

// 1. WiFi Credentials
#define WIFI_SSID "Ten_Wifi_Nha_Ban"
#define WIFI_PASSWORD "Mat_Khau_Wifi"

// 2. Firebase Project Info
// Lấy trong Project Settings -> General -> Web App
#define API_KEY "cEjzpyZD1DPsxk0MY58SYtN7nRQv4bww0k6BK73v"

// Lấy trong Realtime Database -> Data (Copy link có https)
#define DATABASE_URL "https://smarthome-41aff-default-rtdb.asia-southeast1.firebasedatabase.app/"


// 3. User Authentication (Nên dùng Email/Pass cho thiết bị IoT cố định)
// Vào Authentication -> Users -> Add user (ví dụ: device@smarthome.com)
#define USER_EMAIL "hoaianhngu2003@gmail.com"
#define USER_PASSWORD ""

// 4. Token Refresh (Quan trọng để chạy lâu dài)
// Thư viện sẽ tự xử lý, nhưng cần khai báo để nó biết
// Include này sẽ được thêm vào trong file main.cpp của project sử dụng Firebase
// #ifdef ARDUINO
// #include <addons/TokenHelper.h>
// #endif

#endif