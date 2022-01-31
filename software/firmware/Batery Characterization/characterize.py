import serial
import time

bcb = serial.Serial('COM6',115200)

current = float(input("Enter discharge current (multiple of 0.1A):\n"))
fileName = input("name battery Type:\n")

bcb.write((str(chr(int(current*10)))).encode())

fileName = ("logs/" + fileName + "_" + str(current) + ".csv")

f = open(fileName, 'w')

print("Running profile")
time.sleep(1)

while(1):
    try:
        line = bcb.readline()
        line = line.decode("utf-8")
        line = line[2:]
        print(line)
        f.write(line)
    except:
        f.close()
        exit()
