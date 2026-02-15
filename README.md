# ESP32 IMU + Camera Fusion

Multi-sensor data acquisition system combining MPU6050 IMU and OV3660 camera on ESP32-WROVER.

## Current Status

✅ **Camera (OV3660)** - Working in Arduino C++
- 640x480 JPEG capture
- Serial streaming to PC
- Frame grabber script functional

🚧 **IMU (MPU6050)** - In progress
- MicroPython version working
- Porting to Arduino C++ next

## Hardware

- **MCU:** ESP32-WROVER (Freenove board)
- **Camera:** OV3660 (800x600 max)
- **IMU:** MPU6050 (accelerometer + gyroscope)

## Quick Start

### 1. Flash Camera Firmware
```bash
# Open esp32_firmware/camera_test.ino in Arduino IDE
# Board: ESP32 Wrover Module
# PSRAM: Enabled
# Upload to ESP32
``` 

### Next Steps
- [x] Port IMU code to Arduino C++

- [ ] Design unified serial protocol

- [ ] Synchronized timestamp logging

- [ ] Jupyter visualization notebook
