#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"

#include "/home/pi/projectDaredevil/outlier_detection/OutlierDetector.hpp"
#include "/home/pi/projectDaredevil/sound_library/sound_library.h"
#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

const int SOUND_COUNT = 1;
const float calibrationSquareDimension = 0.0251f; //meters
const float arucoSquareDimension = .137f; //meters
const Size chessboardDimensions = Size(9, 6);

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

    OutlierDetector marker_filter[SOUND_COUNT];
	SL_Sound source[SOUND_COUNT];
	SL_Sound arrived;
    int source_map[SOUND_COUNT] = {28};
    bool seen[SOUND_COUNT];

	SL_Init();
    for(int i = 0; i < SOUND_COUNT; i++)
    {
        //marker_filter[i] = OutlierDetector(10);
        marker_filter[i].error_bounds = 0.15;
    	SL_InitSource(&source[i]);
    }
	SL_InitSource(&arrived);
	SL_LoadSound(&source[0],(char *)"mario_jump.wav");
    SL_LoadSound(&arrived,(char *)"mario_coin.wav");
	/*SL_LoadSound(&source[1],(char *)"test.wav");
	SL_LoadSound(&source[2],(char *)"waterfall.wav");*/

	SL_TurnUser(0.0, 0.0, 1.0, 0.0, 1.0, 0.0);

	Vec3f eulerAngles;

	vector<int> markerIds;
	vector<vector<Point2f>> markerCorners, rejectedCandidates;
	aruco::DetectorParameters parameters;

	Ptr < aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);
	VideoCapture vid(0);

	if (!vid.isOpened()) {
		return -1;
	}
	//namedWindow("Webcam", CV_WINDOW_AUTOSIZE);
	vector<Vec3d> rotationVectors, translationVectors;

	while (true) {
		if (!vid.read(frame)) {
			break;
		}

		parameters.cornerRefinementMethod = cv::aruco::CORNER_REFINE_CONTOUR;
		parameters.adaptiveThreshConstant=true;

		aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds);
		aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimension, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors);
        for(int j = 0; j < SOUND_COUNT; j++)
            seen[j] = false;

		for (int i = 0; i < markerIds.size(); i++) {
			aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], arucoSquareDimension); 
            for(int j = 0; j < SOUND_COUNT; j++)
            {
    			if(markerIds[i] == source_map[j])
                {
                    seen[j] = true;
                    marker_filter[j].add(translationVectors[i]);
                    Vec3f pos = marker_filter[j].detect();
				    source[j].x = pos[0];
				    source[j].y = pos[1];
				    source[j].z = pos[2];
				    SL_PlaceSound(&source[j]);
				    SL_PlaySound(&source[j], 1, 0);
			        cout << markerIds[i] << ": ";
			        cout << "\nTranslation X Y Z: ";
			        cout << translationVectors[i] << "\n\n";
                }
            }
		}

        for(int j = 0; j < SOUND_COUNT; j++)
        {
            if(seen[j] == false)
            {
                marker_filter[j].empty();
                if(marker_filter[j].count() == 0)
        		    SL_PlaySound(&source[j], 0, 0);
            }
        }
        if(marker_filter[0].count() > 0)
        {
            double dist = sqrt(source[0].x*source[0].x + source[0].z * source[0].z);
            if(dist <= 0.67)
            {
                SL_PlaySound(&arrived, 1, 0);
            }
            else
            {
                SL_PlaySound(&arrived, 0, 0);
            }
        }

		//imshow("Webcam", frame);
		if (waitKey(30) >= 0) break;
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
