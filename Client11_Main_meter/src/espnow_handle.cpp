#ifndef ESPNOW_HANDLE_H
#define ESPNOW_HANDLE_H

#include <Arduino.h>
#include <WiFi.h>
#include "esp_now_config.h"
#include "meter_handle.h" 
#include "esp_now.h"
#include "data_struct.h"
#include "espnow_handle.h"

void initESPNow();
void sendTotalPowerToGateway(MeterReadings readings);

#endif


// callback gửi
/*
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // In log debug gọn
     Serial.print(status == ESP_NOW_SEND_SUCCESS ? "." : "!");
}
*/
void initESPNow() {
    WiFi.mode(WIFI_STA);
    Serial.print(" Client 11 MAC: ");
    Serial.println(WiFi.macAddress());
/*
    if (esp_now_init() != ESP_OK) {
        Serial.println(" Init ESP-NOW Fail");
        ESP.restart();
    }
    esp_now_register_send_cb(OnDataSent);

    // Thêm Gateway vào Peer
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, gatewayAddress, 6);
    peer.channel = WIFI_CHANNEL;
    peer.encrypt = false;
    
    if (esp_now_add_peer(&peer) != ESP_OK) {
        Serial.println(" Lỗi thêm Gateway Peer");
    }
*/
}
/*
void sendTotalPowerToGateway(MeterReadings readings) {
    MainMeterPacket packet;
    
    // Header
    packet.header = createPacketHeader(CLIENT_ID_MAIN_METER, MSG_TYPE_SENSOR_DATA, sizeof(MainMeterData));
    
    // Payload
    packet.data.voltage = readings.voltage_V;
    packet.data.current = readings.current_A;
    packet.data.power   = readings.power_W; // Watt
    
    // Checksum
    packet.checksum = calculatePacketChecksum(packet);
    
    // Gửi đi
    esp_now_send(gatewayAddress, (uint8_t*)&packet, sizeof(packet));
}*/