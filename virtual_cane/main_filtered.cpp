#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"

#include "/home/pi/projectVirtualCane/outlier_detection/OutlierDetector.hpp"
#include "/home/pi/projectVirtualCane/sound_library/sound_library.h"
#include <sstream>
#include <iostream>
#include <fstream>

#define CHAIR_SOUND     "mario_jump.wav"
#define ARRIVAL_SOUND "mario_coin.wav"
using namespace std;
using namespace cv;

const int AVG_SIZE = 25;
const float calibrationSquareDimension = 0.0251f; //meters
const float arucoSquareDimension = .137f; //meters
const Size chessboardDimensions = Size(9, 6);


/*
// This is a worthless pile of garbage that will not help you solve the project problem at all and I strongly recommend you leave this commented out

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
*/

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
	SL_Sound source, arrival_source;
	//initFivePointAverage();

	SL_Init();
	SL_InitSource(&source);
    SL_InitSource(&arrival_source);
	SL_LoadSound(&source,(char *) CHAIR_SOUND);
    SL_LoadSound(&arrival_source, (char *) ARRIVAL_SOUND);
	SL_TurnUser(    0.0, 0.0, 1.0, 
        					    0.0, 1.0, 0.0);

    arrival_source.x = 0;
    arrival_source.y = 0;
    arrival_source.z = 2;
    SL_PlaceSound(&arrival_source);

	// ITS HARD CODED RIGHT NOW.  PLS NO TOUCH
    const int MAX_MARKERS = 24;

    bool valid_marker[MAX_MARKERS]; 
    Mat aTc[MAX_MARKERS];
    OutlierDetector marker_filter[MAX_MARKERS];
    OutlierDetector chair_consensus;
    OutlierDetector chair_filter(10);

    chair_filter.error_bounds = 0.05;
    for(int i = 0; i < MAX_MARKERS; i++)
    {
        // Set marker filter to behave in angle mode and set error bounds to 10º
        marker_filter[i].setAngleMode(true);
        marker_filter[i].error_bounds = 10.0 * PI / 180.0;
        aTc[i] = atcLoad(i);
        if(aTc[i].at<double>(3,0) == 1)
        {
            valid_marker[i] = false;
        }
        else
        {
            valid_marker[i] = true;
        }
    }

	Mat pTa;
	Mat pTc;
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
        
        // Used to figure out which markers were not seen.  Markers seen will mark their respective
        // indices as "-1" as a flag to denote that they were seen and accounted for in the chair position thingee
        int markerSeen[MAX_MARKERS];
        for(int i = 0; i < MAX_MARKERS; i++)
        {
            markerSeen[i] = i;
        }
        for(int i = 0 ; i < 10; i++)
            chair_consensus.empty();

		for (int i = 0; i < markerIds.size(); i++)
        {
			if(markerIds[i] < MAX_MARKERS && valid_marker[markerIds[i]] == true)		
            {
                markerSeen[markerIds[i]] = -1;
                /*
                 * I'm really hoping that performing the angular filter on the rodrigues rotation vector will actually work out
                 * I have absolutely no reason to think this, and it's basically just blind faith at this point
                 * simply because we're running out of options
                 * and time
                 * and willpower
                 * S.B.
                 */
                cv::Mat expected;
                cv::Rodrigues(rotationVectors[i], expected);

                eulerAngles = rotationMatrixToEulerAngles(expected);
                marker_filter[markerIds[i]].add(eulerAngles);
                Vec3f filtered_rotation = marker_filter[markerIds[i]].detect();
                expected = eulerAnglesToRotationMatrix(filtered_rotation);
                cv::Rodrigues(expected, rotationVectors[i]);
                aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], arucoSquareDimension); //0.0235f

			    //expected = fivePointAverage(expected);
			    pTa = (Mat_<double>(4,4) << 	expected.at<double>(0,0), expected.at<double>(0,1), expected.at<double>(0,2), translationVectors[i][0],
										    expected.at<double>(1,0), expected.at<double>(1,1), expected.at<double>(1,2), translationVectors[i][1],
										    expected.at<double>(2,0), expected.at<double>(2,1), expected.at<double>(2,2), translationVectors[i][2],
										    0, 0, 0, 1);
			    pTc = pTa * aTc[markerIds[i]];

                Vec3f chair_pos;
			    chair_pos[0] = pTc.at<double>(0, 3);
			    chair_pos[1] = pTc.at<double>(1, 3);
			    chair_pos[2] = pTc.at<double>(2, 3);

                // Add past chair data to the consensus algorithm
                if(chair_filter.count() > 0)
                {
                    chair_consensus.add(chair_filter.detect());
                }
                chair_consensus.add(chair_pos);

            /*
                cout << markerIds[i] << ": ";
                cout << "Rotation (euler) X Y Z";
                cout << eulerAngles * 180 / 3.14;
                cout << "\nRotation (euler) X Y Z";
                cout << filtered_rotation * 180 / 3.14;
                cout << "\nTranslation X Y Z: ";
                cout << translationVectors[i] << "\n\n";
            */
			}
		}
        // Loop through every marker that was not seen:
        int not_seen = 0;
        for(int i = 0; i < MAX_MARKERS; i++)
        {
            if(markerSeen[i] != -1)
            {
                marker_filter[i].empty();
                not_seen++;
            }
        }

        if(not_seen == MAX_MARKERS)
        {
            chair_filter.empty();
        }
        else
        {
            Vec3f chair_position = chair_consensus.detect();
            chair_filter.add(chair_position);
        }
        if(chair_filter.count() >= 3)
        {
            Vec3f final_chair_pos = chair_filter.detect();
            /*
	        cout << "Chair Offset <X Y Z>: ";
	        cout << final_chair_pos;            
            cout << "\n";
            */
            source.x = final_chair_pos[0];
            source.y = final_chair_pos[1];
            source.z = final_chair_pos[2];
            SL_PlaceSound(&source);
            SL_PlaySound(&source, 1, 0);
            double dist = sqrt(final_chair_pos[0]*final_chair_pos[0] + final_chair_pos[2] * final_chair_pos[2]);
            if(dist <= 0.67)
            {
                SL_PlaySound(&arrival_source, 1, 0);
            }
            else
            {
                SL_PlaySound(&arrival_source, 0, 0);
            }
        }
        else
        {
            SL_PlaySound(&source, 0, 0);
            SL_PlaySound(&arrival_source, 0, 0);
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
