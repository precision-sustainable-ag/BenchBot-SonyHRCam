import RPi.GPIO as GPIO
import time
import numpy as np

# GPIO Mode (BOARD / BCM)
GPIO.setmode(GPIO.BCM)

def distance(trigger,echo):
    # set Trigger to HIGH
    GPIO.output(trigger, True)

    # set Trigger after 0.01ms to LOW
    time.sleep(0.00001)
    GPIO.output(trigger, False)

    StartTime = time.time()
    StopTime = time.time()

    # save StartTime
    while GPIO.input(echo) == 0:
        StartTime = time.time()

    # save time of arrival
    while GPIO.input(echo) == 1:
        StopTime = time.time()

    # time difference between start and arrival
    TimeElapsed = StopTime - StartTime
    # multiply with the sonic speed (34300 cm/s)
    # and divide by 2, because there and back
    distance = (TimeElapsed * 34300) / 2
    return distance

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