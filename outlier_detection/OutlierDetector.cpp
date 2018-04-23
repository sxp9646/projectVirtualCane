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

void OutlierDetector::add(Vec3f input)
{
    data[index] = input;
    empty_ticker = 0;
    index++;
    if(index >= DATA_SIZE)
    {
        filled = true;
        index = 0;
    }
}

void OutlierDetector::empty()
{
    if(empty_ticker >= 8)
    {
        filled = false;
        index = 0;
    }
    else
    {
        empty_ticker++;
    }
}


bool OutlierDetector::check()
{
    return filled;
}

int OutlierDetector::count()
{
    if(filled == true)
    {
        return DATA_SIZE;
    }
    else
    {
        return index;
    }
}

Vec3f OutlierDetector::detect()
{
    int SIZE;
    if(filled == true)
    {
        SIZE = DATA_SIZE;
    }
    else if(index > 1)
    {
        SIZE = index;
    }
    else if(index == 1)
    {
        return data[0];
    }
    else
    {
        return {0,0,0};
    }
    double bounds_temp = error_bounds;
    int consensus_count[SIZE];
    Vec3f consensus_avg[SIZE];

    int max_consensus = 1;
    int consensus_idx = 0;

    while(max_consensus <= (SIZE / 2))
    {

        // Iterate through every element of the array
        for(int i = 0; i < SIZE; i++)
        {
            Vec3f storage[SIZE];
            storage[0] = data[i];
            
            Vec3f error;
            //consensus_avg[i] = data[i];
            consensus_count[i] = 1;

            // Forward sweep for indices less than i:
            for(int j = 0; j < i; j++)
            {
                error = computeError(data[i], data[j]);
                if(compareLessThan(error, bounds_temp))
                {
                    storage[consensus_count[i]] = data[j];
                    consensus_count[i]++;
                }
            }
            // Backward sweep for indices greater than i:
            for(int j = (SIZE - 1); j > i; j--)
            {
                error = computeError(data[i], data[j]);
                if(compareLessThan(error, bounds_temp))
                {
                    storage[consensus_count[i]] = data[j];
                    consensus_count[i]++;
                }                
            }
            if(consensus_count[i] > max_consensus)
            {
                max_consensus = consensus_count[i];
                consensus_idx = i;
            }
            consensus_avg[i] = computeAverage(storage, consensus_count[i]);
        }
        
        bounds_temp = bounds_temp * 2;
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
    if(angle_mode == false)
    {
	for(int i = 0; i < 3; i++)
	{
            error[i] = abs(error[i]);
	}
    }
    else if(angle_mode == true)
    {
        for(int i = 0; i < 3; i++)
        {
	    error[i] = acos(cos(A[i]) * cos(B[i]) + sin(A[i]) * sin(B[i]));
        }
    }
    return error;
}

Vec3f OutlierDetector::computeAverage(Vec3f *input, int count)
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
                x_sum[j] += cos(input[i][j]);
                y_sum[j] += sin(input[i][j]);
            }
        }
	for(int j = 0; j < 3; j++)
	{
            average[j] = atan2(y_sum[j], x_sum[j]);
	}
    }
    return average;
}

/*
int main()
{
    
	// Main method needs to be modified

	OutlierDetector suhail;
	suhail.error_bounds = 10.0 * PI / 180.0;
	suhail.setAngleMode(true);
	string line;
	const char *delim = ",";

	ifstream datafile ("test_data_vec3f.csv");
	if (datafile.is_open())	
	{
		int i = 0;
		while (getline(datafile, line))
		{
			Vec3f data_in;
			// Split string
			// to avoid modifying original string
			// first duplicate the original string and return a char pointer then free the memory
			char * dup = strdup(line.c_str());
			char * token = strtok(dup, delim);
			int j = 0;
			while(token != NULL)
			{
				data_in[j] = atof(token) * PI / 180;
				token = strtok(NULL, delim);
				j++;
			}
			cout << (data_in * 180 / PI)<< "\t\t\t";
			suhail.add(data_in);
			//if(suhail.check() == true)
			//{
				cout << suhail.detect() * 180 / PI;
			//}
			cout << "\n";
			i++;
		}
		datafile.close();
	}
	return 0;
}*/
