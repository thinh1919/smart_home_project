#ifndef FIREBASE_LISTENER_H
#define FIREBASE_LISTENER_H

#ifdef ARDUINO
#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <data_struct.h>

// Khởi tạo Firebase Stream Listener
void initFirebaseListener();

// Xử lý stream trong loop
void handleFirebaseStream();

// Callback khi có thay đổi trên Firebase
void onFirebaseCommandChanged(const char* path, FirebaseData* data);

#endif // ARDUINO
#endif // FIREBASE_LISTENER_H
