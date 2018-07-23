#include<stdio.h>
#include "../include/canny.hpp"
#include<string>
using namespace std;

// 10组测试数据
float low[10] = {2.5, 1.0, 4.0, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5, 2.5};
float high[10] = {7.5, 7.5, 7.5, 5.0, 9.0, 7.5, 7.5, 7.5, 7.5, 7.5};
float radius[10] = {2.0, 2.0, 2.0, 2.0, 2.0, 1.0, 4.0, 2.0, 2.0, 2.0};
float width[10] = {16, 16, 16, 16, 16, 16, 16, 8, 32, 16};
float normalised[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1};


int main() {
    // 测试图像lena
    canny c("../images/bmp/lena.bmp");
    string name = "lena";
    for (int i = 0; i < 10; i++) {
        c.resetParams(low[i], high[i], radius[i], width[i], normalised[i]);
        c.displayEdge();
        c.save("../output/test", name+char(i+'0'));
    }
    return 0;
}

