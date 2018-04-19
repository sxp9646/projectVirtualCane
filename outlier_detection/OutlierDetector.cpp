#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include "OutlierDetector.hpp"

using namespace std;

OutlierDetector::OutlierDetector()
{
    error_bounds = 0.10;
    index = 0;
    empty_ticker = 0;
    filled = false;
    angle_mode = false;
}

OutlierDetector::~OutlierDetector()
{
    // You should probably clean something up here.  ./shrug
}

void OutlierDetector::setAngleMode(bool mode)
{
    angle_mode = mode;
}

void OutlierDetector::add(double input)
{
    data[index] = input;
    empty_ticker = 0;
    index++;
    if(index >= 10)
    {
        filled = true;
        index = 0;
    }
}

void OutlierDetector::empty()
{
    empty_ticker++;
    if(empty_ticker > 5)
    {
        filled = false;
        index = 0;
    }
}


bool OutlierDetector::check()
{
    return filled;
}

double OutlierDetector::detect()
{
    int consensus_count[10];
    Vec3f consensus_avg[10];

    int max_consensus = 1;
    int consensus_idx = 0;

    while(max_consensus < 6)
    {

        // Iterate through every element of the array
        for(int i = 0; i < 10; i++)
        {
            Vec3f storage[10];
            storage[0] = data[i];
            
            double error;
            consensus_avg[i] = data[i];
            consensus_count[i] = 1;

            // Forward sweep for indices less than i:
            for(int j = 0; j < i; j++)
            {
                error = computeError(data[i], data[j]);
                if(compareLessThan(error, error_bounds))
                {
                    consensus_count[i]++;
                    storage[consensus_count[i]] = data[j];
                }
            }
            // Backward sweep for indices greater than i:
            for(int j = 9; j > i; j--)
            {
                error = computeError(data[i], data[j]);
                if(compareLessThan(error, error_bounds))
                {
                    consensus_count[i]++;
                    storage[consensus_count[i]] = data[j];
                }                
            }
            if(consensus_count[i] > max_consensus)
            {
                max_consensus = consensus_count[i];
                consensus_idx = i;
            }
            consensus_avg[i] = computeAverage(storage, consensus_count);
            
            /*
            cout << "Index: " << i;
            cout << " Consensus Count: " << consensus_count[i];
            cout << " Consensus Count: " << consensus_avg[i];
            cout << "\n";
            */
        }
        
        error_bounds = error_bounds * 2;
    }
    return consensus_avg[consensus_idx];
}

// Performs the check that A is less than B.
// Will return FALSE if ANY member of A is greater than B.
bool OutlierDetector::compareLessThan(Vec3f A, double B)
{
    bool lessthan = true;
    for(int i = 0; i < 3; i++)
    {
        if(A[i] >= B)
        {
            lessthan = false;
        }
    }
    return lessthan;
}

// Computes the absolute error between 3-pt vector A and B
// Works for angles between pi and -pi when in angle mode
Vec3f OutlierDetector::computeError(Vec3f A, Vec3f B)
{
    Vec3f error = A - B;
    // If running angle mode, correct error computation
    if(angle_mode == true)
    {
        for(int i = 0; i < 3; i++)
        {
            if(error[i] > PI)
            {
                error[i] -= 2*PI;
            }
            else if(error[i] < -PI)
            {
                error[i] += 2*PI;
            }
        }
    }
    return abs(error);
}

Vec3f OutlierDetector::computeAverage(Vec3f input[], int count)
{
    Vec3f average;
    if(angle_mode == false)
    {
        Vec3f sum = {0,0,0};
        for(int i = 0; i < count; i++)
        {
            sum += input[i];
        }
        average = sum / count;
    }
    else if( angle_mode == true)
    {
        Vec3f x_sum = {0,0,0};
        Vec3f y_sum = {0,0,0};
        for(int i = 0; i < count; i++)
        {
            for(int j = 0; j < 3; j++)
            {
                x_sum += cos(input[i][j]);
                y_sum += sin(input[i][j]);
            }
        }
        average = atan2(y_sum, x_sum);
    }
    return average;
}

int main()
{
    
    // Main method needs to be modified
    /*
    OutlierDetector suhail;
    suhail.error_bounds = 7;
    suhail.setAngleMode(true);
	string line;
	ifstream datafile ("test_data.csv");
	if (datafile.is_open())	
	{
		int i = 0;
		while (getline(datafile, line))
		{
            suhail.add(atof(line.c_str()));
            if(suhail.check() == true)
            {
                cout << suhail.detect() << "\n";
                //datafile.close();
                //return 0;
            }
            i++;
		}
		datafile.close();
	}
    return 0;
    */
}