import serial
import time
import csv
from pathlib import Path

# Configuration
PORT = "COM13"
BAUD = 921600

# Create output folders
script_dir = Path(__file__).parent  # pc_scripts/
project_root = script_dir.parent     # esp32-imu-camera-fusion/
output_dir  = project_root / "data" / "test_capture"
frames_dir = output_dir / "frames"
output_dir.mkdir(exist_ok=True, parents=True)
frames_dir.mkdir(exist_ok=True)

# CSV file for IMU data
imu_csv = output_dir / "imu_data.csv"

print(f"Connecting to {PORT} at {BAUD} baud...")
print(f"Saving IMU data to: {imu_csv}\n")
print(f"Saving frames to: {frames_dir}\n")

# Creating CSV file with header row
with open(imu_csv, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['timestamps_ms', 'ax_raw', 'ay_raw', 'az_raw', 'gx_raw', 'gy_raw', 'gz_raw'])

# Open serial connection
ser = serial.Serial(PORT,BAUD, timeout=2)
time.sleep(2) 

print("Connected! Capturing data...")
print("Press Ctrl+C to stop\n")

imu_count = 0 # Counter for IMU samples
frame_count = 0 # Counter for frames

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

            # Print progress every 50 samples
            if imu_count % 50 == 0:
                print(f"IMU: {imu_count} samples | Frames: {frame_count}")

        elif line.startswith("FRAME,"):
            #Parse: FRAME,timestamp,length
            parts = line.split(',')
            timestamp = int(parts[1])
            length = int(parts[2])

            print(f"\n Receiving frame {frame_count} (t={timestamp}ms, {length} bytes)...")

            # Read the binary JPEG data
            jpeg_data = ser.read(length)

            # Check if we got all the data
            if len(jpeg_data) == length:
                # Create filename with timestamp
                filename = f"frame_{frame_count:04d}_t{timestamp}.jpg"
                filepath = frames_dir / filename

                # Save as JPEG file (binary mode)
                with open(filepath, 'wb') as f:
                    f.write(jpeg_data)

                print(f"Saved {filename}")
                frame_count += 1

                # Read the footer lines (empty + FRAME_END)
                ser.readline() # Empty line
                ser.readline() # FRAME_END
            else:
                print(f"❌ Incomplete frame! Got {len(jpeg_data)} bytes, expected {length}")

        # Other messages
        elif line in ["INIT_START", "IMU_READY", "CAMERA_READY", "INIT_COMPLETE"]:
            print(f"🔧 {line}")

except KeyboardInterrupt:
    print(f"\n\n{'='*50}")
    print(f"Stopped!")
    print(f"IMU samples: {imu_count}")
    print(f"Frames: {frame_count}")
    print(f"Data saved to: {output_dir}")
    print(f"{'='*50}")
    ser.close()
