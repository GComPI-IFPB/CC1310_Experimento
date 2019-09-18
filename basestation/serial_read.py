import serial

def convert(byte):
    if byte > 127:
        return (256-byte)*(-1)
    return byte


with serial.Serial("COM9", 115200) as sr:
    while True:
        string = sr.read(32)
        print("Dado:", string[0:30].hex(), end=" ")
        print("RSSI:", convert(int.from_bytes(string[31:32], byteorder='big')))


