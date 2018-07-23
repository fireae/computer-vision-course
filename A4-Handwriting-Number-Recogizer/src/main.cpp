#include<string>
#include<queue>
#include<vector>
#include<algorithm>
#include<cmath>
#include<climits>
#include<cstdlib>
#include<ctime>
using namespace std;


#include "SVM_recognizer/SVM_recognizer.hpp"
#include "line_detection/cannylit.hpp"
#include "line_detection/lineDetector.hpp"
#include "line_detection/cannylit.hpp"
#include "segmentation/Ostu.hpp"
#include "segmentation/GT.hpp"
#include "segmentation/LT.hpp"
#include "warping/warping.hpp"
#include "cropping/cropping.hpp"
#include "io_interface/io_interface.hpp"

// 输入路径，输出路径，测试图像
// string inputPath = "../images/Dataset1/bmp/";
// string outputPath = "../output/";
// string pictureName[6] = {"1", "2", "3", "4", "5", "6"};

#include<CImg.h>
using namespace cimg_library;


#include <boost/filesystem.hpp>
using namespace boost::filesystem;


int main(int argc, char** argv) {
	clock_t start, end;


    if (argc < 3) {
        printf("./main.out [input dir] [output dir]\n");
        return 1;
    }

	// Load image list from directory.
	vector<string> imgPathList;
    int res = getImageList(path(argv[1]), imgPathList);
    if (res == 1) return 1;


	vector<string> failImg;
	for (int i = 0; i < imgPathList.size(); i++) {
		printf("\nProcessing the %d image \"%s\"\n", i+1, imgPathList[i].c_str());

		// Read as CImg image.
		CImg<unsigned char> rgbImg(imgPathList[i].c_str());
		// rgbImg.display();

		// Detect line.
		start = clock();
		lineDetector detector(&rgbImg);
		vector<double> w, h;
		detector.getCrossPoint(&w, &h);
		end = clock();
		printf("Line detecting costs %f sec.  \n", (double)(end - start) / CLOCKS_PER_SEC);


		// Projective Warp image.
		CImg<unsigned char> outImg;
		if (w.size() < 4) {
			failImg.push_back(imgPathList[i]);
			if (rgbImg.width() > rgbImg.height()) {
				outImg = CImg<unsigned char>(rgbImg.height(), rgbImg.width(), 1, rgbImg.spectrum());
				for (int h = 0; h < outImg.height(); h++) {
					for (int w = 0; w < outImg.width(); w++) {
						outImg(w, h, 0, 0) = rgbImg(h, outImg.width()-1-w, 0, 0);
						outImg(w, h, 0, 1) = rgbImg(h, outImg.width()-1-w, 0, 1);
						outImg(w, h, 0, 2) = rgbImg(h, outImg.width()-1-w, 0, 2);
					}
				}
			}
			else outImg = rgbImg;
		}
		else {
			start = clock();
			A4WarpingProjective(&rgbImg, &outImg, w, h);
			end = clock();
			printf("Projective Warping costs %f sec. \n", (double)(end - start) / CLOCKS_PER_SEC);
		}
	
		// outImg.display();
		// string outpath = "output/warping.jpg";
		// outImg.save(outpath.c_str());

		// Segmentation.
		start = clock();
		CImg<unsigned char> gimg(outImg.width(), outImg.height());
		LT lt;
		lt.segmentation(outImg, gimg);
		end = clock();
		printf("LT segmentation costs %f sec. \n", (double)(end - start) / CLOCKS_PER_SEC);

		// gimg.display();
		// outpath = "output/segmentation.jpg";
		// gimg.save(outpath.c_str());

		// Crop number bbox.
		start = clock();
		vector<vector<BBOX> > lines;
		CImg<long> mark;
		crop_number(gimg, mark, lines);
		end = clock();
		printf("Cropping number bboxes costs %f sec. \n", (double)(end - start) / CLOCKS_PER_SEC);

		// Recognizer.
		start = clock();
		SVM_recognizer recognizer;
		vector<vector<int> > number_list;
		recognizer.load("models/svm_model.xml");
		bool warning = false;
		for (int i = 0; i < lines.size(); i++) {
			number_list.push_back(vector<int>());
			for (int j = 0; j < lines[i].size(); j++) {
				int number;
				try {
					recognizer.vote_number(gimg.get_crop(lines[i][j][0], lines[i][j][2], 0, 0, lines[i][j][1], lines[i][j][3], 0, 0), number);
					printf("%d", number);
					number_list[i].push_back(number);
				}
				catch (const exception& ex) {
					warning = true;
				}
			}
			printf("\n");
		}
		end = clock();
		printf("Recognizing number bboxes costs %f sec. \n", (double)(end - start) / CLOCKS_PER_SEC);

		if (warning) failImg.push_back(imgPathList[i]);

		// Output.
		saveCSVResult(imgPathList[i], string(argv[2]), number_list);
	}

	printf("-----fail line detect img-----\n");
	for (int i = 0; i < failImg.size(); i++) {
		printf("%s\n", failImg[i].c_str());
	}
    return 0;
}

