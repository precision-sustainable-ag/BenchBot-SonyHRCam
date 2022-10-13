import argparse
import time

import numpy as np

from bbot_utils import (find_orientation, get_config, get_distances_rpi,
                        read_offsets)
from MachineMotion import *


class GoHome:

    def __init__(self, offset_path, move_distance):
        """Main BBot operation class that use configuration information to 
        move the benchbot back to start position after acquisition has 
        completed for a single bed.

        Args:
            offset_path (str): path to SensorOffsets.csv
            move_distance (int): user defined distance that moves the bbot. 
            Should be less than 500.
        """
        self.cfg = get_config()
        self.offsets = read_offsets(offset_path)
        self.move_distance = move_distance
        self.mm_acceleration = 50
        self.mm_speed = 80
        self.total_distance = [10000, 10000]  #Total track for each wheel
        self.traversed_distance = 0
        # Init bbot
        self.mm = MachineMotion(DEFAULT_IP)
        self.mm.releaseEstop()
        self.mm.resetSystem()
        time.sleep(1)  # in seconds

    # host type either RPi/Windows
    def configure_machine_motion(self):
        """Configure machine motion to move in reverse using
            DIRECTION.REVERSE
        """
        print("\nConfiguring Axis...")
        # config machine motion
        for axis in self.cfg.WHEEL_MOTORS:
            self.mm.configAxis(axis, MICRO_STEPS.ustep_8,
                               MECH_GAIN.enclosed_timing_belt_self.mm_turn)
            self.mm.configAxisDirection(axis, self.cfg.DIRECTIONS[axis - 2])
        self.mm.emitAcceleration(self.mm_acceleration)
        self.mm.emitSpeed(self.mm_speed)

    # Helper function for getting distance measurements from sensor
    def correct_path(self):
        """Makes small adjustments based on ultrasonic sensors proximity to rail. 
        Makes adjust one wheel at a time. 
        """
        print("\nCorrecting path...")
        corrected_distance = get_distances_rpi(self.cfg, self.offsets)
        if "error" in corrected_distance:
            print(corrected_distance)
            return

        # Getting angle
        ang = find_orientation(self.cfg.ROBOT_LENGTH, corrected_distance)
        # Calculate how much distance motor needs to move to align platform
        d_correction_mm = 2 * np.pi * self.cfg.ROBOT_WIDTH * (abs(ang) /
                                                              360) * 10

        # Create if statement to indicate which motor moves
        if ang > 0.5:
            print(f"\nMoving motor {self.cfg.WHEEL_MOTORS[0]}")
            self.mm.moveRelative(self.cfg.WHEEL_MOTORS[0], d_correction_mm)
            self.mm.waitForMotionCompletion()
        elif ang < -0.5:
            print(f"\nMoving motor {self.cfg.WHEEL_MOTORS[1]}")
            self.mm.moveRelative(self.cfg.WHEEL_MOTORS[1], d_correction_mm)
            self.mm.waitForMotionCompletion()

    def check_desired_position(self):
        # Read the position of several axes
        desiredPositions = self.mm.getDesiredPositions()
        print("\nDesired position of axis 2 is : " + str(desiredPositions[2]) +
              " mm.")
        print("Desired position of axis 3 is : " + str(desiredPositions[3]) +
              " mm.")

    def check_actual_position(self):
        actualPositions = self.mm.getActualPositions()
        print("\nActual position of axis 2 is : " + str(actualPositions[2]) +
              " mm.")
        print("Actual position of axis 3 is : " + str(actualPositions[3]) +
              " mm.")
        return actualPositions[2], actualPositions[3]

    def check_relative_wheel_pos(self):
        ax2_pos, ax3_pos = self.check_actual_position()

        if ax2_pos > ax3_pos + 10 or ax2_pos > abs(ax3_pos - 10):
            print("\nCorrecting path again...")
            self.correct_path()

        if ax3_pos > ax2_pos + 10 or ax3_pos > abs(ax2_pos - 10):
            print("\nCorrecting path again...")
            self.correct_path()

    def stop(self):
        print("\nStopping BenchBot")
        self.mm.triggerEstop()

    def move_wheels(self):
        """Moves both wheels, checks positions, and corrects path. 
        """
        # Move both wheels
        self.mm.moveRelativeCombined(self.cfg.WHEEL_MOTORS,
                                     [self.move_distance, self.move_distance])
        self.mm.waitForMotionCompletion()
        self.correct_path()
        self.mm.waitForMotionCompletion()
        self.check_desired_position()
        self.mm.waitForMotionCompletion()

    def start(self):
        """Main function that controls movement based on distance from start. 
        Also serves as a second check for differences between axis 2 and axis 3.
        """
        # Configure bbot
        self.configure_machine_motion()
        move_check = True
        while move_check:
            print("\nMoving BenchBot...")
            self.move_wheels()
            self.check_relative_wheel_pos()
            act_poss = self.check_actual_position()

            # Stop after user defined travel distance
            if act_poss[0] + 50 > self.move_distance or act_poss[
                    1] + 50 > self.move_distance:
                print(
                    f"\nActual distance ({act_poss[0]} and {act_poss[1]}) exceeds move distance ({self.move_distance} by more than 50. \n Stopping..."
                )
                move_check = False
                self.stop()
                break
        print("\nFinished moving.")


def main():
    parser = argparse.ArgumentParser(description='Move the benchbot back home')
    parser.add_argument(
        '--move',
        default=500,
        help=
        'Intervals inwhich the BenchBot moves before checking and readjusting itself. Better to keep this below 1000.'
    )
    parser.add_argument('--offsets',
                        help="Path to 'SensorOffsets.csv' file",
                        default="SensorOffsets.csv")

    args = vars(parser.parse_args())
    offsets_path = args['offsets']
    travel_dist = args['move']
    print("\nInitializing GoHome...")
    gh = GoHome(offsets_path, travel_dist)
    gh.start()


if __name__ == "__main__":
    main()
