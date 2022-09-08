import socket
from utils import findOrientation
import numpy as np
import time


# HOST = "127.0.0.1"
HOST = ""
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)
lenRobot = 133
widRobot =215

## LOADING OFFSETS
offsets = np.loadtxt(
    '/home/pi/BenchBot/Laptop setup/GUI/SensorOffsets.csv', 
    delimiter=',', skiprows = 1
    )# delimiter added

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def getDistances(s,offsets):

    # Number of queries of sensor measurements
    N = 2

    dist_list = np.zeros((N,6))
    dist_raw = np.zeros((N,6))

    for k in range(0,N):
        # Sending a request for data and receiving it
        # s.sendto(b" ",(HOST, PORT))
        s.sendall(b" ")
        data = s.recv(1024)
        time.sleep(0.25)

        # Parsing the measurements
        for i,item in enumerate(data.split()):
            dist_raw[k,i] = float(item)
            dist_list[k,i] = float(item) - offsets[i%3]
    print('debug_dist_raw', dist_raw)
    print('debug_dist_list', dist_list)
    dist_list = np.median(dist_list,0)
    
    return dist_list


print("Connect to host")
s.connect((HOST, PORT))
for k in range(0,50):
    dist_corrected = getDistances(s, offsets)
    # Getting angle
    ang = findOrientation(dist_corrected, lenRobot)
    print('debug_angle=', ang)
    #Calculate how much distance motor needs to move to align platform
    d_correction_mm = 2*np.pi*widRobot*(abs(ang)/360)*10
    print('debug_d_correction=', d_correction_mm)
