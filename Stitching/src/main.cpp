#include "./sift_matcher/sift_matcher.hpp"
#include "./sift_matcher/trans_matrix.hpp"
#include "./sift_matcher/warp_image.hpp"
#include <CImg.h>
#include <string>
#include <algorithm>
#include <numeric>
#include <ctime>
#include <cmath>
#include <climits>
#include <random> // std::default_random_engine  
#include <chrono> // std::chrono::system_clock 

using namespace std;
using namespace cimg_library;

// Get H by Ransac
int MAX_LOOP = 40000;
void Ransac_H_matrices(vector<vector<mPoint_pair> > &matchers, vector<vector<int> > &img_pairs,
        vector<trans_matrix> &h_matrices) {
    srand(time(NULL));
    vector<bool> has_matrix(matchers.size(), false);
    for (int i = 0; i < matchers.size(); i++) {
        int pairs_len = matchers[i].size();
        int img1 = img_pairs[i][0], img2 = img_pairs[i][1];

        // for max vote
        int max_vote_ind = -1, max_vote = -1;
        
        // matrix candidate
        vector<trans_matrix> candidate_matrices;
        vector<int> candidate_vote;
        int l = 0;

        while(l < MAX_LOOP) {
            // random shuffle and test matrix

            int pr1 = rand()%pairs_len, pr2 = rand()%pairs_len, 
                pr3 = rand()%pairs_len, pr4 = rand()%pairs_len;
            trans_matrix h;
            h.solveProjective(matchers[i][pr1].p1, matchers[i][pr2].p1, matchers[i][pr3].p1, matchers[i][pr4].p1,
                matchers[i][pr1].p2, matchers[i][pr2].p2, matchers[i][pr3].p2, matchers[i][pr4].p2);
            // h.solveAffine(matchers[i][pr1].p1, matchers[i][pr2].p1, matchers[i][pr3].p1,
            //     matchers[i][pr1].p2, matchers[i][pr2].p2, matchers[i][pr3].p2);


            // vote candidate
            int votes = 0;
            for (int s = 0; s < matchers[i].size(); s++) {
                mPoint p_ = h.mul(matchers[i][s].p1);
                if (sqrt(pow(p_.x-matchers[i][s].p2.x, 2) + pow(p_.y-matchers[i][s].p2.y, 2))< 3) {
                    votes++;
                }
            }
            if (votes > max_vote) {
                max_vote = votes;
                max_vote_ind = candidate_matrices.size();
                candidate_matrices.push_back(h);
                candidate_vote.push_back(votes);
            }
            l++;
        }

        // add max vote matrix
        h_matrices.push_back(candidate_matrices[max_vote_ind]);
    }
}
double maxOfFour(double w, double x, double y, double z) {
    double p1 = max(w, x);
    double p2 = max(y, z);
    return max(p1, p2);
}
double minOfFour(double w, double x, double y, double z) {
    double p1 = min(w, x);
    double p2 = min(y, z);
    return min(p1, p2);
}

clock_t _sift_start, _sift_end, _ransac_start, _ransac_end, _stitching_start, _stitching_end;

int main(int argc, const char* argv[]) {
    string mode;
    if (argc == 2) {
        mode = string(argv[1]);  // format "xxx/"
    }
    else {
        printf("Wrong input format");
    }

    printf("%s\n", mode.c_str());
    vector<string> filename;
    string file_root;
    if (mode == "TEST4") {
        file_root = "./input/TEST-ImageData(1)/";
        filename.push_back(string("pano1_0011.bmp"));
        filename.push_back(string("pano1_0010.bmp"));
        filename.push_back(string("pano1_0009.bmp"));
        filename.push_back(string("pano1_0008.bmp"));
    }
    else if (mode == "TEST18") {
        file_root = "./input/TEST-ImageData(2)/";
        filename.push_back(string("100NIKON-DSCN0008_DSCN0008.JPG"));
        filename.push_back(string("100NIKON-DSCN0009_DSCN0009.JPG"));
        filename.push_back(string("100NIKON-DSCN0010_DSCN0010.JPG"));
        filename.push_back(string("100NIKON-DSCN0011_DSCN0011.JPG"));
        filename.push_back(string("100NIKON-DSCN0012_DSCN0012.JPG"));
        filename.push_back(string("100NIKON-DSCN0013_DSCN0013.JPG"));
        filename.push_back(string("100NIKON-DSCN0014_DSCN0014.JPG"));
        filename.push_back(string("100NIKON-DSCN0015_DSCN0015.JPG"));
        filename.push_back(string("100NIKON-DSCN0016_DSCN0016.JPG"));
        filename.push_back(string("100NIKON-DSCN0017_DSCN0017.JPG"));
        filename.push_back(string("100NIKON-DSCN0018_DSCN0018.JPG"));
        filename.push_back(string("100NIKON-DSCN0019_DSCN0019.JPG"));
        filename.push_back(string("100NIKON-DSCN0020_DSCN0020.JPG"));
        filename.push_back(string("100NIKON-DSCN0021_DSCN0021.JPG"));
        filename.push_back(string("100NIKON-DSCN0022_DSCN0022.JPG"));
        filename.push_back(string("100NIKON-DSCN0023_DSCN0023.JPG"));
        filename.push_back(string("100NIKON-DSCN0024_DSCN0024.JPG"));
        filename.push_back(string("100NIKON-DSCN0025_DSCN0025.JPG"));

    }
    for (int i = 0; i < filename.size(); i++) {
        filename[i] = file_root+filename[i];
    }
    // Sift extracting and find matching pair.
    _sift_start = clock();
    vector<vector<mPoint_pair> > matchers;
    vector<vector<int> > img_pairs;
    sift_matcher(filename, matchers, img_pairs);
    _sift_end = clock();
    // RANSAC get H matrix
    _ransac_start = clock();
    vector<trans_matrix> h_matrices;
    Ransac_H_matrices(matchers, img_pairs, h_matrices);
    _ransac_end = clock();

    for (int i = 0; i < h_matrices.size(); i++) {
        printf("%d %d\n", img_pairs[i][0], img_pairs[i][1]);
        printf("[%f, %f, %f\n", h_matrices[i].a, h_matrices[i].b, h_matrices[i].c);
        printf("%f, %f, %f\n", h_matrices[i].d, h_matrices[i].e, h_matrices[i].f);
        printf("%f, %f, %f]\n\n", h_matrices[i].g, h_matrices[i].h, h_matrices[i].i);
    }

    //-- Image Stitching
    _stitching_start = clock();
    // get the invert matrices to the first image
    vector<trans_matrix> r_h_matrices(filename.size());
    vector<trans_matrix> back_matrices(filename.size());
    vector<bool> img_marked(filename.size(), false);
    for (int i = 0; i < img_pairs.size(); i++) {
        int i1 = img_pairs[i][0], i2 = img_pairs[i][1];
        if (img_marked[i1]) {
            r_h_matrices[i2] = r_h_matrices[i1].mul(h_matrices[i].invert());
            back_matrices[i2] = h_matrices[i].mul(back_matrices[i1]);
            img_marked[i2] = true;
        }
        else {
            img_marked[i1] = true;
            img_marked[i2] = true;
            r_h_matrices[i2] = h_matrices[i].invert();
            back_matrices[i2] = h_matrices[i];
        }
    }

    // get minPoint and maxPoint
    vector<mPoint> minPs, maxPs;
    mPoint minP(INT_MAX, INT_MAX), maxP(INT_MIN, INT_MIN);
    vector<CImg<unsigned char> > img_list(filename.size());
    vector<int> img_order;
    for (int i = 0; i < filename.size(); i++) {
        // read image and resize
        img_list[i] = CImg<unsigned char>(filename[i].c_str());
        if (img_list[i].height()>512) {
            double scale = 512.0/img_list[i].height();
            img_list[i].resize(scale*img_list[i].width(), scale*img_list[i].height());
        }
        warp_image(img_list[i]);
        
        mPoint p1 = r_h_matrices[i].mul(mPoint(0, 0));
        mPoint p2 = r_h_matrices[i].mul(mPoint(0, img_list[i].height()));
        mPoint p3 = r_h_matrices[i].mul(mPoint(img_list[i].width(), 0));
        mPoint p4 = r_h_matrices[i].mul(mPoint(img_list[i].width(), img_list[i].height()));

        minPs.push_back(mPoint(minOfFour(p1.x, p2.x, p3.x, p4.x), minOfFour(p1.y, p2.y, p3.y, p4.y)));
        maxPs.push_back(mPoint(maxOfFour(p1.x, p2.x, p3.x, p4.x), maxOfFour(p1.y, p2.y, p3.y, p4.y)));

        minP.x = min(minP.x, minPs[i].x);
        minP.y = min(minP.y, minPs[i].y);
        maxP.x = max(maxP.x, maxPs[i].x);
        maxP.y = max(maxP.y, maxPs[i].y);
    }
    CImg<unsigned char> result(maxP.x-minP.x+1, maxP.y-minP.y+1, 1, 3, 0);

    // get painting order
    img_order.push_back(img_pairs[0][0]);
    for (int i =0; i < img_pairs.size(); i++) {
        img_order.push_back(img_pairs[i][1]);
    }
    // int bl_end_len = 10;
    // paint every point on picture
    for (int j = 0; j < img_order.size(); j++) {
        int i = img_order[j];
        for (int h = minPs[i].y; h <= maxPs[i].y; h++) {
            int begin = -1;
            for (int w = minPs[i].x; w <= maxPs[i].x; w++) {
                int w_fixed = w - minP.x;
                int h_fixed = h - minP.y;
                
                mPoint p = back_matrices[i].mul(mPoint(w, h));
                // paint image
                if (p.x >= 0 && p.y >= 0 && p.x <= img_list[i].width()-1 && p.y <= img_list[i].height()-1) {
                    unsigned char r = (unsigned char)linear_atXYZ(img_list[i], p.x, p.y, 0, 0);
                    unsigned char g = (unsigned char)linear_atXYZ(img_list[i], p.x, p.y, 1, 0);
                    unsigned char b = (unsigned char)linear_atXYZ(img_list[i], p.x, p.y, 2, 0);
                    if (!(r == 0 && g == 0 && b == 0)) {
                        result(w_fixed, h_fixed, 0, 0) = r;
                        result(w_fixed, h_fixed, 0, 1) = g;
                        result(w_fixed, h_fixed, 0, 2) = b;
                    }
                }
            }

        }
        // result.display();
    }
    _stitching_end = clock();
    printf("sift extracting and pair matching cost %f s\n", (double)(_sift_end-_sift_start)/CLOCKS_PER_SEC);
    printf("ransac costs %f s\n", (double)(_ransac_end-_ransac_start)/CLOCKS_PER_SEC);
    printf("stitching costs %f s\n", (double)(_stitching_end-_stitching_start)/CLOCKS_PER_SEC);

    result.display();
    string name = "./output/result_" + to_string(img_list.size()) + ".jpg";
    result.save(name.c_str());

    return 0;
}