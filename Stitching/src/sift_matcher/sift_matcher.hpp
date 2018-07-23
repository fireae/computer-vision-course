#ifndef SIFT_MATCHER_HPP
#define SIFT_MATCHER_HPP
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/xfeatures2d.hpp> //Thanks to Alessandro
#include <opencv2/flann.hpp>
#include <iostream>
#include <CImg.h>
#include <string>
#include <queue>
#include <vector>
#include "point.hpp"
#include "warp_image.hpp"
using namespace cv;
using namespace cimg_library;

int MIN_HESSIAN = 1500;
Ptr<xfeatures2d::SIFT> __detector = xfeatures2d::SIFT::create(MIN_HESSIAN);
flann::KDTreeIndexParams __indexParam(2);
flann::SearchParams __params(32);

void __sift_extractor(std::string &file_path, std::vector<mPoint> &mpl, Mat &descriptors) {
    std::vector<KeyPoint> kpl;
    Mat img = imread(file_path.c_str());
    if (img.rows > 512) {
        double scale = 512.0/img.rows;
        resize(img, img, Size(), scale, scale);
    }
    warp_image(img);
    // img.convertTo(img, CV_8U);
    // cvtColor(img, img, COLOR_BGR2GRAY);
    // TODO: reduce size
    __detector->detectAndCompute(img, noArray(), kpl, descriptors);
    for (int i = 0; i < kpl.size(); i++) {
        mpl.push_back(mPoint(kpl[i].pt.x, kpl[i].pt.y));
    }
}

void sift_matcher(std::vector<std::string> &filepath_list, std::vector<std::vector<mPoint_pair> > &matchers, 
                std::vector<std::vector<int> > &img_pairs) {
    // flann::Index kdtree1(descriptors1, indexParam);
    std::vector<flann::Index*> kdtree_list(filepath_list.size());
    std::vector<std::vector<mPoint> > siftpoint_list(filepath_list.size());
    // PIT: The Index can't be copied by assigned, so we use pointer here.
    std::vector<Mat> descriptors_list(filepath_list.size());

    // get every img sift point and kdtree
    for (int i = 0; i < filepath_list.size(); i++) {
        __sift_extractor(filepath_list[i], siftpoint_list[i], descriptors_list[i]);
        // Compute kdtree index
        kdtree_list[i] = new flann::Index(descriptors_list[i], __indexParam);
    }
    
    // for every img make knn search
    std::vector<bool> matched(descriptors_list.size(), false);
    std::queue<int> next_img;
    next_img.push(0);
    while(!next_img.empty()) {
        int top = next_img.front();
        next_img.pop();
        if (!matched[top]) {
            matched[top] = true;
            // find the img[i] match to img[top] with the number of match pair larger than a theshold
            for (int i = 0; i < matched.size(); i++) {
                if (!matched[i]) {
                    std::vector<mPoint_pair> temp;
                    // for every key point descriptor in img[top], make "Feature-space outlier rejection"
                    for (int r = 0; r <  descriptors_list[top].rows; r++) {
                        std::vector<int> matchIndex(2);
                        std::vector<float> matchDist(2);
                        kdtree_list[i]->knnSearch(descriptors_list[top].row(r), 
                                        matchIndex, matchDist, 2, __params);
                        
                        // reject the match if N1/N2 less than threshold
                        float N1_to_N2 = matchDist[0]/matchDist[1];
                        if (N1_to_N2 < 0.35) { //0.2
                            // accept the match pair
                            temp.push_back(mPoint_pair(siftpoint_list[top][r], siftpoint_list[i][matchIndex[0]]));
                        }
                    }

                    // accept the img if match pair number larger than a threshold
                    if (temp.size() > 20) {
                        matchers.push_back(temp);
                        std::vector<int> img_pair(2);
                        img_pair[0] = top;
                        img_pair[1] = i;
                        img_pairs.push_back(img_pair);

                        next_img.push(i);
                    }
                }
            }
        }
    }
}

#endif