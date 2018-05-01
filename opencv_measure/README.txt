This file exists to help measure physical properties of the project.

data_gather will look for exactly one aruco marker (currently hard-coded to 2), and dump out it's translation data 
(in meters) and pose angle (in euler angle degrees) to a file upon pressing spacebar.  when spacebar is pressed again
the capture stream will stop and two newlines will be added to the file (that way, all samples associated with one test
are grouped together).

Similarly, chair_gather and chair_gather_filtered will attempt to record the data from the aruco marker, but this time
only after computing the full math between the person and the chair, thus it will dump out the translation vector
that will associate the "person" (AKA the camera) to the "chair".

WARNING: IN ORDER FOR CHAIR_GATHER PROGRAMS TO WORK, YOU MUST HAVE "CAPTURED THE CHAIR LOCATION" FIRST, SO COPY ALL "aruco*.txt" FILES
OVER FROM THE VIRTUAL CANE FOLDER INTO THE MEASUREMENT FOLDER.