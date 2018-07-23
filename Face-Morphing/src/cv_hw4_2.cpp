#include "./poseDetector.hpp"
#include "./trans_matrix.hpp"
#include "./facePose.hpp"
#include<stdio.h>
#include "lib/CImg.h"
#include<string>
#include<queue>
#include<vector>
#include<algorithm>
#include<cmath>
#include<climits>
#include<cstdlib>
#include<ctime>

using namespace std;
using namespace cimg_library;

// 双线性插值
vector<double> getColor(CImg<unsigned char> *rgbImg, pose &v) {
	
	double a = v.w - floor(v.w);
	double b = v.h - floor(v.h);
	double nw, ne, sw, se;
    vector<double> color;
	for (int c = 0; c < rgbImg->spectrum(); c++) {
		nw = (*rgbImg)(floor(v.w), floor(v.h), c);
		ne = (*rgbImg)(ceil(v.w), floor(v.h), c);
		sw = (*rgbImg)(floor(v.w), ceil(v.h), c);
		se = (*rgbImg)(ceil(v.w), ceil(v.h), c);

		color.push_back((1-a)*(1-b)*nw + a*(1-b)*sw + (1-a)*b*ne + a*b*se);
	}
    return color;
}

// 计算中间色
void getMColor(CImg<unsigned char> &mImg, CImg<unsigned char> &img1, CImg<unsigned char> &img2, pose &mp, pose &p1, pose &p2, double r) {
    if (p1.w < 0 || p1.w >=img1.width() || p1.h < 0 || p1.h >= img1.height()
    || p2.w < 0 || p2.w >= img2.width() || p2.h < 0 || p2.h >= img2.height()) return;
    vector<double> color1 = getColor(&img1, p1);
    vector<double> color2 = getColor(&img2, p2);

    mImg(mp.w, mp.h, 0) = r*color1[0] + (1-r)*color2[0];
    mImg(mp.w, mp.h, 1) = r*color1[1] + (1-r)*color2[1];
    mImg(mp.w, mp.h, 2) = r*color1[2] + (1-r)*color2[2];
}

// 计算中间图
void computeMiddleImg(CImg<unsigned char> &img1, CImg<unsigned char> &img2,
vector<triangle> &mesh1, vector<triangle> &mesh2, double r = 0.5, string oPath = "") {
    // get middle size
    double mWidth = r*img1.width() + (1-r)*img2.width(), mHeight = r*img1.height() + (1-r)*img2.height();

    // middle Img
    CImg<unsigned char> mImg((int)mWidth, (int)mHeight, 1, 3);
    trans_matrix M1, M2;

    // rate
    double rW1, rW2, rH1, rH2;
    rW1 = (double)img1.width()/mWidth;
    rW2 = (double)img2.width()/mWidth;
    rH1 = (double)img1.height()/mHeight;
    rH2 = (double)img2.height()/mHeight;

    // get middle color
    for (int h = 0; h < mHeight; h++) {
        for (int w = 0; w < mWidth; w++) {
            pose mp = pose(w, h);
            pose p1 = pose(w*rW1, h*rH1);
            pose p2 = pose(w*rW2, h*rH2);
            getMColor(mImg, img1, img2, mp, p1, p2, r);
        }
    }

    // get middle mesh
    vector<triangle> mMesh = middleMesh(mesh1, mesh2, r);
    /* checkMesh(mImg, mMesh); */

    // solve middle triangle
    for (int t = 0; t < mesh1.size(); t++) {

        trans_matrix Mt1, Mt2;
        Mt1.solveAffine(mMesh[t], mesh1[t]);
        Mt2.solveAffine(mMesh[t], mesh2[t]);

        // traverse every point on triangle
        pose v1 = mMesh[t].v1, v2 = mMesh[t].v2, v3 = mMesh[t].v3;
        if (v1.h > v2.h) swap(v1, v2);
        if (v1.h > v3.h) swap(v1, v3);
        if (v2.h > v3.h) swap(v2, v3);
        double r_h = 1;
        if (v3.h-v1.h != 0) {
            r_h = (v2.h-v1.h)/(v3.h-v1.h);
        }
        pose pCross = pose(v1.w + r_h*(v3.w-v1.w), v2.h);

        for (int h = (int)v1.h; h <= (int)v2.h; h++) {
            double r_wb = 1.0, r_we = 1.0;
            if (v2.h - v1.h != 0) r_wb = (h - v1.h)/(v2.h - v1.h);
            if (pCross.h - v1.h != 0) r_we = (h - v1.h)/(pCross.h - v1.h);

            double wb = r_wb*(v2.w - v1.w) + v1.w;
            double we = r_we*(pCross.w - v1.w) + v1.w;
            if (wb > we) swap(wb, we);
            for (int w = (int)wb; w <= (int)we; w++) {
                pose mp(w, h);
                pose p1 = Mt1.mul(mp);
                pose p2 = Mt2.mul(mp);
                getMColor(mImg, img1, img2, mp, p1, p2, r);
            }
        }
        for (int h = (int)v2.h; h <= (int)v3.h; h++) {
            double r_wb = 1.0, r_we = 1.0;
            if (v2.h - v3.h != 0) r_wb = (h - v3.h)/(v2.h - v3.h);
            if (pCross.h - v3.h != 0) r_we = (h - v3.h)/(pCross.h - v3.h);

            double wb = r_wb*(v2.w - v3.w) + v3.w;
            double we = r_we*(pCross.w - v3.w) + v3.w;
            if (wb > we) swap(wb, we);
            for (int w = (int)wb; w <= (int)we; w++) {
                pose mp(w, h);
                pose p1 = Mt1.mul(mp);
                pose p2 = Mt2.mul(mp);
                getMColor(mImg, img1, img2, mp, p1, p2, r);
            }
        }
    }

    // Save image
    if (oPath != "") {
        mImg.save(oPath.c_str());
    }
}


// TODO: 增加面部辅助点
void addPose(CImg<unsigned char> img, vector<pose> &pl) {
    for (int p = 0; p < pl.size(); p++) {
        printPose(img, pl[p], 2, 'r');
    }
    CImgDisplay disp(img, "Click image add new pose");
    while(!disp.is_closed()) {
        if (disp.button()&1) {
            int x = disp.mouse_x();
            int y = disp.mouse_y();
            if (x == -1 || y == -1) {
                continue;
            }
            printf("%d %d\n", x, y);
            pose p(x, y);
            printPose(img, p, 4, 'b');
            pl.push_back(p);
            disp.render(img);
            disp.paint();
        }
        disp.wait();
    }
}

// 输入路径，输出路径，测试图像
string inputBmpPath = "../images/Dataset2/bmp/";
string inputPngPath = "../images/Dataset2/";
string outputPath = "../output/";
string pictureName[6] = {"1", "2"};

int main(int argc, char ** argv) {
    clock_t start, stop;
    if (argc <= 2) {
        printf("Error! Please input with two image name!");
        return 0;
    }

    // Solve image1 and image2 poses
    start = clock();
    string pngPath = inputPngPath + argv[1] + ".png";
    vector<pose> poses1 = poseDetector(pngPath.c_str());
    pngPath = inputPngPath + argv[2] + ".png";
    vector<pose> poses2 = poseDetector(pngPath.c_str());
    stop = clock();
    printf("\n Poses detection on two images costs %f sec.  \n", (double)(stop - start) / CLOCKS_PER_SEC);
    
    // print pose to CImg
    //  Image1
    string bmpPath = inputBmpPath + argv[1] + ".bmp";
    CImg<unsigned char> img1(bmpPath.c_str());
    addPose(img1, poses1);
    poses1.push_back(pose(0, 0, 0, poses1.size()));
    poses1.push_back(pose(0, img1.height()-1, 0, poses1.size()));
    poses1.push_back(pose(img1.width()-1, 0, 0, poses1.size()));
    poses1.push_back(pose(img1.width()-1, img1.height()-1, 0, poses1.size()));
    printf("Image1 totally has %lu poses\n", poses1.size());

    //  Image2
    bmpPath = inputBmpPath + argv[2] + ".bmp";
    CImg<unsigned char> img2(bmpPath.c_str());
    addPose(img2, poses2);
    poses2.push_back(pose(0, 0, 0, poses2.size()));
    poses2.push_back(pose(0, img2.height()-1, 0, poses2.size()));
    poses2.push_back(pose(img2.width()-1, 0, 0, poses2.size()));
    poses2.push_back(pose(img2.width()-1, img2.height()-1, 0, poses2.size()));
    printf("Image2 totally has %lu poses\n", poses2.size());


    // get mesh and check
    start = clock();
    vector<triangle> mesh1 = solveMesh(poses1, img1.width(), img1.height());
    stop = clock();
    printf("\n Solving delaunay triangles of image1 costs %f sec.  \n", (double)(stop - start) / CLOCKS_PER_SEC);
    checkMesh(img1, mesh1);
    // get mesh2 relative to mesh1
    vector<triangle> mesh2 = relativeMesh(poses2, mesh1);
    checkMesh(img2, mesh2);

    // compute middle image
    double r[11] = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1};
    start = clock();
    for (int i = 0; i < 11; i++) {
        string oPath = outputPath + to_string(i) + ".bmp";
        computeMiddleImg(img1, img2, mesh1, mesh2, r[i], oPath);
    }
    stop = clock();
    printf("\n Getting 11 middle image flashes costs %f sec.  \n", (double)(stop - start) / CLOCKS_PER_SEC);
    
}
