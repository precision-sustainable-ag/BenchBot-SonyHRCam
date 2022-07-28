from MachineMotion import *
import sys

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

wheel_motors = [2, 3]
directions = ["positive", "negative"]
for axis in wheel_motors:
    mm.configAxis(axis, MICRO_STEPS.ustep_8,
                  MECH_GAIN.enclosed_timing_belt_mm_turn)
    mm.configAxisDirection(axis, directions[axis-2])
mm.emitAcceleration(50)
mm.emitSpeed(80)

distance = -300
positions = [distance, distance+00]

print('Starting Motion')

mm.moveRelativeCombined(wheel_motors, positions)
mm.waitForMotionCompletion()

print('Done')
