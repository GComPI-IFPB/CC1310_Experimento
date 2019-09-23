import serial


def convert(byte):
    if byte > 127:
        return (256-byte)*(-1)
    return byte


with serial.Serial("COM9", 115200) as sr:
    while True:
        string = sr.read(32)
        print("| ID:", string[0], "|", end=" ")
        print("SeqNumber:", int.from_bytes(string[1:3], byteorder="big"), "|", end=" ")
        print("Dado:", string[3:30].hex(), "|", end=" ")
        print("RSSI:", convert(int.from_bytes(string[31:32], byteorder="big")), "|")

