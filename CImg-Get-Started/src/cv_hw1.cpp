#include "include/image.h"
#include "include/rgb_color.h"

rgb_color yellow(255, 255, 0);
rgb_color blue(0, 0, 255);

int main() {
    // Task 1
    image img("./image/1.bmp");
    img.display();

    // Task 2
    rgb_color white_black[2] = {rgb_color(255, 255, 255), rgb_color(0, 0, 0)};
    rgb_color red_green[2] = {rgb_color(255, 0, 0), rgb_color(0, 255, 0)};
    img.replace(white_black, red_green, 2).display();

    // Task 3
    img.draw_circle(50, 50, 30, blue).display();

    // Task 4
    img.draw_circle(50, 50, 3, yellow).display();
    return 0;
}

