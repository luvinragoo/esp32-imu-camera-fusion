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

// ============================================
// FreeRTOS handles
// ============================================
TaskHandle_t imuTaskHandle = NULL;
TaskHandle_t cameraTaskHandle = NULL;
SemaphoreHandle_t serialMutex = NULL;

// ============================================
// Forward declarations of functions
// ============================================
void initIMU();
void readIMU(unsigned long timestamp);
void initCamera();
void captureFrame(unsigned long timestamp);

// ─────────────────────────────────────────────────────────────
// IMU TASK — Core 0
// ─────────────────────────────────────────────────────────────
void imuTask(void *parameters){
  initIMU();
  delay(500);

  while (true) {
    unsigned long now = millis();
    readIMU(now);

    vTaskDelay(IMU_INTERVAL_MS / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}

// ─────────────────────────────────────────────────────────────
// Camera TASK — Core 1
// ─────────────────────────────────────────────────────────────
void cameraTask(void *parameters){
  delay(3000);
  initCamera();
  delay(500);

  while (true){
    unsigned long now = millis();
    captureFrame(now);

    vTaskDelay(CAMERA_INTERVAL_MS / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}


void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.setDebugOutput(false);
  delay(5000);  // give you time to open Serial Monitor
  
  serialMutex = xSemaphoreCreateMutex();

  Serial.println("INIT_START");

  xTaskCreatePinnedToCore(
    imuTask,
    "IMU-Task",
    4096,
    NULL,
    1,
    &imuTaskHandle,
    0
  );

  xTaskCreatePinnedToCore(
    cameraTask,
    "Camera-Task",
    8192,
    NULL,
    1,
    &cameraTaskHandle,
    1
  );

  if(xSemaphoreTake(serialMutex, portMAX_DELAY)) {
    Serial.println("TASKS_CREATED");
    xSemaphoreGive(serialMutex);
  }


}

void loop() {
// leave empty

}

// ============================================
// IMU Functions
// ============================================
void initIMU() {
  Wire.begin(MPU_SDA, MPU_SCL);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(false);

  if(xSemaphoreTake(serialMutex, portMAX_DELAY)){
    Serial.println("IMU_READY");

    xSemaphoreGive(serialMutex);
  }
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
  if(xSemaphoreTake(serialMutex, portMAX_DELAY)){
    Serial.print("IMU,");
    Serial.print(timestamp);
    Serial.print(",");
    Serial.print(ax); Serial.print(",");
    Serial.print(ay); Serial.print(",");
    Serial.print(az); Serial.print(",");
    Serial.print(gx); Serial.print(",");
    Serial.print(gy); Serial.print(",");
    Serial.println(gz);

    xSemaphoreGive(serialMutex);
  }
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
    config.jpeg_quality = 5;             // 0-63, lower = higher quality
    config.fb_count = 2;                  // Use 2 frame buffers with PSRAM
  } else {
    config.frame_size = FRAMESIZE_VGA;   // 640x480
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    if(xSemaphoreTake(serialMutex, portMAX_DELAY)) {
      Serial.printf("CAMERA_FAIL,0x%x\n", err);
      xSemaphoreGive(serialMutex);
    }
    return;
  }

  if(xSemaphoreTake(serialMutex, portMAX_DELAY)) {
    Serial.println("CAMERA_READY");
    xSemaphoreGive(serialMutex);
  }
}

void captureFrame(unsigned long timestamp) {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    if(xSemaphoreTake(serialMutex, portMAX_DELAY)) {
      Serial.println("FRAME_ERROR");
      xSemaphoreGive(serialMutex);
    }

    return;
  }
  
  if(xSemaphoreTake(serialMutex, portMAX_DELAY)) {
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

    xSemaphoreGive(serialMutex);
  }
  
  // Return frame buffer to free memory
  esp_camera_fb_return(fb);
}




