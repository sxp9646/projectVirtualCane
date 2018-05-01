This code is the "main method" of the project, split into three concepts:

pre_demo is a preliminary demo where sounds are placed directly on aruco markers

capture_chair is a capturing program which will record an aruco marker's relationship to the camera's current position:
1: Use capture_chair while standing at the position of the object of interest.  
2: Once the pose angle of the aruco marker is visually correct, then press spacebar to capture the "aruco-to-chair" matrix
3: Repeat for all markers that are visible from the current position
4: This will spit out "arucoX.txt" files, do not touch these unless you want to re-capture the chair location!

main_filtered, which compiles into virtual_cane, is the final demo of the project, where looking at an aruco marker will 
inform the user how to get to the object of interest (which was captured while running capture_chair), and it will place
a sound at the location of that object.

These libraries require the compilation of sound_library and outlier_detector.


