from MachineMotion import *
import sys
import paramiko
import socket
import time
import numpy as np

HOST = "192.168.7.3"  # The server's hostname or IP address
PORT = 65432  # The port used by the server

# Length of the robot (between front and back sensors) measured in cm
robot_length = 133

# Helper function for getting disntance measurements from sensor


def get_distances(s, offsets):

    # Number of queries of sensor measurements
    N = 3

    dist_list = np.zeros((N, 6))
    dist_raw = np.zeros((N, 6))

    for k in range(0, N):
        # Sending a request for data and receiving it
        s.sendall(b" ")
        data = s.recv(1024)
        time.sleep(0.25)

        # Parsing the measurements
        for i, item in enumerate(data.split()):
            dist_raw[k, i] = float(item)
            dist_list[k, i] = float(item) - offsets[i % 3]
    # print(dist_raw)
    # print(dist_list)
    dist_list = np.median(dist_list, 0)

    return dist_list


# Helper function for computing orientation of robot
def find_orientation(distance, robot_length):
    R1 = distance[0]
    R2 = distance[1]
    fac = 1

    d = robot_length

    if R1 == R2:
        return 0
    elif R1 > R2:
        R1 = distance[1]
        R2 = distance[0]
        fac = -1

    a = d*R1/(R2-R1)

    tmp = a*R1/np.sqrt(np.power(a, 2)+np.power(R1, 2))
    p1 = np.array([-R1*tmp/a, tmp])

    tmp = (a+d)*R2/np.sqrt(np.power(a+d, 2)+np.power(R2, 2))
    p2 = np.array([(a+d)*tmp/R2-np.sqrt(np.power(a, 2)+np.power(R1, 2)), tmp])

    C2 = np.sqrt(np.power(a+d, 2)+np.power(R2, 2)) - \
        np.sqrt(np.power(a, 2)+np.power(R1, 2))

    dp = p2-p1
    dp[1] = fac*dp[1]
    theta = np.arctan2(dp[1], dp[0])

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


## MACHINE MOTION INITIALIZATION - START ###########################

sys.path.append("..")

# Initialize the machine motion object
mm = MachineMotion(DEFAULT_IP)
print('mm initialized')

# Remove the software stop
print("--> Removing software stop")
mm.releaseEstop()
print("--> Resetting system")
mm.resetSystem()

wheel_motors = [2, 3]
directions = ["positive", "negative"]
for axis in wheel_motors:
    mm.configAxis(axis, MICRO_STEPS.ustep_8,
                  MECH_GAIN.enclosed_timing_belt_mm_turn)
    mm.configAxisDirection(axis, directions[axis-2])
mm.emitAcceleration(50)
mm.emitSpeed(80)

travelDist = 300

## MACHINE MOTION INITIALIZATION - END ###########################

# MAIN CONTROL LOOP

for k in range(0, 14):
    # Getting distance measurements
    dist_list = get_distances(s, offsets)
    print(np.round(dist_list, 1))

    # Getting angle
    angle_right = -find_orientation(dist_list[1:3], robot_length)
    angle_left = find_orientation(dist_list[4:6], robot_length)
    ang = np.array([angle_right, angle_left, (angle_right + angle_left)/2])
    print(np.round(ang*180/np.pi, 1))

    # Computing feedback
    Kp = 0.02
    Kd = 0.02
    # Using only right hand side because of issue with sensors
    U = Kp*(10-dist_list[1])-Kd*ang[0]

    posFeedback = [travelDist*(1+U), travelDist*(1-U)]
    print(posFeedback)

    mm.moveRelativeCombined(wheel_motors, posFeedback)
    mm.waitForMotionCompletion()
