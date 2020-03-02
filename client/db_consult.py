from influxdb import InfluxDBClient


def createList():
    l = client.query('SHOW MEASUREMENTS')
    l = list(l.items()[0][1])
    new_list = []
    for i in l:
        new_list.append(i['name'])

    return new_list


client = InfluxDBClient(host='150.165.254.119', username='client', password='', database='CC1310_Packets', port=8086)
# line = input()
rs = createList()
print("Lista de tabelas:")
for i in range(len(rs)):
    print(i, rs[i])

query = input("\nDigite a query: ")
rs = client.query(query)
for i in rs.get_points():
    print(i)
