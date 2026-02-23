#include "wifi_udp.h"
#include <Arduino.h>

// ============================================
// WiFi + UDP Configuration
// ============================================
static const char* WIFI_SSID     = "YOUR_WIFI_NAME";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
static const char* UDP_HOST      = "YOUR_PC_IP";
static const uint16_t UDP_PORT   = 5005;

// ============================================
// Module-private objects
// ============================================
static WiFiUDP udp;                    // static = private to this file
QueueHandle_t imuQueue = NULL;         // extern in header, defined here

extern SemaphoreHandle_t serialMutex; // defined in main .ino

// ============================================
// Public Functions
// ============================================
void wifiUdpInit() {
    imuQueue = xQueueCreate(20, sizeof(IMUPacket));
}


// ============================================
// wifiTask()
// FreeRTOS task — runs forever on Core 0
// Phase 1: connect to WiFi
// Phase 2: loop — receive packets from queue, send over UDP
// ============================================
void wifiTask(void* parameters) {

  // Phase 1 : connect to WIFI
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    if (xSemaphoreTake(serialMutex, portMAX_DELAY)) {
        Serial.print("[WiFi] Connecting");
        xSemaphoreGive(serialMutex);
    }

    // Poll every 500ms until connected
    // vTaskDelay() yields to scheduler — other tasks run while we wait
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if (xSemaphoreTake(serialMutex, portMAX_DELAY)) {
            Serial.print(".");
            xSemaphoreGive(serialMutex);
        }
    }

    // WiFi connected — print assigned IP address
    if (xSemaphoreTake(serialMutex, portMAX_DELAY)) {
        Serial.print("\n[WiFi] Connected! IP: ");
        Serial.println(WiFi.localIP());
        xSemaphoreGive(serialMutex);
    }

// --- Phase 2: Receive from queue, send over UDP ---
    IMUPacket packet;        // reusable struct — filled by xQueueReceive
    char udpBuffer[128];     // reusable string buffer for formatted output

    while (true) {
        // Block here until imuTask puts a packet in the queue
        // portMAX_DELAY = wait forever — zero CPU used while waiting
        // When a packet arrives, xQueueReceive copies it into 'packet'
        // and returns true
        if (xQueueReceive(imuQueue, &packet, portMAX_DELAY)) {

          // Format packet as CSV string
            snprintf(udpBuffer, sizeof(udpBuffer),
                "IMU,%lu,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
                packet.timestamp_ms,
                packet.ax, packet.ay, packet.az,
                packet.gx, packet.gy, packet.gz);

            // Three-step UDP transmission:
            // 1. beginPacket — open a packet addressed to host:port
            // 2. print       — write the CSV string into the packet
            // 3. endPacket   — seal and fire the packet over WiFi
            udp.beginPacket(UDP_HOST, UDP_PORT);
            udp.print(udpBuffer);
            udp.endPacket();
        }
    }
    vTaskDelete(NULL);
}
