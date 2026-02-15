#include <MPU6050_tockn.h>
#include <Wire.h>

// ============================================
// MPU6050 IMU Configuration
// ============================================
// Custom I2C pins (separate from camera I2C bus)
#define MPU_SDA 13
#define MPU_SCL 14

// MPU6050 sensor object
MPU6050 mpu6050(Wire);

void setup() {
  // Initialize serial communication at 115200 baud
  Serial.begin(115200);
  
  // Initialize I2C bus on custom pins (GPIO 13=SDA, 14=SCL)
  // Note: Camera uses GPIO 26/27 for its own I2C bus
  Wire.begin(MPU_SDA, MPU_SCL);
  
  // Initialize MPU6050 sensor
  mpu6050.begin();
  
  // Calibrate gyroscope offsets (sensor must be stationary)
  // This removes drift/bias from gyroscope readings
  mpu6050.calcGyroOffsets(true);
  
  Serial.println("MPU6050 Ready");
  delay(100);
}

void loop() {
  // Update sensor readings from MPU6050
  mpu6050.update();
  
  // Get current timestamp in milliseconds since boot
  unsigned long t_ms = millis();
  
  // Read raw 16-bit sensor values
  // Accelerometer: ±2g range → raw values ±16384
  int16_t ax = mpu6050.getRawAccX();  // X-axis acceleration
  int16_t ay = mpu6050.getRawAccY();  // Y-axis acceleration
  int16_t az = mpu6050.getRawAccZ();  // Z-axis acceleration
  
  // Gyroscope: ±250°/s range → raw values ±32768
  int16_t gx = mpu6050.getRawGyroX();  // X-axis rotation rate
  int16_t gy = mpu6050.getRawGyroY();  // Y-axis rotation rate
  int16_t gz = mpu6050.getRawGyroZ();  // Z-axis rotation rate
  
  // Output format: timestamp|ax,ay,az|gx,gy,gz
  // Raw values are converted to physical units in Python
  Serial.print(t_ms);
  Serial.print("|");
  Serial.print(ax); Serial.print(",");
  Serial.print(ay); Serial.print(",");
  Serial.print(az);
  Serial.print("|");
  Serial.print(gx); Serial.print(",");
  Serial.print(gy); Serial.print(",");
  Serial.println(gz);
  
  // Sample at 100Hz (10ms delay)
  // Adjust for different rates: 5ms=200Hz, 20ms=50Hz, 50ms=20Hz
  delay(10);
}

// ============================================
// Conversion formulas (apply in Python):
// ============================================
// Accelerometer (raw → g):   value / 16384.0
// Gyroscope (raw → °/s):     value / 131.0
// ============================================
