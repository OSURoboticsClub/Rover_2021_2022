import serial
import time

bcb = serial.Serial('COM8',115200, timeout=1)

str_current = input("Enter discharge current (multiple of 0.1A) or leave blankd for default 0:\n")
if(str_current == ""):
    str_current = "0.0"
current = float(str_current)
fileName = input("name battery Type or leave blank to not log data:\n")



bcb.write((str(chr(int(current*10)))).encode())

if(fileName != ""):
    fileName = ("logs/" + fileName + "_" + str(current) + ".csv")
if(fileName != ""):
    f = open(fileName, 'w')

print("Running profile")
startTime = time.perf_counter()

while(1):
    try:
        line = bcb.readline()
        line = line.decode("utf-8")
        line = line[1:]
        if(line == ""):
            if(fileName != ""):
                f.close()
            exit()
        timer = str(round(time.perf_counter() - startTime, 3))
        line = timer + "," + line
        print(line)
        if(fileName != ""):
            f.write(line)
    except:
        if(fileName != ""):
            f.close()
        exit()
