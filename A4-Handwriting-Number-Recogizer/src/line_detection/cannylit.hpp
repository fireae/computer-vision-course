#ifndef CANNY_HPP
#define CANNY_HPP
#include<string>
#include<string.h>
#include <stdlib.h>
#include <math.h>
#include<CImg.h>
using namespace std;
using namespace cimg_library;


#define ffabs(x) ( (x) >= 0 ? (x) : -(x) ) 
#define GAUSSIAN_CUT_OFF 0.005f
#define MAGNITUDE_SCALE 100.0f
#define MAGNITUDE_LIMIT 1000.0f
#define MAGNITUDE_MAX ((int) (MAGNITUDE_SCALE * MAGNITUDE_LIMIT))


class cannylit {
    private:
        float lowThreshold;
        float highThreshold;
		float gaussianKernelRadius;
        int gaussianKernelWidth;

        int high;             /* High threshold. */
        int low;              /* Low threshold. */
        int width;
        int height;

        unsigned char* data;    /* input of image data */
        int *magnitude;
        int *idata;          /* output for edges */
        unsigned char *outdata;          /* output for edges */
        float *xConv;        /* temporary for convolution in x direction */
        float *yConv;        /* temporary for convolution in y direction */
        float *xGradient;    /* gradients in x direction, as detected by DoG */
        float *yGradient;    /* gradients in x direction,a s detected by DoG */

        void cannyProcess();            // Make canny process on image
        void DoG_filter();              // Filter DoG that make convolution with image.
        void HysteresisThreshold();     // Hysteresis Threshold
      
        void follow(int x1, int y1, int i1, int threshold);  /* recursive portion of edge follower */
        void allocatebuffers(unsigned char *data);     /* buffer allocation */
        void killbuffers();     /* buffers destructor */\
        float hypotenuse(float x, float y);     /* x^2+y^2 */
        float gaussian(float x, float sigma);   /* Gaussian function */
    public:
        // Default: 2.5f, 7.5f, 2.0f, 16, 0
        cannylit(unsigned char* grayData, int width, int height, 
            float lowThreshold=2.5f, float highThreshold=7.5f, 
			float gaussianKernelRadius=2.0f, int gaussianKernelWidth=16);
        ~cannylit();
	
        // Get data buffer pointer of edgeImage.
        unsigned char* edgeData();
        int getWidth();
        int getHeight();
        void display();

        // Reset parameters and make canny process again.
        void resetParams(float lowThreshold=2.5f, float highThreshold=7.5f, 
			float gaussianKernelRadius=2.0f, int gaussianKernelWidth=16); 
};


#endif
