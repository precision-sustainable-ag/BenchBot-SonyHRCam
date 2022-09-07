from MachineMotion import *
import sys
import paramiko
import socket
import time
import numpy as np

state = 'MD'  # for now change this to have files renamed with the state you are in
# Length of the robot (between front and back sensors) measured in cm
robot_length = 133
# Width of the robot (distance between two front wheels) in cm
robot_width = 215
# if you haven't changed your pi's name the default username is 'pi'
pi_username = 'benchbot'
# if you haven't changed your pi's password, the default is 'raspberrypi'
pi_password = 'bb-semi-field'
distTravel = 300
# if BB moving backwards change the motors order to [3,2]
wheel_motors = [2, 3]


HOST = "192.168.7.3"  # The server's hostname or IP address
PORT = 65432  # The port used by the server

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
    print('debug_dist_raw', dist_raw)
    print('debug_dist_list', dist_list)
    dist_list = np.median(dist_list, 0)

    return dist_list


# Helper function for computing orientation of robot

def find_orientation(distance, robot_length):
    '''
    :param distance: np array. Average of N distances measured by the ultrasonic sensors. Array[0]=
    front right sensor, Array[1]= back right sensor.
    :param robot_length: distance in cm from front right to back right sensor
    :return: angle of drift from straight trajectory
    '''
    print('distance[0]=', distance[0])
    print('distance[1]=', distance[1])
    theta = np.arctan2((distance[0]-distance[1]), robot_length)*180/np.pi
    return theta

# INITIALIZING SERVER IN RPI


ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect(HOST, username='benchbot', password='bb-semi-field')
ssh_stdin, ssh_stdout, ssh_stderr = ssh.exec_command(
    'python /home/benchbot/ultrasonic_calibration/RPI_ServerSensors.py &')
time.sleep(2)


# CONNECTING TO RPI SERVER

# Setting up the connection
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))


# LOADING OFFSETS
offsets = np.loadtxt('SensorOffsets.csv', delimiter=',',
                     skiprows=1)  # delimiter added


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
#directions = ["positive","negative"]
directions = ["negative", "positive"]
for axis in wheel_motors:
    mm.configAxis(axis, MICRO_STEPS.ustep_8,
                  MECH_GAIN.enclosed_timing_belt_mm_turn)
    mm.configAxisDirection(axis, directions[axis-2])
mm.emitAcceleration(50)
mm.emitSpeed(80)

distance = 300

## MACHINE MOTION INITIALIZATION - END ###########################

# MAIN CONTROL LOOP

for k in range(0, 50):
    dist_corrected = get_distances(s, offsets)
    # Getting angle
    ang = find_orientation(dist_corrected, robot_length)
    print('debug_angle=', ang)

    # Calculate how much distance motor needs to move to align platform
    d_correction_mm = 2*np.pi*robot_width*(abs(ang)/360)*10
    print('debug_d_correction=', d_correction_mm)
    # Create if statement to indicate which motor moves

    if ang > 0.5:
        mm.moveRelative(wheel_motors[1], d_correction_mm)
        mm.waitForMotionCompletion()
    elif ang < -0.5:
        mm.moveRelative(wheel_motors[0], d_correction_mm)
        mm.waitForMotionCompletion()

    mm.moveRelativeCombined(wheel_motors, [distTravel, distTravel])
    mm.waitForMotionCompletion()