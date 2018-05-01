#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include "OutlierDetector.hpp"

using namespace std;


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
}