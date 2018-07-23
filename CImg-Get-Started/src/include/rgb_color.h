#ifndef RGB_COLOR_H
#define RGB_COLOR_H
struct rgb_color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char rgb[3];
    rgb_color(unsigned char r=0, unsigned char g=0, unsigned char b=0) {
        this->r = rgb[0] = r;
        this->g = rgb[1] = g;
        this->b = rgb[2] = b;
    }
};
#endif