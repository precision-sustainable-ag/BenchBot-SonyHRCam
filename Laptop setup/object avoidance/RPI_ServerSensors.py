# server.py

import socket
import RPi.GPIO as GPIO
import time
import json

# GPIO Mode (BOARD / BCM)
GPIO.setmode(GPIO.BCM)


def distance(trigger, echo):
    # set Trigger to HIGH
    print(trigger, echo)
    GPIO.output(trigger, True)

    # set Trigger after 0.01ms to LOW
    time.sleep(0.00001)
    GPIO.output(trigger, False)

    start_time = time.time()
    stop_time = time.time()

    # save start_time
    print("start time")
    while GPIO.input(echo) == 0:
        start_time = time.time()

    print("stop time")
    # save time of arrival
    while GPIO.input(echo) == 1:
        stop_time = time.time()

    # time difference between start and arrival
    time_elapsed = stop_time - start_time
    # multiply with the sonic speed (34300 cm/s)
    # and divide by 2, because there and back
    distance = (time_elapsed * 34300) / 2
    print(distance)

    return distance


HOST = "192.168.7.3"  # Standard loopback interface address (localhost)
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)

# Setting up the connection
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen()
conn, addr = s.accept()
print(f"Connected by {addr}")

while True:
    # Waiting for request
    data = conn.recv(1024)
    if not data:
        break

    json_data_received = json.loads(data)
    gpio_trigger_list = json_data_received.get("trigger_pins")
    gpio_echo_list = json_data_received.get("echo_pins")

    if len(gpio_trigger_list) != len(gpio_echo_list):
        conn.sendall(bytearray('Array size mismatch!', 'utf8'))

    dist_list = [0.0] * len(gpio_echo_list)

    # Updating distances
    for k in range(0, len(gpio_trigger_list)):
        trigger = gpio_trigger_list[k]
        echo = gpio_echo_list[k]

        # Set GPIO direction (IN / OUT)
        GPIO.setup(trigger, GPIO.OUT)
        GPIO.setup(echo, GPIO.IN)

        dist_list[k] = round(distance(trigger, echo), 1)
        print(dist_list)

    # Sending response
    response = " ".join([str(d) for d in dist_list])
    print(response)
    conn.sendall(bytearray(response, 'utf8'))
