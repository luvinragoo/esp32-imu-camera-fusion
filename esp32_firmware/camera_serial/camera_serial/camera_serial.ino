#include "esp_camera.h"

// Freenove ESP32-WROVER-CAM Pin Definition
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    21
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

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

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("Camera Test - OV3660");

  // Camera configuration
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
  
  // Frame size and quality
  if(psramFound()){
    Serial.println("PSRAM found!");
    config.frame_size = FRAMESIZE_VGA; // 640x480
    config.jpeg_quality = 10;  // 0-63, lower is higher quality
    config.fb_count = 2;
  } else {
    Serial.println("No PSRAM!");
    config.frame_size = FRAMESIZE_VGA;  // 640x480
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  // Get camera sensor
  sensor_t * s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    Serial.println("OV3660 detected!");
  } else {
    Serial.printf("Camera PID: 0x%02X (expected OV3660)\n", s->id.PID);
  }

  Serial.println("Camera initialized successfully!");
}

void loop() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("ERR_NO_FRAME");
    delay(2000);
    return;
  }

  // Frame header: easy to detect on PC side
  Serial.println("FRAME_START");
  Serial.println(fb->len);          // send length as decimal, then newline

  // Send raw JPEG bytes
  Serial.write(fb->buf, fb->len);

  // Optional: end marker (helps sanity-check)
  Serial.println();
  Serial.println("FRAME_END");

  esp_camera_fb_return(fb);

  delay(3000);  // one frame every 3s for now
}

