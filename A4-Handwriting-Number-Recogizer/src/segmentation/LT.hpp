#ifndef LT_HPP
#define LT_HPP
#include<CImg.h>
#include<cstring>
#include<cmath>
#include<ctime>
#include<vector>
#include "../gamma/gamma.hpp"
#include "./Ostu.hpp"
#include "./GT.hpp"
using namespace std;
using namespace cimg_library;
class LT {
    private:
        int block_size;
        double additive;
    public:
        LT(int block_size = 60, double additive = -20) {
            this->block_size = block_size;
            this->additive = additive;
        }
        void segmentation(CImg<unsigned char> &img, CImg<unsigned char> &gimg, bool show=false, const char* outPath = NULL, bool save=false) {
            // CImg<unsigned char> gimg(img.width(), img.height()); 
            toGray(img, gimg); // 获取灰度图
            gimg.blur(1.2);
            // gimg.display();

            double r_ = 2600.0/gimg.height();
            gimg.resize(gimg.width()*r_, gimg.height()*r_);
            
            int width = gimg.width();
            int height = gimg.height();

            int w_num = ceil((double)width/this->block_size);
            int h_num = ceil((double)height/this->block_size);

            for (int h_ = 0; h_ < h_num; h_++) {
                for (int w_ = 0; w_ < w_num; w_++) {
                    // int sum = 0;
                    // for (int h = h_ * this->block_size; h < (h_ + 1) * this->block_size && h < height; h++) {
                    //     for (int w = w_ * this->block_size; w < (w_ + 1) * this->block_size && w < width; w++) {
                    //         sum += gimg(w, h);
                    //     }
                    // }

                    // double threshold = (double)sum/(this->block_size * this->block_size) + additive;

                    int ul = w_ * this->block_size, ut = h_ * this->block_size;
                    int lr =  (w_ + 1) * this->block_size-1, ld = (h_ + 1) * this->block_size-1;
                    if (lr >= width) lr = width-1;
                    if (ld >= height) ld = height-1;

                    // GT gt;
                    // int threshold = gt.get_threshold(gimg.get_crop(ul, ut, 0, 0, lr, ld, 0, 0)) + additive;
                    Ostu ostu;
                    int threshold = ostu.get_threshold(gimg.get_crop(ul, ut, 0, 0, lr, ld, 0, 0)) + additive;


                    for (int h = h_ * this->block_size; h < (h_ + 1) * this->block_size && h < height; h++) {
                        for (int w = w_ * this->block_size; w < (w_ + 1) * this->block_size && w < width; w++) {
                            if (gimg(w, h) >= threshold) gimg(w, h) = 255;
                            else gimg(w, h) = 0;
                        }
                    }
                }
            }

            // delete single point
        }
        void toGray(CImg<unsigned char> &img, CImg<unsigned char> &gimg) {
             // 返回灰度图
            for (int w = 0; w < img.width(); w++) {
                for (int h = 0; h < img.height(); h++) {
                    gimg(w, h) = 0.3*img(w, h, 0) + 0.59*img(w, h, 1) + 0.11*img(w, h, 2);
                }
            }
            // gimg.display();
            GammaCorrectiom(gimg, 1.4);
            // gimg.display();
        }
};
#endif