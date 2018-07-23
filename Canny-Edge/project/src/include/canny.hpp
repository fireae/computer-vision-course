#ifndef CANNY_HPP
#define CANNY_HPP
#include "../lib/CImg.h"
#include<string>
#include<string.h>
#include <stdlib.h>
#include <math.h>
using namespace cimg_library;
using namespace std;


#define ffabs(x) ( (x) >= 0 ? (x) : -(x) ) 
#define GAUSSIAN_CUT_OFF 0.005f
#define MAGNITUDE_SCALE 100.0f
#define MAGNITUDE_LIMIT 1000.0f
#define MAGNITUDE_MAX ((int) (MAGNITUDE_SCALE * MAGNITUDE_LIMIT))


class canny {
    private:
        string imgPath;
        CImg<unsigned char> rgbImg;    /* Input rgb image */
        CImg<unsigned char> grayImg;    /* Input gray image */
        CImg<float> magImg;     /* Magnitude image processed by DoG */
        CImg<float> onePixelImg; /* Singe Pixel image processed by non-maximal suppression */
        CImg<unsigned char> edgeImg;    /* Output edge image processed by Hysteresis threshold */

        float lowThreshold;
        float highThreshold;
		float gaussianKernelRadius;
        int gaussianKernelWidth;
		int contrastNormalised;

        int high;             /* High threshold. */
        int low;              /* Low threshold. */
        int width;
        int height;

        int *idata;          /* output for edges */
        float *xConv;        /* temporary for convolution in x direction */
        float *yConv;        /* temporary for convolution in y direction */
        float *xGradient;    /* gradients in x direction, as detected by DoG */
        float *yGradient;    /* gradients in x direction,a s detected by DoG */

        void cannyProcess();            // Make canny process on image
        void normalizeContrast();       // Make narmalize
        void DoG_filter();              // Filter DoG that make convolution with image.
        void NMS();                     // Non-maximum suppression
        void HysteresisThreshold();     // Hysteresis Threshold
      
        void follow(float *onePixel, int x1, int y1, int i1, int threshold);  /* recursive portion of edge follower */
        void allocatebuffers();     /* buffer allocation */
        void killbuffers();     /* buffers destructor */

        CImg<unsigned char> rgb2gray(CImg<unsigned char> &img);
        float hypotenuse(float x, float y);     /* x^2+y^2 */
        float gaussian(float x, float sigma);   /* Gaussian function */
    public:
        // Default: 2.5f, 7.5f, 2.0f, 16, 0
        canny(const string imgPath, 
            float lowThreshold=2.5f, float highThreshold=7.5f, 
			float gaussianKernelRadius=2.0f, int gaussianKernelWidth=16,  
			int contrastNormalised=0);
        ~ canny();

        void displayRgb();
        void displayGray();
        void displayMagnitude();
        void displayOnePixel();
        void displayEdge();
        void displayAll();
        
        // Reset parameters and make canny process again.
        void resetParams(float lowThreshold=2.5f, float highThreshold=7.5f, 
			float gaussianKernelRadius=2.0f, int gaussianKernelWidth=16,  
			int contrastNormalised=0); 

        // Save output image.
        void save(string filePath, string name, string option="edge");
};


#endif