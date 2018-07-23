#include<stdio.h>
#include "include/canny.hpp"
#include<string>
using namespace std;

// 输入路径，输出路径，测试图像
string inputPath = "../images/bmp/";
string outputPath = "../output";
string pictureName[4] = {"bigben", "lena", "stpietro", "twows"};

int main() {
    for (int i = 0; i < 4; i++) {
        canny c(inputPath + pictureName[i] + ".bmp");
        c.displayAll();
        c.save(outputPath, pictureName[i]);
    }
    return 0;
}

