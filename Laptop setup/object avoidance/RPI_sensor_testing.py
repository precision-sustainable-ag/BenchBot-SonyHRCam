
#Libraries
import RPi.GPIO as GPIO
import time
 
#GPIO Mode (BOARD / BCM)
GPIO.setmode(GPIO.BCM)
 
#set GPIO Pins
GPIO_TRIGGER = 5
GPIO_ECHO = 19

#GPIO_TRIGGER 25 and GPIO_ECHO 8 correspond to the front left sensor
#GPIO_TRIGGER 2 and GPIO_ECHO 3 correspond to back left sensor
#GPIO_TRIGGER 17 and GPIO_ECHO 4 corresponds to back right sensor
#GPIO_TRIGGER 18 and GPIO_ECHO 23 correspond to front right sensor
#GPIO_TRIGGER 22 and GPIO_ECHO 27 corresponds to the right forward sensor
#GPIO_TRIGGER 5 and GPIO_ECHO 19 corresponds to the left forward sensor

#front Left sensor is 9cm from the wheel
#back left sensor is 15cm from the edge of the gaussets
#back right sensor is 15cm from the edge of the gaussets
#front right sensor is 13cm from the wheel


#set GPIO direction (IN / OUT)
GPIO.setup(GPIO_TRIGGER, GPIO.OUT)\
                         
                         
GPIO.setup(GPIO_ECHO, GPIO.IN)
 
def distance():
    # set Trigger to HIGH
    GPIO.output(GPIO_TRIGGER, True)
 
    # set Trigger after 0.01ms to LOW
    time.sleep(0.00001)
    GPIO.output(GPIO_TRIGGER, False)
 
    StartTime = time.time()
    StopTime = time.time()
 
    # save StartTime
    while GPIO.input(GPIO_ECHO) == 0:
        StartTime = time.time()
 
    # save time of arrival
    while GPIO.input(GPIO_ECHO) == 1:
        StopTime = time.time()
 
    # time difference between start and arrival
    TimeElapsed = StopTime - StartTime
    # multiply with the sonic speed (34300 cm/s)
    # and divide by 2, because there and back
    distance = (TimeElapsed * 34300) / 2
 
    return distance
 
if __name__ == '__main__':
    try:
        while True:
            dist = distance()
            print ("Measured Distance = %.1f cm" % dist)
            time.sleep(1)
 
        # Reset by pressing CTRL + C
    except KeyboardInterrupt:
        print("Measurement stopped by User")
        GPIO.cleanup()
