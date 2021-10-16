import cv2
import numpy as np
import argparse
import sys
import time

from ctypes import *
from v4l2framesink import *

parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input-device", required = True)
parser.add_argument("-o", "--output-device", required = True)
parser.add_argument("--width", default = 640, required = False)
parser.add_argument("--height", default = 480, required = False)

args = parser.parse_args()

inputDevice = cv2.VideoCapture(args.input_device)
if not inputDevice.isOpened():
    print(f"Could not open {inputDevice}")
    sys.exit(-1)

sink = FrameSinkHandle()

result = v4l2_openFrameSink(byref(sink), args.output_device, args.width, args.height, FRAMESINK_YUV420)
if result != 0:
    print(f"Error opening {args.output_device}, error was: {result}")
    sys.exit(result)

done = False
while not done:
    ret, frame = inputDevice.read()
    if not ret:
        done = True
        continue
    
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray3 = cv2.merge([gray, gray, gray])
    ret = v4l2_writeFrame(sink, gray3.ctypes.data_as(POINTER(c_uint8)), args.width, args.height, FRAMESINK_BGR24)
    if ret != 0:
        done = True
        continue

v4l2_closeFrameSink(sink)
inputDevice.release()