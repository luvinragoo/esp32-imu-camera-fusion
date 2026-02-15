from mpu6050 import MPU6050
import time
 
mpu=MPU6050(14,13) #attach the IIC pin(sclpin,sdapin)
mpu.MPU_Init()     #initialize the MPU6050
G = 9.8
time.sleep_ms(1000)#waiting for MPU6050 to work steadily

try:
    while True:
        t_ms = time.ticks_ms();
        accel=mpu.MPU_Get_Accelerometer()#gain the values of Acceleration
        gyro=mpu.MPU_Get_Gyroscope()     #gain the values of Gyroscope
        
        print("{}|{},{},{}|{},{},{}".format(t_ms, accel[0], accel[1], accel[2], gyro[0], gyro[1], gyro[2]))

        time.sleep_ms(50)
except:
    pass
