import serial as sl
import datetime as dt
from time import sleep
from influxdb import InfluxDBClient


def convert(byte):
    if byte > 127:
        return (256-byte)*(-1)
    return byte


sleep(5)

client = InfluxDBClient(host='150.165.254.119', username='allan', password='020898', database='CC1310_Packets', port=8086)
with sl.Serial("/dev/ttyACM0", 115200) as sr:  # GCOMPI ROOT
    while True:
        string = sr.read(124)
        for i in range(string[0]):
            line = "packet,sourceID={src},routerID={rtr},destinyID={dst},seqNumber={sn},txPower={tp} rssi={rssi},temperature={temp}".format(
                src=string[(i*8) + 1], rtr=string[(i*8) + 2], dst=string[(i*8) + 3], sn=int.from_bytes(string[(i*8) + 4:(i*8) + 6], byteorder='big'),
                tp=string[(i*8) + 6], rssi=convert(string[(i*8) + 7]), temp=string[(i*8) + 8])
            client.write(data=line, params={'db':'CC1310_Packets'}, expected_response_code=204, protocol='line')
            #print(line)
            #sleep(0.1)

