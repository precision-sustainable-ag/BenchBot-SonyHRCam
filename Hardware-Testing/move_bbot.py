import os
import sys
import threading
import time

import openpyxl
import pandas as pd

sys.path.append("..")
sys.path.append("../..")

import socket
from datetime import date

import numpy as np
import paramiko
from PyQt5 import QtWidgets
from PyQt5.QtCore import QTimer
from PyQt5.QtWidgets import *
from ultrasonic_calibration.sensor_utils import distance, findOrientation

from MachineMotion import *

state = 'TX' 
lenRobot = 133 
widRobot =218 
pi_username = 'pi'
pi_password = 'raspberrypi'
distTravel = 300
wheelMotors = [2, 3]

HOST = "192.168.7.1"
PORT = 65432 

## CONNECTING TO RPI SERVER
# Setting up the connection
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

# Initialize the machine motion object
mm = MachineMotion(DEFAULT_IP)
print("--> Removing software stop")
mm.releaseEstop()
print("--> Resetting system")
mm.resetSystem()
print("successfully created mm object")
camMotor = 1
mm.configAxis(camMotor, MICRO_STEPS.ustep_8, MECH_GAIN.ballscrew_10mm_turn)
mm.configAxisDirection(camMotor, DIRECTION.POSITIVE)

directions = ["positive","negative"]
for axis in wheelMotors:
    mm.configAxis(axis, MICRO_STEPS.ustep_8, MECH_GAIN.enclosed_timing_belt_mm_turn)
    mm.configAxisDirection(axis, directions[axis-2])
mm.emitAcceleration(50)
mm.emitSpeed(80)

mm.moveRelative(wheelMotors[0], 10)

mm.waitForMotionCompletion()
mm.triggerEstop()
