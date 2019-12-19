from influxdb import InfluxDBClient

client = InfluxDBClient(host='150.165.254.119', username='client', password='', database='CC1310_Packets', port=8086)
line = input()
rs = client.query('{}'.format(line))
print(line)
for i in rs.get_points(measurement='packet'):
    print(i)


# SELECT * FROM packet WHERE (sourceID='1') AND (time > now() - 1h) limit 1