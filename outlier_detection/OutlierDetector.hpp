#include "opencv2/core.hpp"
#define PI (3.14)
#define DATA_SIZE 20
using namespace cv;

class OutlierDetector
{
    public:
        Vec3f data[DATA_SIZE];
        double error_bounds;
        
        OutlierDetector();       // constructor
        ~OutlierDetector();
        
        void setAngleMode(bool mode);
        void add(Vec3f input);
        bool check();
        void empty();
        Vec3f detect();
    private:
        int index;
        int empty_ticker;
        bool filled;
        bool angle_mode;
        
    bool compareLessThan(Vec3f A, double B);
    Vec3f computeError(Vec3f A, Vec3f B);
    Vec3f computeAverage(Vec3f *input, int count);
};
