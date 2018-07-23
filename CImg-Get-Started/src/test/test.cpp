#include "../include/image.h"
#include "../include/rgb_color.h"

int main() {
    // Test display(): show the image 1.bmp.
    image img("../image/1.bmp");
    img.display();

    // Test draw_circle(): draw circle on image.
    img.draw_circle(300, 80, 100, rgb_color(255, 255, 0)).display();
    img.draw_circle(10, 300, 50, rgb_color(0, 255, 0)).display();
    img.draw_circle(350, 150, 90, rgb_color(0, 128, 255)).display();

    // Test replace(): replace yellow with blue, replace black with red.
    rgb_color o_color[2] = {rgb_color(255, 255, 0), rgb_color(0, 0, 0)};
    rgb_color t_color[2] = {rgb_color(0, 0, 255), rgb_color(255, 0, 0)};
    img.replace(o_color, t_color, 2).display();

    return 0;
}