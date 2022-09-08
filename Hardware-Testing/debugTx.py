import paramiko
import socket
import time
import numpy as np
import sys
from subprocess import call
sys.path.append("..")
from MachineMotion import *


state = 'TX' # for now change this to have files renamed with the state you are in
lenRobot = 133  # Length of the robot (between front and back sensors) measured in cm
widRobot =215 # Width of the robot (distance between two front wheels) in cm
pi_username = 'pi' # if you haven't changed your pi's name the default username is 'pi'
pi_password = 'raspberrypi' # if you haven't changed your pi's password, the default is 'raspberrypi'
distTravel = 300
wheelMotors = [2, 3] # if BB moving backwards change the motors order to [3,2]

HOST = "192.168.7.1"  # The server's hostname or IP address
# HOST = "127.0.0.1"
PORT = 65432  # The 3ort used by the server

# Helper function for getting disntance measurements from sensor
def getDistances(s,offsets):

    # Number of queries of sensor measurements
    N = 2

    dist_list = np.zeros((N,6))
    dist_raw = np.zeros((N,6))

    for k in range(0,N):
        # Sending a request for data and receiving it
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


# Helper function for computing orientation of robot

def findOrientation(Dist, lenRobot):
    '''
    :param Dist: np array. Average of N distances measured by the ultrasonic sensors. Array[0]=
    front right sensor, Array[1]= back right sensor.
    :param lenRobot: distance in cm from front right to back right sensor
    :return: angle of drift from straight trajectory
    '''
    print('Dist[0]=', Dist[0])
    print('Dist[1]=', Dist[1])
    theta=np.arctan2((Dist[0]-Dist[1]), lenRobot)*180/np.pi
    return theta
## INITIALIZING SERVER IN RPI

# ssh = paramiko.SSHClient()
# ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
# ssh.connect(HOST, username='benchbot', password='bb-semi-field')
# ssh_stdin, ssh_stdout, ssh_stderr = ssh.exec_command('python /home/benchbot/ultrasonic_calibration/RPI_ServerSensors.py &')
# call('python /home/pi/BenchBot/ultrasonic_calibration/RPI_ServerSensors.py', shell=True)
time.sleep(2)


## CONNECTING TO RPI SERVER

# Setting up the connection
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
print(f"Connected to {HOST}")

## LOADING OFFSETS
offsets = np.loadtxt('SensorOffsets.csv', delimiter=',', skiprows = 1)# delimiter added


## MACHINE MOTION INITIALIZATION - START ###########################
# Initialize the machine motion object
mm = MachineMotion(DEFAULT_IP)
print('mm initialized')

# Remove the software stop
print("--> Removing software stop")
mm.releaseEstop()
print("--> Resetting system")
mm.resetSystem()

wheelMotors = [2,3]
directions = ["positive","negative"]
# directions = ["negative","positive"]
for axis in wheelMotors:
    mm.configAxis(axis, MICRO_STEPS.ustep_8, MECH_GAIN.enclosed_timing_belt_mm_turn)
    mm.configAxisDirection(axis, directions[axis-2])
mm.emitAcceleration(50)
mm.emitSpeed(80)
        
dist = 300

## MACHINE MOTION INITIALIZATION - END ###########################

## MAIN CONTROL LOOP
for k in range(0,50):
    dist_corrected = getDistances(s, offsets)
    # Getting angle
    ang = findOrientation(dist_corrected, lenRobot)
    print('debug_angle=', ang)

    #Calculate how much distance motor needs to move to align platform
    d_correction_mm = 2*np.pi*widRobot*(abs(ang)/360)*10
    print('debug_d_correction=', d_correction_mm)
    #Create if statement to indicate which motor moves
    
    if ang > 0.5:
        mm.moveRelative(wheelMotors[1], d_correction_mm)
        mm.waitForMotionCompletion()
    # elif ang < -0.5:
        # mm.moveRelative(wheelMotors[0], d_correction_mm)
        # mm.waitForMotionCompletion()

    mm.moveRelativeCombined(wheelMotors, [distTravel, distTravel])
    mm.waitForMotionCompletion()
