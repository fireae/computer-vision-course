#ifndef IMAGE_H
#define IMAGE_H
#include "../lib/CImg.h"
#include<string>
#include "rgb_color.h"
using namespace cimg_library;
/**
 * This class is use to test CImg object.
 * We process the instance of the class like a real world picture.
 */
class image{
    private:
        CImg<unsigned char> img;
        char filepath[128];
    public:    
        image(const char* filepath) {
            strncpy(this->filepath, filepath, strlen(filepath));
            img = CImg<unsigned char>(filepath);
        }
        /** 
         * @Author: flydreame 
         * @Date: 2018-03-15 13:56:09 
         * @Desc: Display picture instance.
         */        
        void display() {
            img.display();
        }
        /** 
         * @Author: flydreame 
         * @Date: 2018-03-15 13:59:29 
         * @Desc: Replace origin colors in list with target colors
         * @Params: Origin color list, target color list
         * @Returns: The instance of this image.
         */        
        image& replace(rgb_color* origin_colors, rgb_color* target_colors, int n) {
            for (int x = 0; x < img.width(); x++) {    
                for (int y = 0; y < img.height(); y++) {

                    // Compare the pixel color with origin color list.
                    for (int c = 0; c < n; c++) {
                        if (is_equal(x, y, origin_colors[c])) {
                            assign_color(x, y, target_colors[c]);
                            break;
                        }
                    }
                }
            }
            return (*this);
        }
        // Compare pixel color with a image pixel at (w, h)
        bool is_equal(int x, int y, rgb_color color) {
            return img(x, y, 0) == color.r
                && img(x, y, 1) == color.g
                && img(x, y, 2) == color.b;
        }
        // Assign a image pixel to a color
        void assign_color(int x, int y, rgb_color color) {
            img(x, y, 0) = color.r;
            img(x, y, 1) = color.g;
            img(x, y, 2) = color.b;
        }
        
        /** 
         * @Author: flydreame 
         * @Date: 2018-03-15 14:18:36 
         * @Desc: Draw circle graph on image.
         * @Params: (x, y) position of the circle center, circle's r, circle's color.
         * @Returns: The instance of this image.
         */

        image& draw_circle(int x, int y, int r, rgb_color c) {
            img.draw_circle(x, y, r, c.rgb);
            return (*this);
        }
};
#endif