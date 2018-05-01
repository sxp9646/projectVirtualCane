The Outlier Detector is a custom-built filter made for this project, designed around the idea of looking extreme noise within data and throwing it out.

The OutlierDetector is a C++ class that will store up to a fixed amount of past data, and upon calling "detect()" it will check all the data
for any anomalies, and throw them out.  It will then average all the points that "agreed" with eachother and return the result. This works because
most data tends to fall within very small error bands, except for when the pose angle flips to a different solution, at which point the jump is
large enough that the detector realizes it is an anomaly.  If the detector cannot find a point that more than half the data "agrees" with, it will
double it's error threshold and try again.

Thus, if multiple things need to be filtered, multiple OutlierDetectors need to be created, one for each object that needs filtering.  For example,
all aruco markers have their pose angle filtered, thus there will be an outlier detector for every visible aruco marker.

To test it, run test_detector and it will open out test_data_vec3f and it will filter out the "outliers" of test_data_vec3f.csv