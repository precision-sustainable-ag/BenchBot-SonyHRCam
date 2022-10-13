import os
from dataclasses import dataclass

import numpy as np
from dacite import from_dict
from dotenv import load_dotenv

from MachineMotion import *
from RPI_Sensors import UltrasonicSensor

###############################################################
######################### DATACLASSES #########################
###############################################################


@dataclass
class BBotConfig():
    """Main dataclass for bbot that holds config information from .env file
    """
    HOST_TYPE: str
    OAK: str
    ROBOT_LENGTH: str
    ROBOT_WIDTH: str
    HOME_TO_END_SENSOR_DISTANCE: str
    PI_USERNAME: str
    PI_PASSWORD: str
    DISTANCE_TRAVELED: str
    WHEEL_MOTORS: str
    NUMBER_OF_SENSORS: str
    DIRECTIONS: str
    TRIGGER_PINS: str
    ECHO_PINS: str

    def __post_init__(self):
        self.TRIGGER_PINS = list(map(int, self.TRIGGER_PINS.split(',')))
        self.ECHO_PINS = list(map(int, self.ECHO_PINS.split(',')))
        self.WHEEL_MOTORS = list(map(int, self.WHEEL_MOTORS.split(',')))
        self.ROBOT_LENGTH = int(self.ROBOT_LENGTH)
        self.ROBOT_WIDTH = int(self.ROBOT_WIDTH)
        self.HOME_TO_END_SENSOR_DISTANCE = int(self.HOME_TO_END_SENSOR_DISTANCE)
        self.DISTANCE_TRAVELED = int(self.DISTANCE_TRAVELED)
        self.NUMBER_OF_SENSORS = int(self.NUMBER_OF_SENSORS)
        self.DIRECTIONS = list(map(str, self.DIRECTIONS.split(',')))


####################################################################
########################  HELPER FUNCTIONS  ########################
####################################################################


def get_config():
    """Gets config setting for gohome.py

    Returns:
        dataclass object
    """
    load_dotenv()
    keys = dict(os.environ).keys()
    cfg_dict = {}
    for key in keys:
        cfg_dict.update({key: os.environ.get(key)})
    cfg_dc = from_dict(data_class=BBotConfig, data=cfg_dict)
    return cfg_dc


def read_offsets(offset_path):
    """Reads ultrasonic sensor offset informations

    Args:
        offset_path (str): path to offsets.csv

    Returns:
        array
    """
    offsets = np.loadtxt(offset_path, delimiter=',', skiprows=1)
    return offsets


def get_distances_rpi(cfg, offsets):
    """Calculates distance of ultrasonic sensor to rail.

    Args:
        cfg (dataclass): BBotConfig dataclass
        offsets (array): array of sensor offsets

    Returns:
        list: list of distance offsets for each wheel
    """
    trigger_list = cfg.TRIGGER_PINS
    echo_list = cfg.ECHO_PINS
    dist_list = np.zeros((cfg.NUMBER_OF_SENSORS, cfg.NUMBER_OF_SENSORS))
    sensor = []

    for num in range(0, cfg.NUMBER_OF_SENSORS):
        sensor.append(UltrasonicSensor(trigger_list[num], echo_list[num]))

    for k in range(0, cfg.NUMBER_OF_SENSORS):
        for i in range(0, cfg.NUMBER_OF_SENSORS):
            dist_raw = round(sensor[i].distance(), 1)
            dist_list[k, i] = dist_raw - offsets[i]

    dist_list = np.median(dist_list, 0)
    return dist_list


# Helper function for computing orientation of robot
def find_orientation(bbot_length, distance):
    '''
    :param distance: np array. Average of N distances measured by the ultrasonic sensors.
    Array[0]= front right sensor, Array[1]= back right sensor.
    :param ROBOT_LENGTH: distance in cm from front right to back right sensor
    :return: angle of drift from straight trajectory
    '''
    print("\nFinding orientation...")
    # print('distance[0]=', distance[0])
    # print('distance[1]=', distance[1])
    theta = np.arctan2((distance[0] - distance[1]), bbot_length) * 180 / np.pi
    return theta
