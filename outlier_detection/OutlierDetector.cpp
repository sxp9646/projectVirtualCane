#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

class OutlierDetector
{
    public:
        double data[10];
        
        OutlierDetector();       // constructor
        ~OutlierDetector();
        
        void add(double input);
        bool check();
        void empty();
        double detect();
 private:
        int index;
        int empty_ticker;
        bool filled;
};

OutlierDetector::OutlierDetector()
{
    index = 0;
    empty_ticker = 0;
    filled = false;
}

OutlierDetector::~OutlierDetector()
{
    // You should probably clean something up here.  ./shrug
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
    if(empty_ticker > 10)
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
    // 7 degrees
    double error_bounds = 7;
    
    int consensus_count[10];
    double consensus_avg[10];
    
    bool consensus = false;

    int max_consensus = 1;
    int consensus_idx = 0;

    while(max_consensus < 6)
    {

        // Iterate through every element of the array
        for(int i = 0; i < 10; i++)
        {
            double error;
            consensus_avg[i] = data[i];
            consensus_count[i] = 1;

            // Forward sweep for indices less than i:
            for(int j = 0; j < i; j++)
            {
                error = abs(data[i] - data[j]);
                if(error < error_bounds)
                {
                    consensus_avg[i] += data[j];
                    consensus_count[i]++;
                }
            }
            // Backward sweep for indices greater than i:
            for(int j = 9; j > i; j--)
            {
                error = abs(data[i] - data[j]);
                if(error < error_bounds)
                {
                    consensus_avg[i] += data[j];
                    consensus_count[i]++;
                }                
            }
            if(consensus_count[i] > max_consensus)
            {
                max_consensus = consensus_count[i];
                consensus_idx = i;
            }
            consensus_avg[i] /= consensus_count[i];
            
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


int main()
{
    OutlierDetector suhail;
    
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
}