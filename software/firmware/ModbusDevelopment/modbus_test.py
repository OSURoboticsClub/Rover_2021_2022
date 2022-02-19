import serial
import time

"""
in c

UInt16 ModRTU_CRC(byte[] buf, int len)
{
  UInt16 crc = 0xFFFF;

  for (int pos = 0; pos < len; pos++) {
    crc ^= (UInt16)buf[pos];          // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  return crc;
}

"""

def modRTU_CRC(packet,len):
    crc = 0xffff
    for i in range(len):
        crc ^= packet[i]

        for j in range(8,0,-1):
            if((crc & 0x001) != 0):
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    crcar = crc.to_bytes(2,'big')
    return crcar

node = serial.Serial('COM5',115200, timeout=1)

"""
write command (from master)
[slave id, function code, start register bytes (x2), end register bytes (x2), number of bytes, data bytes (x???), CRC bytes(x2) ]

write response (from slave)
[slave id, function code, start register bytes(x2), end register bytes (x2), CRC bytes (x2)]

read command (from master)
[slave id, function code, start register bytes(x2), end register bytes (x2), CRC bytes (x2)]

read response (from slave)
[slave id, function code, number of bytes, data bytes (x???), CRC bytes(x2)]
"""

test_packets = [
    [0x01, 0x10, 0x00, 0x00, 0x00, 0x01, 0x02, 0x42, 0x42, 0xc1, 0x16]#, #write 4112 to int reg
    # [0x01, 0x10, 0x01, 0x00, 0x01, 0x01, 0x04, 0x40, 0x48, 0xf5, 0x7c, 0xe7], #write 3.14 to float reg
    # [0x01, 0x10, 0x02, 0x00, 0x02, 0x01, 0x01, 0x42, 0x3d, 0x40], #write 'B' to char reg
    # [0x01, 0x10, 0x03, 0x00, 0x03, 0x01, 0x01, 0x01, 0xe1, 0x01], #write True to bool reg
]


print("Running profile")
startTime = time.perf_counter()

for packet in test_packets:
    node.write(bytes(packet))
    response_bytes = node.readline()
    response_bytes_readable = [hex(b) for b in response_bytes]

    print(response_bytes_readable)
