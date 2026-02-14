import serial

ser = serial.Serial('COM13',115200)
print("Connected to ESP32")

try:
    with open('data/imu_log.csv', 'w') as f:
        f.write('timestamp,ax,ay,az,gx,gy,gz\n')

        for i in range(200):
            line = ser.readline().decode().strip()
            line_list = line.split('|')
            timestamp = line_list[0]
            ax = int(line_list[1].split(',')[0])
            ay = int(line_list[1].split(',')[1])
            az = int(line_list[1].split(',')[2])

            gx = int(line_list[2].split(',')[0])
            gy = int(line_list[2].split(',')[1])
            gz = int(line_list[2].split(',')[2])

            f.write(f'{timestamp},{ax},{ay},{az},{gx},{gy},{gz}\n')

            if (i + 1) % 50 == 0:
                print(f"Captured {i + 1} samples...")
                
        print("Done! Data saved to data/imu_log.csv")
 
except IOError as e:
    print(f"An error occurred: {e}")

    
