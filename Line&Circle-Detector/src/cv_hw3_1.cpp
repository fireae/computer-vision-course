#include<stdio.h>
#include "include/cannylit.hpp"
#include "lib/CImg.h"
#include "include/lineDetector.hpp"
#include<string>
#include<queue>
#include<vector>
#include<algorithm>
#include<cmath>
#include<cstdlib>
using namespace std;
using namespace cimg_library;

// 输入路径，输出路径，测试图像
string inputPath = "../images/Dataset1/bmp/";
string outputPath = "../output";
string pictureName[6] = {"1", "2", "3", "4", "5", "6"};

int main() {
	for (int i = 0; i < 6; i++) {
		string filepath = inputPath + pictureName[i] + ".bmp";
		CImg<unsigned char> rgbImg(filepath.c_str());
		lineDetector detector(&rgbImg);
	}
    return 0;
}

