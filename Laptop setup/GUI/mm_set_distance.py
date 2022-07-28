import sys

from MachineMotion import *

sys.path.append("..")

#### Machine Motion initialization ####

# Initialize the machine motion object
mm = MachineMotion(DEFAULT_IP)
print('mm initialized')

# Remove the software stop
print("--> Removing software stop")
mm.releaseEstop()
print("--> Resetting system")
mm.resetSystem()

# Configure the axis number 1, 8 uSteps and 10 mm / turn for a ball screw linear drive
axis = AXIS_NUMBER.DRIVE2
micro_step = MICRO_STEPS.ustep_8
mech_gain = MECH_GAIN.ballscrew_10mm_turn
mm.configAxis(axis, micro_step, mech_gain)
print("Axis " + str(axis) + " configured with " + str(micro_step) + " microstepping and " + str(
    mech_gain) + "mm/turn mechanical gain")

# Configure the axis direction
ax_direction = DIRECTION.POSITIVE
mm.configAxisDirection(axis, ax_direction)
print("Axis direction set to " + str(ax_direction))

# Relative Move Parameters
speed = 10
acceleration = 80

# Load Relative Move Parameters
mm.emitSpeed(speed)
mm.emitAcceleration(acceleration)

# Start the relative move
distance = int(input('How much do you want to move?:'))
d = input('If direction positive press p. If direction negative press n.')
if d == 'p':
    direction = 'positive'
elif d == 'n':
    direction = 'negative'
# change direction ('positive' or 'negative) and distance to test the best position
mm.emitRelativeMove(axis, direction, distance)
mm.waitForMotionCompletion()
mm.emitStop()
distance_home = mm.getCurrentPositions()[2]
print(f'You have moved {distance_home}')
