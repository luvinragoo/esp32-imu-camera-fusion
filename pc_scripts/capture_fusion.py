import serial
import time
import csv
from pathlib import Path

# Configuration
PORT = "COM13"
BAUD = 921600

# Create output folder
script_dir = Path(__file__).parent  # pc_scripts/
project_root = script_dir.parent     # esp32-imu-camera-fusion/
output_dir  = project_root / "data" / "test_capture"
output_dir .mkdir(exist_ok=True, parents=True)

# CSV file for IMU data
imu_csv = output_dir / "imu_data.csv"

print(f"Connecting to {PORT} at {BAUD} baud...")
print(f"Saving IMU data to: {imu_csv}\n")

# Creating CSV file with header row
with open(imu_csv, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['timestamps_ms', 'ax_raw', 'ay_raw', 'az_raw', 'gx_raw', 'gy_raw', 'gz_raw'])

# Open serial connection
ser = serial.Serial(PORT,BAUD, timeout=2)
time.sleep(2) 

print("Connected! Reading data...")
print("Press Ctrl+C to stop\n")

imu_count = 0 # Counter for IMU samples

try:

    while True:
        # Read one line from serial
        line = ser.readline().decode('utf-8', errors='ignore').strip()

        if not line:
            continue

        # Check if it's an IMU line
        if line.startswith("IMU,"):
            # Split by comma: IMU,timestamp,ax,ay,az,gx,gy,gz
            parts = line.split(',')

            # Extract the values (skip "IMU" at index 0)
            timestamp = parts[1]
            ax = parts[2]
            ay = parts[3]
            az = parts[4]
            gx = parts[5]
            gy = parts[6]
            gz = parts[7]

            # Save to csv
            with open(imu_csv, 'a', newline='') as f:
                writer = csv.writer(f)
                writer.writerow([timestamp, ax, ay, az, gx, gy, gz])

            imu_count += 1

            # Print progress every 10 samples
            if imu_count % 10 == 0:
                print(f"IMU samples saved: {imu_count}")

        else:
            print(f"[Other] {line}")

except KeyboardInterrupt:
    print("\nStopped! Saved {imu_count} IMU samples to {imu_csv}")
    ser.close()
