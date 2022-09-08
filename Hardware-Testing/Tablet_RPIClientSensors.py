# client.py

import paramiko
import socket
import time
import numpy as np

HOST = "192.168.7.3"  # The server's hostname or IP address
PORT = 65432  # The port used by the server

# Length of the robot (between front and back sensors) measured in cm
robot_length = 133

# Helper function for computing orientation of robot


def find_orientation(distance, robot_length):
    r1 = distance[0]
    r2 = distance[1]
    fac = 1

    d = robot_length

    if r1 == r2:
        return 0
    elif r1 > r2:
        r1 = distance[1]
        r2 = distance[0]
        fac = -1

    a = d*r1/(r2-r1)

    tmp = a*r1/np.sqrt(np.power(a, 2)+np.power(r1, 2))
    p1 = np.array([-r1*tmp/a, tmp])

    tmp = (a+d)*r2/np.sqrt(np.power(a+d, 2)+np.power(r2, 2))
    p2 = np.array([(a+d)*tmp/r2-np.sqrt(np.power(a, 2)+np.power(r1, 2)), tmp])

    C2 = np.sqrt(np.power(a+d, 2)+np.power(r2, 2)) - \
        np.sqrt(np.power(a, 2)+np.power(r1, 2))

    dp = p2-p1
    dp[1] = fac*dp[1]
    theta = np.arctan2(dp[1], dp[0])
    print(dp)

    return theta

# INITIALIZING SERVER IN RPI


ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect(HOST, username='pi', password='raspberry')
ssh_stdin, ssh_stdout, ssh_stderr = ssh.exec_command(
    'python /home/pi/ultrasonic_calibration/RPI_ServerSensors.py &')
time.sleep(2)


# CONNECTING TO RPI SERVER

# Setting up the connection
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))


# LOADING OFFSETS
offsets = np.loadtxt('SensorOffsets.csv')
print(offsets)


# GETTING MESSAGES FROM RPI SERVER

for i in range(0, 10):
    # Sending a request for data and receiving it
    s.sendall(b" ")
    data = s.recv(1024)
    print(data)

    # Parsing the measurements
    dist_list = []
    for item in data.split():
        dist_list.append(float(item))
    print(np.round(dist_list, 1))
    for k in range(0, len(dist_list)):
        dist_list[k] -= offsets[k % 3]
    print(np.round(dist_list, 1))

    # Getting angle
    angle_right = -find_orientation(dist_list[1:3], robot_length)
    angle_left = find_orientation(dist_list[4:6], robot_length)
    print(np.round(angle_right*180/np.pi, 1),
          np.round(angle_left*180/np.pi, 1))

    time.sleep(1)
