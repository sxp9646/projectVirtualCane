#!/bin/bash
#Here's how to compile the code in this folder:

g++ -o data_gather data_gather.cpp -lopencv_highgui -lopencv_core -lopencv_videoio -lopencv_imgcodecs -lopencv_aruco -lopencv_calib3d

g++ -o chair_gather chair_gather.cpp -lopencv_highgui -lopencv_core -lopencv_videoio -lopencv_imgcodecs -lopencv_aruco -lopencv_calib3d

g++ -o chair_gather_filtered chair_gather_filtered.cpp -lopencv_highgui -lopencv_core -lopencv_videoio -lopencv_imgcodecs -lopencv_aruco -lopencv_calib3d ../outlier_detection/OutlierDetector.o