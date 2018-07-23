#ifndef WARP_IMAGE_HPP
#define WARP_IMAGE_HPP
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/xfeatures2d.hpp> //Thanks to Alessandro
#include <opencv2/flann.hpp>
#include <iostream>
#include <CImg.h>
#include <cmath>
using namespace cv;
using namespace cimg_library;


int R = 780;
// interpolate for warped image
double linear_atXYZ(CImg<unsigned char> &img, double x, double y, double c, double default_=0) {
    if (x < 0 || x+1 > img.width() || y < 0 || y+1 > img.height()) return default_;

	double a = x - floor(x);
	double b = y - floor(y);
	double nw, ne, sw, se;
    nw = img(floor(x), floor(y), 0, c);
    ne = img(ceil(x), floor(y), 0, c);
    sw = img(floor(x), ceil(y), 0, c);
    se = img(ceil(x), ceil(y), 0, c);

    // if ((nw == 0 && ne == 0) 
    //     || (sw == 0 && se == 0)
    //     || (nw == 0 && sw == 0)
    //     || (ne == 0 && se == 0))
    if (nw == 0 || se == 0 || ne == 0 || sw == 0)
        return default_;

    return (1-a)*(1-b)*nw + a*(1-b)*sw + (1-a)*b*ne + a*b*se;
}
double interpolate(Mat &img, double x, double y, int c) {
    if (x < 0 || x+1 > img.cols || y < 0 || y+1 > img.rows) return 0;

	double a = x - floor(x);
	double b = y - floor(y);
	double nw, ne, sw, se;
    nw = img.at<Vec3b>(floor(y), floor(x))[c];
    ne = img.at<Vec3b>(floor(y), ceil(x))[c];
    sw = img.at<Vec3b>(ceil(y), floor(x))[c];
    se = img.at<Vec3b>(ceil(y), ceil(x))[c];

    return (1-a)*(1-b)*nw + a*(1-b)*sw + (1-a)*b*ne + a*b*se;
}

void warp_image(Mat &img) {
    Mat img_copied = img.clone();
    int width = img.cols;
    int height = img.rows;

    for (int wnum = 0; wnum < width; wnum++) {
        for (int hnum = 0; hnum < height; hnum++) {
            double k = R / sqrt(R*R + (wnum- width / 2) * (wnum - width / 2));  
            double x = (wnum - width / 2) / k + width / 2;  
            double y = (hnum - height / 2) / k + height / 2; 

            img.at<Vec3b>(hnum, wnum)[0] = interpolate(img_copied, x, y, 0);
            img.at<Vec3b>(hnum, wnum)[1] = interpolate(img_copied, x, y, 1);
            img.at<Vec3b>(hnum, wnum)[2] = interpolate(img_copied, x, y, 2);
        }
    }
}

void warp_image(CImg<unsigned char> &img) {
    CImg<unsigned char> img_copied = CImg<unsigned char>(img);
    int width = img.width();
    int height = img.height();

    for (int wnum = 0; wnum < img.width(); wnum++) {
        for (int hnum = 0; hnum < img.height(); hnum++) {
            double k = R / sqrt(R*R + (wnum- width / 2) * (wnum - width / 2));  
            double x = (wnum - width / 2) / k + width / 2;  
            double y = (hnum - height / 2) / k + height / 2; 
            img(wnum, hnum, 0, 0) = linear_atXYZ(img_copied, x, y, 0, 0);
            img(wnum, hnum, 0, 1) = linear_atXYZ(img_copied, x, y, 1, 0);
            img(wnum, hnum, 0, 2) = linear_atXYZ(img_copied, x, y, 2, 0);
        }
    }
}
#endif