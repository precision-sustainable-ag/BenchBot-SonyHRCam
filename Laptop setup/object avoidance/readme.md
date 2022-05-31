
# File Description

**Tablet_move_forward.py** - Script to have the platform just moving forward or backward. This is just for testing of the controller.

**Tablet_RPIClientSensors.py** - Script for communicating with RPI Sensors and getting measurements. This script initializes the server in the RPI automatically. The location of the script may need to be updated depending on where it is put at the end.

**Tablet_ForwardControl.py** - Main script controlling the forward motion of the platform to make sure that it is following the RIGHT rail. Some issues were observed with the left sensors so we are not using them for the control at this point.

**RPI_sensor_testing.py** - This script tests one ultrasound sensor at the time. The parameters to be used for the trigger and echo depending on the sensor location are specified in this document.

**RPI_ServerSensors.py** - This is the script that starts are server for sending distance measurements when getting requests from a client (i.e, the script from the tablet). Note that the tablet scripts automatically will execute this script.

**SensorOffsets.csv** - It contains the offsets (in cm) for each sensor. The first offset is just a value of 1 cm for the forward / collision sensor. The second value corresponds to the distance between the outer part of the front wheel and the front sensor. The third value is the distance between the outer part of the front wheel and the back sensor.

**Diagrams.ppt** - Contains some details on how to wire the sensors.

# Requirements

The RPI scripts require **RPi.GPIO** (which I think comes pre-installed in the RPIs) and the tablet scripts only require **paramiko**.

# Setup

Here are a couple of pointers for setting up the system:

- After wiring each sensor, you may want to use the **RPI_sensor_testing** script to double check that the measurements are correct. Use the 5V power when wiring the sensors.
- Follow the steps here: https://www.makeuseof.com/raspberry-pi-set-static-ip/  to have the RPI with an static IP 192.168.7.3. The DNS can be the same as the routers IP address.
- I noticed that the and the tablet sometimes don't get the correct IP address right away. Often the MachineMotion needs to set up the network switch first. I check this by running `ipconfig` in the tablet to check that its ip address is 192.168.7.XX. If not, I would disconnect the ethernet table and connect it again. Once that is ready, you can check the connection to the MachineMotio by executing `ping 192.168.7.2`, and the connection to the RPI can be checked by executing `ping 192.168.7.3`.