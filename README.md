# ESP32 IMU + Camera Fusion

Multi-sensor data logging and fusion system combining MPU-6050 IMU with ESP32-CAM for timestamped sensor streams and motion estimation.

## Hardware
- ESP32-WROVER development board
- MPU-6050 6-axis IMU (accelerometer + gyroscope) via I2C
- ESP32-CAM module

## Project Structure
- `esp32_firmware/` - MicroPython code for ESP32
- `pc_scripts/` - Python scripts for PC-side data capture and visualization
- `notebooks/` - Jupyter notebooks for sensor fusion analysis
- `data/` - Sample data logs (not committed)

## Status
🚧 Work in progress (February 14, 2026)

- [x] MPU-6050 streaming at ~18Hz with timestamps
- [ ] Camera frame capture with sync
- [ ] Python fusion notebook
- [ ] Demo recording

## Quick Start
_Coming soon_
