# ESP32 Firmware

Arduino C++ firmware for ESP32-WROVER with OV3660 camera and MPU6050 IMU.

## Arduino IDE Setup

1. Install ESP32 board support
2. Board: **ESP32 Wrover Module**
3. PSRAM: **Enabled**
4. Partition: **Huge APP (3MB No OTA)**

## Required Libraries

Install via **Tools → Manage Libraries**:

- **MPU6050_tockn** by tockn

That's it! The camera uses the built-in ESP32 camera driver.

## Sketches

- `camera_serial/` - OV3660 camera capture over serial
- `mpu6050_serial/` - MPU6050 IMU data @ 100Hz

## Hardware Connections

### Camera (OV3660)
- I2C: GPIO 26 (SDA), GPIO 27 (SCL)
- See camera_serial sketch for full pinout

### IMU (MPU6050)
- I2C: GPIO 13 (SDA), GPIO 14 (SCL)
- VCC: 3.3V, GND: GND
