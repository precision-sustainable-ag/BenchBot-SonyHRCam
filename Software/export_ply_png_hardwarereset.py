## License: Apache 2.0. See LICENSE file in root directory.
## Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#####################################################
##               Export to PNG & PLY               ##
#####################################################

# First import the library
import pyrealsense2 as rs
import numpy as np
import cv2
import time

# Declare pointcloud object, for calculating pointclouds and texture mappings
pc = rs.pointcloud()
# We want the points object to be persistent so we can display the last cloud when a frame drops
points = rs.points()

# Declare RealSense pipeline, encapsulating the actual device and sensors
pipe = rs.pipeline()
config = rs.config()
config.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)
config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)
# Enable depth stream
# config.enable_stream(rs.stream.depth)

# Start streaming with chosen configuration
pipe.start(config)

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

    time.sleep(5)

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
    time.sleep(5)   

    #this code create a virtual unplug and plug for the camera, it should be restart all settings.
    print('Start to initialize the camera:')

    ctx = rs.context()
    print(list(ctx.query_devices()))
    #pipe = rs.pipeline(ctx)

    for dev in ctx.query_devices():
        dev_serial = dev.get_info(rs.camera_info.serial_number)
        dev.hardware_reset()
        print('Camera {} initialized successfully'.format(dev.get_info(rs.camera_info.serial_number)))
    time.sleep(5)

    #continue with pipe.start(config) again

    # pipeline = rs.pipeline()
    # config = rs.config()
    # config.enable_stream(rs.stream.depth, 1280, 720, rs.format.z16, 30)
    # config.enable_stream(rs.stream.color, 1920, 1080, rs.format.rgb8, 30)
    # print('Real sense depth and rgb streams initialized')
############################################################################################### 
    
finally:
    pipe.stop()
