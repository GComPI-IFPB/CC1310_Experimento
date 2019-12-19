import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import dates
import time
from datetime import datetime
from influxdb import InfluxDBClient

clt = InfluxDBClient(host='150.165.254.119', username='client', password='', database='CC1310_Packets', port=8086)
#plt.ion() # set plot to animated

fig = plt.figure()
fig2 = plt.figure()
ax1 = fig.add_subplot(1,1,1)
ax2 = fig2.add_subplot(1,1,1)

def animate(i):

    ytempdata = []
    yrssidata = []
    xtimedata = []

    ytempdata2 = []
    yrssidata2 = []
    xtimedata2 = []
    
    while(True):

        data_node_a = clt.query('{}'.format("SELECT * FROM packet WHERE (sourceID='1') AND (time > now() - 1h)"))
        data_node_b = clt.query('{}'.format("SELECT * FROM packet WHERE (sourceID='2') AND (time > now() - 1h)"))

        for i in data_node_a.get_points(measurement='packet'):
            ytempdata.append(int(i['temperature']))
            yrssidata.append(int(i['rssi']))
            string = i['time']
            string = string.replace("T", " ")
            string = string[0:string.index('.')]
            xtimedata.append(datetime.strptime(string, '%Y-%m-%d %H:%M:%S'))
        
        for i in data_node_b.get_points(measurement='packet'):
            ytempdata2.append(int(i['temperature']))
            yrssidata2.append(int(i['rssi']))
            string = i['time']
            string = string.replace("T", " ")
            string = string[0:string.index('.')]
            xtimedata2.append(datetime.strptime(string, '%Y-%m-%d %H:%M:%S'))

        break
    
    ax1.clear()
    lab = 'Temperatura'
    ax1.set_ylabel(lab, fontsize=20)
    ax1.plot(xtimedata, ytempdata, label='Nó 1')
    ax1.plot(xtimedata2, ytempdata2, label='Nó 2')
    ax1.legend()
    ax2.clear()
    lab1 = 'Power (Dbm)'
    ax2.set_ylabel(lab1, fontsize=20)
    ax2.plot(xtimedata, yrssidata, label='Nó 1')
    ax2.plot(xtimedata2, yrssidata2, label='Nó 2')
    ax2.legend()
    plt.ylim([-45,-20])



ani = animation.FuncAnimation(fig, animate, interval=1000)
plt.show()