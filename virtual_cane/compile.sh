#!/bin/bash
#Here's how to compile the main virtual cane method:
#Virtual cane is the core project, and assumes you've captured the aruco marker information
g++ -o virtual_cane main_filtered.cpp -lopencv_highgui -lopencv_core -lopencv_videoio -lopencv_imgcodecs -lopencv_aruco -lopencv_calib3d ../outlier_detection/OutlierDetector.o ../sound_library/libopenal.so ../sound_library/sound_library.o

#Capture aruco will save arucos relationship to an object of interest
g++ -o capture_chair capture_chair.cpp -lopencv_highgui -lopencv_core -lopencv_videoio -lopencv_imgcodecs -lopencv_aruco -lopencv_calib3d ../outlier_detection/OutlierDetector.o

#pre demo is the preliminary demo that places sounds directly on specified aruco markers
g++ -o pre_demo pre_demo.cpp -lopencv_highgui -lopencv_core -lopencv_videoio -lopencv_imgcodecs -lopencv_aruco -lopencv_calib3d ../outlier_detection/OutlierDetector.o ../sound_library/libopenal.so ../sound_library/sound_library.o
