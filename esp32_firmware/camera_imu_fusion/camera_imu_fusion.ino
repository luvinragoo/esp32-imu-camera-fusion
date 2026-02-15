// ============================================
// ESP32 Camera + IMU Fusion System
// ============================================
// Combines OV3660 camera and MPU6050 IMU
// - Camera: 5Hz (every 200ms), 640x480 VGA JPEG
// - IMU: 100Hz (every 10ms), raw accelerometer + gyroscope
// - Serial: 921600 baud for fast transfer
// - Unified timestamped serial protocol
// ============================================

#include "esp_camera.h"
#include <Wire.h>
#include <MPU6050_tockn.h>

// ============================================
// Camera Pin Definitions (Freenove ESP32-WROVER)
// ============================================
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26  // Camera I2C SDA
#define SIOC_GPIO_NUM    27  // Camera I2C SCL

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      19
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

// ============================================
// IMU Pin Definitions (MPU6050)
// ============================================
#define MPU_SDA 13  // Separate I2C bus from camera
#define MPU_SCL 14

// ============================================
// Serial and Timing Configuration
// ============================================
#define SERIAL_BAUD       921600  // High-speed serial for fast frame transfer
#define IMU_INTERVAL_MS   10      // 100Hz IMU sampling (20 samples between frames)
#define CAMERA_INTERVAL_MS 200    // 5Hz camera capture (5 fps)

// ============================================
// Global Objects
// ============================================
MPU6050 mpu6050(Wire);

unsigned long lastIMUTime = 0;
unsigned long lastCameraTime = 0;

// ============================================
// Setup
// ============================================
void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.setDebugOutput(false);  // Reduce debug noise
  
  // Initialize IMU first (faster startup)
  Serial.println("INIT_START");
  initIMU();
  
  // Initialize camera
  initCamera();
  
  Serial.println("INIT_COMPLETE");
  delay(100);
}

// ============================================
// Main Loop
// ============================================
void loop() {
  unsigned long now = millis();
  
  // Sample IMU at 100Hz (every 10ms)
  if (now - lastIMUTime >= IMU_INTERVAL_MS) {
    readIMU(now);
    lastIMUTime = now;
  }
  
  // Capture camera frame at 5Hz (every 200ms)
  if (now - lastCameraTime >= CAMERA_INTERVAL_MS) {
    captureFrame(now);
    lastCameraTime = now;
  }
}

// ============================================
// IMU Functions
// ============================================
void initIMU() {
  Wire.begin(MPU_SDA, MPU_SCL);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  Serial.println("IMU_READY");
}

void readIMU(unsigned long timestamp) {
  mpu6050.update();
  
  // Read raw 16-bit sensor values
  int16_t ax = mpu6050.getRawAccX();
  int16_t ay = mpu6050.getRawAccY();
  int16_t az = mpu6050.getRawAccZ();
  int16_t gx = mpu6050.getRawGyroX();
  int16_t gy = mpu6050.getRawGyroY();
  int16_t gz = mpu6050.getRawGyroZ();
  
  // Output format: IMU,timestamp,ax,ay,az,gx,gy,gz
  Serial.print("IMU,");
  Serial.print(timestamp);
  Serial.print(",");
  Serial.print(ax); Serial.print(",");
  Serial.print(ay); Serial.print(",");
  Serial.print(az); Serial.print(",");
  Serial.print(gx); Serial.print(",");
  Serial.print(gy); Serial.print(",");
  Serial.println(gz);
}

// ============================================
// Camera Functions
// ============================================
void initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // VGA resolution (640x480) for faster transfer
  // Smaller images = faster serial transmission at 5fps
  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;   // 640x480
    config.jpeg_quality = 10;             // 0-63, lower = higher quality
    config.fb_count = 2;                  // Use 2 frame buffers with PSRAM
  } else {
    config.frame_size = FRAMESIZE_VGA;   // 640x480
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("CAMERA_FAIL,0x%x\n", err);
    return;
  }
  
  Serial.println("CAMERA_READY");
}

void captureFrame(unsigned long timestamp) {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("FRAME_ERROR");
    return;
  }
  
  // Frame header: FRAME,timestamp,length
  Serial.print("FRAME,");
  Serial.print(timestamp);
  Serial.print(",");
  Serial.println(fb->len);
  
  // Send raw JPEG bytes
  Serial.write(fb->buf, fb->len);
  
  // Frame footer
  Serial.println();
  Serial.println("FRAME_END");
  
  // Return frame buffer to free memory
  esp_camera_fb_return(fb);
}

// ============================================
// Data Format Summary
// ============================================
// IMU Output: IMU,timestamp_ms,ax,ay,az,gx,gy,gz
//   - timestamp: milliseconds since boot
//   - ax,ay,az: raw accelerometer (±2g → ±16384)
//   - gx,gy,gz: raw gyroscope (±250°/s → ±32768)
//
// Frame Output:
//   FRAME,timestamp_ms,length
//   <binary JPEG data>
//   FRAME_END
//
// Conversion (apply in Python):
//   Acceleration (g): raw_value / 16384.0
//   Gyroscope (°/s): raw_value / 131.0
// ============================================
