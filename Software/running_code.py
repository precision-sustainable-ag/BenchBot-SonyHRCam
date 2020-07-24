import sys

sys.path.append("..")
from MachineMotion import *
import pyrealsense2 as rs
import time
from datetime import datetime
import numpy as np
import cv2
import threading

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
uStep = MICRO_STEPS.ustep_8
mechGain = MECH_GAIN.ballscrew_10mm_turn
mm.configAxis(axis, uStep, mechGain)
print("Axis " + str(axis) + " configured with " + str(uStep) + " microstepping and " + str(
    mechGain) + "mm/turn mechanical gain")

# Configure the axis direction
ax_direction = DIRECTION.POSITIVE
mm.configAxisDirection(axis, ax_direction)
print("Axis direction set to " + str(ax_direction))

#New variable declaration
# Declare pointcloud object, for calculating pointclouds and texture mappings
pc = rs.pointcloud()
# We want the points object to be persistent so we can display the last cloud when a frame drops
points = rs.points()

### real sense initialization ####

pipeline = rs.pipeline()
config = rs.config()
config.enable_stream(rs.stream.depth, 1280, 720, rs.format.z16, 30)
config.enable_stream(rs.stream.color, 1920, 1080, rs.format.rgb8, 30)
print('Real sense depth and rgb streams initialized')


# Home mm at the beginning of each new rep and treatment

if mm.getCurrentPositions()[2] != 0.0:
    mm.configHomingSpeed(axis, 5)
    mm.emitHome(axis)
    print('mm is going home')
    mm.waitForMotionCompletion()
    print('mm is at home')
    mm.emitStop()
else:
    print('mm is already at home')

# Start the file name system
# The file name it's going to be given by the date, rep, treatment and pot position number. The rep and treatment are
# going to be input by the user a the beginning of a new rep an treatment. The pot number changes automatically.

date = datetime.today().strftime('%Y_%m_%d')
print(f'Today is {date}')

rep = ''
while rep != '1' and rep != '2' and rep != '3' and rep != '4':
    print('Which is yor rep? (enter 1, 2 , 3 or 4)')
    rep = input()

trt = ''
while trt != '0' and trt != '70':
    print('Which is your treatment? (enter 0 or 70)')
    trt = input()

file_name_str = f'{date}_rep{rep}_trt{trt}_'

pot_number = 156  # initialize pot number as 1

path_to_file = 'C:/Users/mlcangia/Desktop/greenhouse_prototype/week_1/'

rng = 1  # initialize range as 1

# an input function so the system waits until we are ready to start
print('Press enter when you are positioned at the beginnig of the range and ready to start the sensing')
input()
print('Are you sure?')
input()
print("Let's get started")

# Relative Move Parameters
speed = 300
acceleration = 100
direction = "negative"  # This will alternate between 'positive' and 'negative' from range to range

# Load Relative Move Parameters
mm.emitSpeed(speed)
mm.emitAcceleration(acceleration)

#### Outer loop begins here ####
# This loop iterates 13 times (= number of ranges). Each iteration starts with mm at Home or at End. The first action is
# to position the camera at the first pot, after that the inner loop starts. At the end of each iteration the program
# stops until the user confirms that is ready to start at a new range.


for r in range(13):
    # change the mm direction. The default direction is 'negative' so mm alwas starts as 'positive' at the 1st range of each
    # treatment.
    if direction == 'positive':
        direction = 'negative'
    else:
        direction = 'positive'

    # relative move to position mm at the first pot from home or end

    pot1_distance = 160  # This is the distance from home or end to the first pot. TBD
    mm.emitRelativeMove(axis, direction, pot1_distance)
    mm.waitForMotionCompletion()
    mm.emitStop()
    print(f'mm is at pot {pot_number}')  # this number has to change going from 1 to 65

    #####################################################


    print('Make sure you are positioned correctly and press enter to continue')
    input()

    #####################################################

    # # Make the first video
    # config.enable_record_to_file(
    #     f'{path_to_file}{file_name_str}{pot_number}.bag')  # the name of the file needs to change at every pot and will go from 1 to 65
    # pipeline.start(config)
    # start = time.time()
    # while time.time() - start < 2:
    #     pipeline.wait_for_frames().keep()
    # pipeline.stop()
    # print(f'video {pot_number} has been recorded and saved')

    # Start streaming with chosen configuration
    pipeline.start(config)

    # We'll use the colorizer to generate texture for our PLY
    # (alternatively, texture can be obtained from color or infrared stream)
    colorizer = rs.colorizer()

    try:
        
        # Wait for the next set of frames from the camera
        frames = pipe.wait_for_frames()
        depth_frame = frames.get_depth_frame()
        color_frame = frames.get_color_frame()

        #if not depth_frame or not color_frame:
        #    continue

        colorized = colorizer.process(frames)

        # Create save_to_ply object
        ply = rs.save_to_ply("ply.ply")

        # Set options to the desired values
        # In this example we'll generate a textual PLY with normals (mesh is already created by default)
        ply.set_option(rs.save_to_ply.option_ply_binary, False)
        ply.set_option(rs.save_to_ply.option_ply_normals, True)

        print("Saving to ply.ply...")
        # Apply the processing block to the frameset which contains the depth frame and the texture
        ply.process(colorized)
        print("Done")

        #time.sleep(5)

        # Convert images to numpy arrays
        depth_image = np.asanyarray(depth_frame.get_data())
        color_image = np.asanyarray(color_frame.get_data())

        # Apply colormap on depth image (image must be converted to 8-bit per pixel first)
        depth_colormap = cv2.applyColorMap(cv2.convertScaleAbs(depth_image, alpha=0.03), cv2.COLORMAP_JET)

        # Stack both images horizontally
        images = np.hstack((color_image, depth_colormap))

        # Saving the image 
        print("Saving to images")
        cv2.imwrite('png.png', color_image) 
        cv2.imwrite('depth.png', depth_colormap) 
        print("Done")

        # Show images
        cv2.namedWindow('RealSense', cv2.WINDOW_AUTOSIZE)
        cv2.imshow('RealSense', images)
        cv2.waitKey(1)
        
        #cv2.waitKey(0)
        #time.sleep(5)    
        
    finally:
        pipeline.stop()



    ##########################################Paula addition####################################
    # file_name_bag = f'{path_to_file}{file_name_str}{pot_number}.bag'
    # file_name = f'{path_to_file}{file_name_str}{pot_number}'
    # subprocess.run(
    #     'cd C:\Program Files(x86)Intel RealSense SDK2.0\tools')  # path where rs-convert is, copy this path on cmd. Your path is different than mine.
    # subprocess.run(
    #     'rs-convert.exe -i file_name_bag -p file_name -l file_name')  # change 20200305_112049.bag by path_file_name_bag, and the others '20200305_112049' by path_file_name without extension
    # # rs-convert -i some.bag -p some_dir/some_file_prefix -r some_another_dir/some_another_file_prefix after -p and -l need to type the location folder not the file with extension.
    #############################################################################################

    pot_number = pot_number + 1
    distance = 240  # This is the distance that the camera has to move from pot to pot starting at pot 1 at each range. TBD

    for p in range(4):
        # Begin Relative Move
        mm.emitRelativeMove(axis, direction, distance)
        mm.waitForMotionCompletion()
        mm.emitStop()
        print(f'mm is at pot {pot_number}')

        #####################################################
        print('Ready to make a video?')
        input()

        #####################################################

        # Start streaming with chosen configuration
        pipeline.start(config)

        # We'll use the colorizer to generate texture for our PLY
        # (alternatively, texture can be obtained from color or infrared stream)
        colorizer = rs.colorizer()

        try:
            
            # Wait for the next set of frames from the camera
            frames = pipe.wait_for_frames()
            depth_frame = frames.get_depth_frame()
            color_frame = frames.get_color_frame()

            #if not depth_frame or not color_frame:
            #    continue

            colorized = colorizer.process(frames)

            # Create save_to_ply object
            ply = rs.save_to_ply("ply.ply")

            # Set options to the desired values
            # In this example we'll generate a textual PLY with normals (mesh is already created by default)
            ply.set_option(rs.save_to_ply.option_ply_binary, False)
            ply.set_option(rs.save_to_ply.option_ply_normals, True)

            print("Saving to ply.ply...")
            # Apply the processing block to the frameset which contains the depth frame and the texture
            ply.process(colorized)
            print("Done")

            #time.sleep(5)

            # Convert images to numpy arrays
            depth_image = np.asanyarray(depth_frame.get_data())
            color_image = np.asanyarray(color_frame.get_data())

            # Apply colormap on depth image (image must be converted to 8-bit per pixel first)
            depth_colormap = cv2.applyColorMap(cv2.convertScaleAbs(depth_image, alpha=0.03), cv2.COLORMAP_JET)

            # Stack both images horizontally
            images = np.hstack((color_image, depth_colormap))

            # Saving the image 
            print("Saving to images")
            cv2.imwrite('png.png', color_image) 
            cv2.imwrite('depth.png', depth_colormap) 
            print("Done")

            # Show images
            cv2.namedWindow('RealSense', cv2.WINDOW_AUTOSIZE)
            cv2.imshow('RealSense', images)
            cv2.waitKey(1)
            
            #cv2.waitKey(0)
            #time.sleep(5)    
            
        finally:
            pipeline.stop()
        ##########################################Paula addition####################################
        # file_name_bag = f'{path_to_file}{file_name_str}{pot_number}.bag'
        # file_name = f'{path_to_file}{file_name_str}{pot_number}'
        # subprocess.run(
        #     'cd C:\Program Files(x86)Intel RealSense SDK2.0\tools')  # path where rs-convert is, copy this path on cmd. Your path is different than mine.
        # subprocess.run(
        #     'rs-convert.exe -i file_name_bag -p file_name -l file_name')  # change 20200305_112049.bag by path_file_name_bag, and the others '20200305_112049' by path_file_name without extension
        # # rs-convert -i some.bag -p some_dir/some_file_prefix -r some_another_dir/some_another_file_prefix after -p and -l need to type the location folder not the file with extension.
        #############################################################################################

        pot_number = pot_number + 1

        #### Inner loop ends here ####

    # after the inner loop ends we need mm to move either to Home or to End. Home if direction == 'negative',
    # End if direction == 'positive'.
    # Here we can use a longer distance, mm is going to stop because of the Home or End sensors
    end_distance = 160
    mm.emitRelativeMove(axis, direction, end_distance)
    mm.waitForMotionCompletion()
    mm.emitStop()

    print(
        f'You have reached the end of Range {rng}. Move the prototype to the next range and press enter when you are ready to start.')
    input()
    rng = rng + 1

    #### Outer loop ends here ####

print(f'You have reached the end of this rep and treatment. See you in the next treatment.')

mm.triggerEstop()





