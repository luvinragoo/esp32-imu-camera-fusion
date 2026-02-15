import serial
import time
from pathlib import Path


PORT = "COM13"  # Change to correct port
BAUD = 115200

# Get script directory, then navigate to data folder
script_dir = Path(__file__).parent  # pc_scripts/
project_root = script_dir.parent     # esp32-imu-camera-fusion/
out_dir = project_root / "data" / "captured_frames"
out_dir.mkdir(exist_ok=True, parents=True)


def read_line(ser):
    """Read a line from serial, handle decoding errors gracefully"""
    return ser.readline().decode("utf-8", errors="ignore").strip()

def main():
    print(f"Opening serial port {PORT} at {BAUD} baud...")
    
    try:
        with serial.Serial(PORT, BAUD, timeout=2) as ser:
            # Give ESP32 time to reset/stabilize
            time.sleep(2)
            print("Connected! Waiting for frames...")

            frame_idx = 0
            while True:
                line = read_line(ser)
                
                # Wait for frame start marker
                if line != "FRAME_START":
                    if line:  # Print any other debug messages
                        print(f"Debug: {line}")
                    continue

                # Read length line
                len_line = read_line(ser)
                if not len_line.isdigit():
                    print(f"⚠️  Bad length line: '{len_line}'")
                    continue
                
                length = int(len_line)
                print(f"📸 Receiving frame {frame_idx}, {length} bytes... ", end="", flush=True)

                # Read exactly `length` bytes of JPEG data
                data = ser.read(length)
                if len(data) != length:
                    print(f"❌ Incomplete! Got {len(data)}/{length} bytes")
                    continue

                # Consume trailing newlines and check for FRAME_END
                read_line(ser)  # empty line after binary data
                end_marker = read_line(ser)
                
                if end_marker != "FRAME_END":
                    print(f"⚠️  Expected FRAME_END, got: '{end_marker}'")

                # Save JPEG
                fname = out_dir / f"frame_{frame_idx:03d}.jpg"
                with open(fname, "wb") as f:
                    f.write(data)
                
                print(f"✅ Saved to {fname}")
                frame_idx += 1

    except serial.SerialException as e:
        print(f"❌ Serial error: {e}")
        print(f"\nTip: Check that:")
        print(f"  1. ESP32 is plugged in")
        print(f"  2. Arduino Serial Monitor is CLOSED")
        print(f"  3. PORT is correct (current: {PORT})")
    except KeyboardInterrupt:
        print(f"\n\n⏹️  Stopped. Captured {frame_idx} frames total.")

if __name__ == "__main__":
    main()
