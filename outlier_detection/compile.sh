#!/bin/bash
g++ OutlierDetector.cpp -lopencv_highgui -lopencv_core -lopencv_videoio -lopencv_imgcodecs -lopencv_aruco -lopencv_calib3d -c
g++ -o test_detector test_detector.cpp -lopencv_highgui -lopencv_core -lopencv_videoio -lopencv_imgcodecs -lopencv_aruco -lopencv_calib3d OutlierDetector.o
