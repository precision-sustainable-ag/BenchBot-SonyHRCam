import socket
import RPi.GPIO as GPIO
from utils import distance

HOST = ""
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)

GPIO_TRIGGER_LIST = [18, 17]
GPIO_ECHO_LIST = [23, 4]

# Initializing distances
dist_list = [0.0, 0.0]

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    print(conn)

    with conn:
        print(f"Connected by {addr}")
        while True:
            data = conn.recv(1024)
            if not data:
                break

            # Updating distances
            for k in range(0,len(GPIO_TRIGGER_LIST)):
                trigger = GPIO_TRIGGER_LIST[k]
                echo = GPIO_ECHO_LIST[k]

                # Set GPIO direction (IN / OUT)
                GPIO.setup(trigger, GPIO.OUT)
                GPIO.setup(echo, GPIO.IN)

                dist_list[k] = round(distance(trigger,echo),1)

            # Sending response
            res = " ".join([str(d) for d in dist_list])
            conn.sendall(bytearray(res,'utf8'))

