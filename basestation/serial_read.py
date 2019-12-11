import serial as sl
import datetime as dt
from influxdb import InfluxDBClient
#
#
# def convert(byte):
#     if byte > 127:
#         return (256-byte)*(-1)
#     return byte
#
#
# archNamefrmt = "%d_%m_%Y-%H_%M_%S"
# archStyle = "%d/%m/%Y - %H:%M:%S"
# archname = "Log" + dt.datetime.now().strftime(archNamefrmt) + ".txt"
# archive = "C:\\Users\\Left\\PycharmProjects\\Influx\\" + archname
# client = InfluxDBClient(host='localhost', port=8086)
# with sl.Serial("COM9", 115200) as sr:  # GCOMPI ROOT
#     print("OPA")
#     while True:
#         # file = open(archive, "a")
#         # file = open(archive, "a")
#         string = sr.read(124)
#         # time = dt.datetime.now().strftime(archStyle)
#         # logString = time + "\n"
#         # print(string.hex())
#
#         for i in range(string[0]):
#             line = "packet,srcID={},dstID={},seqNumber={},txPower={} RSSI={},temperature={}".format(
#                 string[(i*7) + 1], string[(i*7) + 6], int.from_bytes(string[(i*7) + 2: (i*7) + 4], byteorder="big"),
#                 string[(i*7) + 5], convert(string[(i*7) + 7]), string[(i*7) + 4])
#             client.write(data=line, protocol='line')
#             # logString += "| Sensor ID: {} | ".format(string[(i*7) + 1])
#             # logString += "SeqNumber: {} | ".format(int.from_bytes(string[(i*7) + 2: (i*7) + 4], byteorder="big"))
#             # logString += "Temperature: {}°C | ".format(string[(i*7) + 4])
#             # logString += "txPower: {} | ".format(string[(i*7) + 5])
#             # logString += "dst ID: {} | ".format(string[(i*7) + 6])
#             # logString += "RSSI: {} | ".format(convert(string[(i*7) + 7]))
#             # logString += "\n"
#             # file.write(logString)
#             # print(logString)
#
#         # file.close()
# -*- coding: utf-8 -*-
"""Tutorial on using the InfluxDB client."""

import argparse

from influxdb import InfluxDBClient


def main(host='localhost', port=8086):
    """Instantiate a connection to the InfluxDB."""
    user = 'root'
    password = 'root'
    dbname = 'example'
    dbuser = 'smly'
    dbuser_password = 'my_secret_password'
    query = 'select Float_value from cpu_load_short;'
    query_where = 'select Int_value from cpu_load_short where host=$host;'
    bind_params = {'host': 'server01'}
    json_body = [
        {
            "measurement": "cpu_load_short",
            "tags": {
                "host": "server01",
                "region": "us-west"
            },
            "time": "2009-11-10T23:00:00Z",
            "fields": {
                "Float_value": 0.64,
                "Int_value": 3,
                "String_value": "Text",
                "Bool_value": True
            }
        }
    ]

    client = InfluxDBClient(host, port)

    print("Create database: " + dbname)
    client.create_database(dbname)

    print("Create a retention policy")
    client.create_retention_policy('awesome_policy', '3d', '3', default=True)

    print("Switch user: " + dbuser)
    client.switch_user(dbuser, dbuser_password)

    print("Write points: {0}".format(json_body))
    client.write_points(json_body)

    print("Querying data: " + query)
    result = client.query(query)

    print("Result: {0}".format(result))

    print("Querying data: " + query_where)
    result = client.query(query_where, bind_params=bind_params)

    print("Result: {0}".format(result))

    print("Switch user: " + user)
    client.switch_user(user, password)

    print("Drop database: " + dbname)
    client.drop_database(dbname)


def parse_args():
    """Parse the args."""
    parser = argparse.ArgumentParser(
        description='example code to play with InfluxDB')
    parser.add_argument('--host', type=str, required=False,
                        default='localhost',
                        help='hostname of InfluxDB http API')
    parser.add_argument('--port', type=int, required=False, default=8086,
                        help='port of InfluxDB http API')
    return parser.parse_args()


if __name__ == '__main__':
    # args = parse_args()
    main()