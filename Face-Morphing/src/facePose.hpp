#ifndef FACE_POSE_HPP
#define FACE_POSE_HPP

#include "lib/CImg.h"
#include<algorithm>
#include<queue>
#include<vector>
#include<climits>
#include <opencv2/imgproc/imgproc.hpp>
using namespace cimg_library;

struct pose {
    double w;
    double h;
    double v;
    int id;
    pose(double w = 0, double h = 0, double v = 0, int id = 0) {
        this->w = w;
        this->h = h;
        this->v = v;
        this->id = id;
    }
    bool operator<(const pose &p2) const {
        return v < p2.v;
    }
};

struct triangle {
    pose v1, v2, v3;
    triangle(pose v1, pose v2, pose v3) {
        this->v1 = v1;
        this->v2 = v2;
        this->v3 = v3;
    }
};


// Print pose on img
void printPose(CImg<unsigned char> &img, pose p, int margin = 10, char color = 'r') {
    unsigned char cr[3] = {255, 0, 0};
    if (color == 'b') {
        cr[0] = 0;
        cr[2] = 255;
    }

    for (int wb = -margin; wb < margin; wb++) {
        for (int hb = -margin; hb < margin; hb++) {
            int w = wb + (int)p.w, h = hb + (int)p.h;
            if (w >= 0 && w < img.width() && h >= 0 && h < img.height())
                for (int c = 0; c < img.spectrum(); c++) {
                    img(w, h, c) = cr[c];
                }
        }
    }
}

// Print line on img
void printLine(CImg<unsigned char> &img, pose p1, pose p2, int margin = 10, char color = 'r') {
    double wB = p1.w, wE = p2.w, hB = p1.h, hE = p2.h;

    double wDiff = wE - wB, hDiff = hE - hB;
    double dLen = std::sqrt(std::pow(wDiff, 2)+std::pow(hDiff, 2));
    
    for (int l = 0; l <= (int)dLen; l++) {
        double r = (double)l/dLen;
        printPose(img, pose(wB + wDiff*r, hB + hDiff*r), margin, color);
    }
}

// ------------------------------------------Mesh------------------------------------------

bool inRange(int p, int w);
void findIndex(pose &p, std::vector<pose> &poseList);

// Find mesh consist of triangles from list of points
/*
    references:
    https://blog.csdn.net/czl389/article/details/62469014
    https://blog.csdn.net/wi162yyxq/article/details/53762617
*/
std::vector<triangle> solveMesh(std::vector<pose> &poseList, size_t w, size_t h) {

    cv::Rect rect(0, 0, w, h);  

    // Insert Point
    cv::Subdiv2D subdiv(rect);
    for (int i = 0; i < poseList.size();i++) {
        subdiv.insert(cv::Point2f(poseList[i].w, poseList[i].h));
    }
    

    // Create Delaunay triangles
    std::vector<cv::Vec6f> triangleList;
    subdiv.getTriangleList(triangleList);

    // Convert to triangle
    std::vector<triangle> triList;
    for (unsigned int i = 0; i < triangleList.size(); i++) {
        cv::Vec6f t = triangleList[i];
        if (inRange(t[0], w) && inRange(t[1], h)
            && inRange(t[2], w) && inRange(t[3], h)
            && inRange(t[4], w) && inRange(t[5], h)) {
            
            // Add Index
            pose p1(t[0], t[1]), p2(t[2], t[3]), p3(t[4], t[5]);
            findIndex(p1, poseList);
            findIndex(p2, poseList);
            findIndex(p3, poseList);
            triList.push_back(triangle(p1, p2, p3));
        }
    }

    return triList;
}

// get relative mesh2 from mesh1
std::vector<triangle> relativeMesh(std::vector<pose> poseList, std::vector<triangle> &mesh1) {
    std::vector<triangle> mesh2;
    for (int t = 0; t < mesh1.size(); t++) {
        pose p1, p2, p3;
        p1 = poseList[mesh1[t].v1.id];
        p2 = poseList[mesh1[t].v2.id];
        p3 = poseList[mesh1[t].v3.id];

        p1.id = mesh1[t].v1.id;
        p2.id = mesh1[t].v2.id;
        p3.id = mesh1[t].v3.id;

        mesh2.push_back(triangle(p1, p2, p3));
    }

    return mesh2;
}

// get middle mesh from two meshes
std::vector<triangle> middleMesh(std::vector<triangle> &mesh1, std::vector<triangle> &mesh2, double r = 0.5) {
    std::vector<triangle> mMesh;
    double nr = 1-r;
    for (int t = 0; t < mesh1.size(); t++) {
        pose p1(mesh1[t].v1.w*r + mesh2[t].v1.w*nr, mesh1[t].v1.h*r + mesh2[t].v1.h*nr, 0, mesh1[t].v1.id);
        pose p2(mesh1[t].v2.w*r + mesh2[t].v2.w*nr, mesh1[t].v2.h*r + mesh2[t].v2.h*nr, 0, mesh1[t].v2.id);
        pose p3(mesh1[t].v3.w*r + mesh2[t].v3.w*nr, mesh1[t].v3.h*r + mesh2[t].v3.h*nr, 0, mesh1[t].v3.id);
        mMesh.push_back(triangle(p1, p2, p3));
    }

    return mMesh;
}


void findIndex(pose &p, std::vector<pose> &poseList) {
    double minDiff = abs(p.w-poseList[0].w) + abs(p.h-poseList[0].h);
    int index = 0;
    for (int i = 1; i < poseList.size(); i++) {
        double diff = abs(p.w-poseList[i].w) + abs(p.h-poseList[i].h);
        if (diff < minDiff) {
            index = i;
            minDiff = diff;
        }
    }
    p.id = index;
}


bool inRange(int p, int w) {
    return p <= w && p >= 0;
}

// Check mesh
void checkMesh(CImg<unsigned char> img, std::vector<triangle> &mesh) {
    for (int t = 0; t < mesh.size(); t++) {
        printLine(img, mesh[t].v1, mesh[t].v2, 1, 'b');
        printLine(img, mesh[t].v2, mesh[t].v3, 1, 'b');
        printLine(img, mesh[t].v3, mesh[t].v1, 1, 'b');
    }
    img.display();
}

#endif