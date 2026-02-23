#ifndef WIFI_UDP_H
#define WIFI_UDP_H

#include <WiFi.h>
#include <WiFiUdp.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// ============================================
// IMU Packet Structure
// ============================================
struct IMUPacket {
    uint32_t timestamp_ms;
    float ax, ay, az;
    float gx, gy, gz;
};

// ============================================
// Queue handle — defined in wifi_udp.cpp
// accessible from main .ino via extern
// ============================================
extern QueueHandle_t imuQueue;

// ============================================
// Public interface
// ============================================
void wifiUdpInit();         // call in setup() — creates queue
void wifiTask(void* parameters);  // FreeRTOS task function

#endif // WIFI_UDP_H
