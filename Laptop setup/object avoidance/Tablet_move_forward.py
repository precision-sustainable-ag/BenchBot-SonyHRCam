import sys

sys.path.append("..")
from MachineMotion import *

#### Machine Motion initialization ####

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
for axis in wheelMotors:
    mm.configAxis(axis, MICRO_STEPS.ustep_8, MECH_GAIN.enclosed_timing_belt_mm_turn)
    mm.configAxisDirection(axis, directions[axis-2])
mm.emitAcceleration(50)
mm.emitSpeed(80)
        
dist = -300
positions = [dist, dist+00]

print('Starting Motion')

mm.moveRelativeCombined(wheelMotors, positions)
mm.waitForMotionCompletion()

print('Done')