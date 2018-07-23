#ifndef OSTU_HPP
#define OSTU_HPP
#define cimg_use_jpeg
#include "CImg.h"
#include<cstring>
#include<cmath>
#include<ctime>
#include<vector>
using namespace std;
using namespace cimg_library;

class Ostu {
    private:
        unsigned long histogram[256];
        unsigned long pdf[256];
    public:
        vector<double> segmentation(const char* inPath, bool show=false, const char* outPath = NULL, bool save=false) {
            clock_t start, end;
            vector<double> costTime;
            // 分割图
            start = clock();
            CImg<unsigned char> img(inPath);
            end = clock();
            costTime.push_back((end-start));

            start = clock();
            CImg<unsigned char> gimg(img.width(), img.height());
            toGray(img, gimg); // 获取灰度图
            updateHistogram(gimg);  // 更新灰度直方图
            // 遍历计算类间方差
            double maxSigma = -1;
            int T = -1;
            for (int p = 0; p < 255; p++) {
                double sigma = sigmaOf(p);
                if (sigma > maxSigma) {
                    maxSigma = sigma;
                    T = p;
                }
            }
            end = clock();
            costTime.push_back((end-start));

            start = clock();
            // 绘制分割图像
            for (int p = 0; p < gimg.size(); p++) {
                if (gimg[p] >= T) gimg[p] = 255;
                else gimg[p] = 0;
            }
            if (save) {
                // 保存分割图像
                gimg.save(outPath);
            }
            if (show) {
                // 展示分割图像
                gimg.display();
            }
            end = clock();
            costTime.push_back((end-start));
            return costTime;
        }
        double sigmaOf(int T) {
            double m1 = 0, m2 = 0, mG = 0;
            unsigned long sum1 = pdf[T];
            unsigned long sum2 = pdf[255]-sum1;
            unsigned long sumG = pdf[255];
            double P1 = (double) pdf[T] / pdf[255];
            double P2 = 1 - P1;

            for (int p = 0; p < T; p++) {
                m1 += (double)histogram[p]/sum1 * p;
                mG += (double)histogram[p]/sumG * p;
            }
            for (int p = 255; p >= T; p--) {
                m2 += (double)histogram[p]/sum2 * p;
                mG += (double)histogram[p]/sumG * p;
            }

            return P1*pow((m1-mG), 2) + P2*pow((m2-mG), 2);
        }
        void toGray(CImg<unsigned char> &img, CImg<unsigned char> &gimg) {
             // 返回灰度图
            for (int w = 0; w < img.width(); w++) {
                for (int h = 0; h < img.height(); h++) {
                    gimg(w, h) = 0.3*img(w, h, 0) + 0.59*img(w, h, 1) + 0.11*img(w, h, 2);
                }
            }
        }
        void updateHistogram(CImg<unsigned char> &img) {
            // 根据图像更新直方图
            memset(histogram, 0, sizeof(unsigned long) * 255);
            for(int p = 0; p < img.size(); p++) {
                histogram[(int)img(p)]++;
            }
            pdf[0] = histogram[0];
            for (int p = 1; p <= 255; p++) {
                pdf[p] = pdf[p-1] + histogram[p];
            }
        }
};

#endif