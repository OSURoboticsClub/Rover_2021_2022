import matplotlib as mpl
import matplotlib.pyplot as plt

filePath = input("Please enter path to raw data:\n")

if(filePath == ""):
    filePath = "logs/LGHG2_10.0.csv"

f = open(filePath, 'r')

timeAr = []
currentAr = []
voltageAr = []
tempCellAr = []
tempFetAr = []

idx = 0
for line in f:
    line = line[:-1]
    line = line.replace(chr(0x00),"")
    lineData = line.split(",")
    timeAr.append(float(lineData[0]))
    voltageAr.append(float(lineData[1]))
    currentAr.append(float(lineData[2]))
    tempCellAr.append(float(lineData[3]))
    tempFetAr.append(float(lineData[4]))

measuredWh = 0
measuredAh = 0

for i in range(1,len(timeAr)):
    measuredAh += (timeAr[i]-timeAr[i-1])*(currentAr[i]+currentAr[i-1])/(2*3600)
    measuredWh += (timeAr[i]-timeAr[i-1])*(currentAr[i]+currentAr[i-1])*(voltageAr[i]+voltageAr[i-1])/(4*3600)

print(filePath[5:-8])
print("measured Ah: " + str(measuredAh))
print("measured Wh: " + str(measuredWh))

plt.subplots(2,1)
plt.subplot(2,1,1)
plt.title(filePath[5:-8] + " " + filePath[-8:-4] + "A")
plt.plot(timeAr,voltageAr)
plt.subplot(2,1,2)
plt.plot(timeAr,currentAr)
plt.xlabel("time (s)")


loadBeginIDX = 0
beginset = False
loadEndIDX = 0
i = 50
while(currentAr[i] <= float(filePath[-8:-4])):
    if(currentAr[i] >= 1 and not beginset):
        loadBeginIDX = i-50
        beginset = True
    i += 1
loadEndIDX = i+50

sagTime = []
sagVoltage = []
sagCurrent = []
for i in range(loadBeginIDX,loadEndIDX):
    sagTime.append(timeAr[i])
    sagVoltage.append(voltageAr[i])
    sagCurrent.append(currentAr[i])

print("estimated " + filePath[-7:-4] + "A sag voltage: " + str(sagVoltage[0] - sagVoltage[-1]))

plt.subplots(2,1)
plt.subplot(2,1,1)
plt.title(filePath[5:-8] + " " + filePath[-8:-4] + "A sag voltage")
plt.plot(sagTime,sagVoltage)
plt.subplot(2,1,2)
plt.plot(sagTime,sagCurrent)
plt.xlabel("time (s)")

plt.show()
