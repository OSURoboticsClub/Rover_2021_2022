import serial
import time

bcb = serial.Serial('COM6',115200)

current = float(input("Enter discharge current (multiple of 0.1A):\n"))

try:
    bcb.write((str(chr(int(current*10)))).encode())
except:
    bcb.close()
    time.sleep(0.5)
    bcb.open()
    bcb.write((str(chr(int(current*10)))).encode())

print("Running profile")
time.sleep(1)

while(1):
    try:
        line = bcb.readline()
    except:
        bcb.close()
        time.sleep(0.5)
        bcb.open()
        line = bcb.readline()
    print(line)
