#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"

#include "/home/pi/projectDaredevil/outlier_detection/OutlierDetector.hpp"
#include "sound_library.h"
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

const int AVG_SIZE = 25;
const float calibrationSquareDimension = 0.0251f; //meters
const float arucoSquareDimension = .137f; //meters
const Size chessboardDimensions = Size(9, 6);


Mat averager[AVG_SIZE];
int cycle = 0;

void initFivePointAverage(){
	Mat zero_matrix = Mat(3,3, CV_64F, double(0));
	for(int i = 0; i < AVG_SIZE; i++)
	{
		averager[i] = zero_matrix;
	}
}
Mat fivePointAverage(Mat input){
	averager[cycle] = input;
	
	cycle++;
	if(cycle >= AVG_SIZE){
		cycle = 0;
	}
	Mat sum = averager[0];
	for(int i = 1; i < AVG_SIZE; i++)
	{
		sum = sum + averager[i];
	}
	sum = sum / AVG_SIZE;
	return sum;
}

void atcWrite(int markerId, Mat atc)
{
	ofstream arucoFile;
	arucoFile.open ("aruco" + std::to_string(markerId) +".txt");
	for(int i = 0; i < atc.rows; i++)
	{
		for(int j = 0; j < atc.cols; j++)
		{
			arucoFile << atc.at<double>(i,j);
			if(j < (atc.cols - 1))
			{
				arucoFile << ",";
			}
		}
		if(i < (atc.rows - 1))
		{
			arucoFile << "\n";
		}
	}
	arucoFile.close();
}
// Load the Aruco To Chair matrix from the associated file
Mat atcLoad(int markerId)
{
	Mat result = Mat(4,4, CV_64F, double(1));;
	string line;
	const char *delim = ",";
	// Open file "arucoX.txt" which has data for how aruco X can get to chair
	ifstream arucoFile ("aruco" + std::to_string(markerId) + ".txt");
	if (arucoFile.is_open())	
	{
		int i = 0;
		while (getline(arucoFile, line))
		{
			// Split string
			// to avoid modifying original string
			// first duplicate the original string and return a char pointer then free the memory
			char * dup = strdup(line.c_str());
			char * token = strtok(dup, delim);
			int j = 0;
			while(token != NULL)
			{
				result.at<double>(i,j) = atof(token);
				token = strtok(NULL, delim);
				j++;
			}
			i++;
		}
		arucoFile.close();
	}
	else
	{
		return result;
	}
}

// This code was definitely not taken from https://www.learnopencv.com/rotation-matrix-to-euler-angles/
// Calculates rotation matrix given euler angles.
Mat eulerAnglesToRotationMatrix(Vec3f &theta)
{
    // Calculate rotation about x axis
    Mat R_x = (Mat_<double>(3,3) <<
               1,       0,              0,
               0,       cos(theta[0]),   -sin(theta[0]),
               0,       sin(theta[0]),   cos(theta[0])
               );
     
    // Calculate rotation about y axis
    Mat R_y = (Mat_<double>(3,3) <<
               cos(theta[1]),    0,      sin(theta[1]),
               0,               1,      0,
               -sin(theta[1]),   0,      cos(theta[1])
               );
     
    // Calculate rotation about z axis
    Mat R_z = (Mat_<double>(3,3) <<
               cos(theta[2]),    -sin(theta[2]),      0,
               sin(theta[2]),    cos(theta[2]),       0,
               0,               0,                  1);
     
     
    // Combined rotation matrix
    Mat R = R_z * R_y * R_x;
     
    return R;
 
}

// Calculates rotation matrix to euler angles
// The result is the same as MATLAB except the order
// of the euler angles ( x and z are swapped ).
Vec3f rotationMatrixToEulerAngles(Mat &R)
{    
    float sy = sqrt(R.at<double>(0,0) * R.at<double>(0,0) +  R.at<double>(1,0) * R.at<double>(1,0) );
 
    bool singular = sy < 1e-6; // If
 
    float x, y, z;
    if (!singular)
    {
        x = atan2(R.at<double>(2,1) , R.at<double>(2,2));
        y = atan2(-R.at<double>(2,0), sy);
        z = atan2(R.at<double>(1,0), R.at<double>(0,0));
    }
    else
    {
        x = atan2(-R.at<double>(1,2), R.at<double>(1,1));
        y = atan2(-R.at<double>(2,0), sy);
        z = 0;
    }
    return Vec3f(x, y, z);     
}

void createArucoMarkers() {
	Mat outputMarker;

	Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);

	for (int i = 0; i < 50; i++) {
		aruco::drawMarker(markerDictionary, i, 500, outputMarker, 1);
		ostringstream convert;
		string imageName = "4x4Marker_";
		convert << imageName << i << ".jpg";
		imwrite(convert.str(), outputMarker);
	}
}

void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>&corners) {
	for (int i = 0; i < boardSize.height; i++) {
		for (int j = 0; j < boardSize.width; j++) {
			corners.push_back(Point3f(j*squareEdgeLength, i*squareEdgeLength, 0.0f));
		}
	}
}

void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool showResults = false) {
	for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++) {
		vector<Point2f> pointBuf;
		bool found = findChessboardCorners(*iter, Size(9,6), pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);

		if (found) {
			allFoundCorners.push_back(pointBuf);
		}

		if (showResults) {
			drawChessboardCorners(*iter, Size(9,6), pointBuf, found);
			imshow("Looking for corners", *iter);
			waitKey(0);
		}
	}
}

int startWebcamMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions){
	Mat frame;

	//Mat atc_inv = aTc.t() * aTc;
	//atc_inv = atc_inv.inv() * aTc.t();

    const int MAX_MARKERS = 50;
    Mat aTc[MAX_MARKERS];
    OutlierDetector marker_filter[MAX_MARKERS];
	Vec3f eulerAngles;

	vector<int> markerIds;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	aruco::DetectorParameters parameters;

	Ptr < aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
	VideoCapture vid(0);

	if (!vid.isOpened()) {
		return -1;
	}
	namedWindow("Webcam", CV_WINDOW_AUTOSIZE);
	vector<Vec3d> rotationVectors, translationVectors;

	while (true) {
		if (!vid.read(frame)) {
			break;
		}
		aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds);
		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimension, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors);
        int markerSeen[MAX_MARKERS];
        for(int i = 0; i < MAX_MARKERS; i++)
        {
            markerSeen[i] = i;
        }
		for (int i = 0; i < markerIds.size(); i++) {
			aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], arucoSquareDimension); //0.0235f

			cout << markerIds[i] << ": ";
			cv::Mat expected;

			// Would like to see exactly what the output rotation matrix is here.
			cv::Rodrigues(rotationVectors[i], expected);

			// Retard route: Convert the whole rotation matrix into euler angles, and then convert back into a known rotation matrix (around y? I guess?)
            eulerAngles = rotationMatrixToEulerAngles(expected);
            marker_filter[markerIds[i]].add(eulerAngles);
            Vec3f filtered_rotation = marker_filter[markerIds[i]].detect();
            expected = eulerAnglesToRotationMatrix(filtered_rotation);

			// This matrix may not be the correct representation of the kinematics matrix that we were assuming we had when we (josh) did the math
			Mat cTa = (Mat_<double>(4,4) << 	expected.at<double>(0,0), expected.at<double>(0,1), expected.at<double>(0,2), translationVectors[i][0],
										expected.at<double>(1,0), expected.at<double>(1,1), expected.at<double>(1,2), translationVectors[i][1],
										expected.at<double>(2,0), expected.at<double>(2,1), expected.at<double>(2,2), translationVectors[i][2],
										0, 0, 0, 1);
            aTc[markerIds[i]] = cTa.inv();
		
			cout << "\n";

			eulerAngles = rotationMatrixToEulerAngles(expected) * 180 / 3.14;

            /*
			cout << "Rotation X Y Z";
			cout << eulerAngles;
			cout << "\nTranslation X Y Z: ";
			cout << translationVectors[i] << "\n\n";
            */
		}
        // Loop through every marker that was not seen:
        for(int i = 0; i < MAX_MARKERS; i++)
        {
            if(markerSeen[i] != -1)
            {
                marker_filter[i].empty();
            }
        }
		imshow("Webcam", frame);
		char character = waitKey(1000 / 20);

		switch (character) {
		case ' ':
    		for (int i = 0; i < markerIds.size(); i++) {
                if(marker_filter[markerIds[i]].check() == true)
                {
        			atcWrite(markerIds[i], aTc[markerIds[i]]);
                    cout << "Saved Aruco[" << markerIds[i] << "].\n";
                }
            }
            break;
		// Return character
		case 13:
			//exit
			return 0;
			break;
		// Escape character
		case 27:
			//exit
			return 0;
			break;
		}

		//if (waitKey(30) >= 0) break;
	}
	return 1;
}

void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCoefficients) {
	vector<vector<Point2f>> checkerboardImageSpacePoints;
	getChessboardCorners(calibrationImages, checkerboardImageSpacePoints, false);

	vector<vector<Point3f>> worldSpaceCornerPoints(1);

	createKnownBoardPosition(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);
	worldSpaceCornerPoints.resize(checkerboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

	vector<Mat> rVectors, tVectors;
	distanceCoefficients = Mat::zeros(8, 1, CV_64F);

	calibrateCamera(worldSpaceCornerPoints, checkerboardImageSpacePoints, boardSize, cameraMatrix, distanceCoefficients, rVectors, tVectors);
}

bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distanceCoefficients) {
	ofstream outStream(name);
	if (outStream) {
		uint16_t rows = cameraMatrix.rows;
		uint16_t columns = cameraMatrix.cols;

		outStream << rows << endl;
		outStream << columns << endl;

		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				double value = cameraMatrix.at<double>(r, c);
				outStream << value << endl;
			}
		}

		rows = distanceCoefficients.rows;
		columns = distanceCoefficients.cols;

		outStream << rows << endl;
		outStream << columns << endl;

		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				double value = distanceCoefficients.at<double>(r, c);
				outStream << value << endl;
			}
		}

		outStream.close();
		return true;
	}

	return false;
}

bool loadCameraCalibration(string name, Mat& cameraMatrix, Mat& distanceCoefficients) {
	ifstream inStream(name);
	if (inStream) {
		uint16_t rows;
		uint16_t columns;

		inStream >> rows;
		inStream >> columns;

		cameraMatrix = Mat(Size(columns, rows), CV_64F);

		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				double read = 0.0f;
				inStream >> read;
				cameraMatrix.at<double>(r, c) = read;
				cout << cameraMatrix.at<double>(r, c) << "\n";

			}
		}

		//Distance(distortion) coefficients
		inStream >> rows;
		inStream >> columns;
		

		distanceCoefficients = Mat::zeros(rows, columns, CV_64F);

		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < columns; c++) {
				double read = 0.0f;
				inStream >> read;
				distanceCoefficients.at<double>(r, c) = read;
				cout << distanceCoefficients.at<double>(r, c) << "\n";
			}
		}
		inStream.close();
		return true;
	}
	return false;
}

void cameraCalibrationProcess(Mat& cameraMatrix, Mat& distanceCoefficients) {
	//createArucoMarkers(); //Used to generate aruco markers

	Mat frame;
	Mat drawToFrame;

	vector<Mat> savedImages;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;

	VideoCapture vid(0);
	if (!vid.isOpened()) {
		return;
	}

	int framesPerSecond = 20;
	namedWindow("Webcam", CV_WINDOW_AUTOSIZE);

	while (true) {
		if (!vid.read(frame)) {
			break;
		}

		vector<Vec2f> foundPoints;
		bool found = false;

		found = findChessboardCorners(frame, chessboardDimensions, foundPoints, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);
		frame.copyTo(drawToFrame);
		drawChessboardCorners(drawToFrame, chessboardDimensions, foundPoints, found);
		if (found)
			imshow("Webcam", drawToFrame);
		else
			imshow("Webcam", frame);
		char character = waitKey(1000 / framesPerSecond);

		switch (character) {
		case ' ':
			//saving image
			if (found) {
				Mat temp;
				frame.copyTo(temp);
				savedImages.push_back(temp);
				cout << "Image Saved: ";
				cout << savedImages.size();
				cout << "\n";

			}
			break;
		case 13:
			//start calib
			if (savedImages.size() > 15) {
				cameraCalibration(savedImages, chessboardDimensions, calibrationSquareDimension, cameraMatrix, distanceCoefficients);
				saveCameraCalibration("Camera Calibration", cameraMatrix, distanceCoefficients);
				cout << "we made it fam 'dab' \n";
			}
			break;
		case 27:
			//exit
			return;
			break;
		}
	}
}

int main(int argv, char** argc) {

	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	Mat distanceCoefficients;


	//cameraCalibrationProcess(cameraMatrix, distanceCoefficients);
	loadCameraCalibration("Camera Calibration", cameraMatrix, distanceCoefficients);
	startWebcamMonitoring(cameraMatrix, distanceCoefficients, 0.099f);
	

	return 0;
}
