#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/xfeatures2d.hpp> //Thanks to Alessandro
#include <opencv2/flann.hpp>
#include <iostream>
using namespace cv;

const char* keys =
    "{ help h |                          | Print help message. }"
    "{ input1 | ../data/box.png          | Path to input image 1. }"
    "{ input2 | ../data/box_in_scene.png | Path to input image 2. }"
    "{ r | 1 | radio to scale image show. }";

int main(int argc, const char* argv[])
{
    //-- Read path of two image
    CommandLineParser parser( argc, argv, keys );

    //-- Load img
    Mat img1 = imread( parser.get<String>("input1"));
    Mat img2 = imread( parser.get<String>("input2"));
    if ( img1.empty() || img2.empty() )
    {
        std::cout << "Could not open or find the image!\n" << std::endl;
        parser.printMessage();
        return -1;
    }

    //-- Detect the keypoints using SIFT Detector, compute the descriptors
    int minHessian = 400;
    Ptr<xfeatures2d::SIFT> detector = xfeatures2d::SIFT::create( minHessian );
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;
    detector->detectAndCompute(img1, noArray(), keypoints1, descriptors1);
    detector->detectAndCompute(img2, noArray(), keypoints2, descriptors2);

    // create kd tree
    flann::KDTreeIndexParams indexParam(2);
    // flann::Index kdtree1(descriptors1, indexParam);
    flann::Index kdtree2(descriptors2, indexParam);

    std::cout<<keypoints1.size()<<std::endl;
    std::cout<<keypoints2.size()<<std::endl;
    std::cout<<descriptors1.rows<<" "<<descriptors1.cols<<" "<<descriptors1.channels()<<std::endl;
    std::cout<<descriptors2.rows<<" "<<descriptors2.cols<<" "<<descriptors1.channels()<<std::endl;

    std::vector<int> outIndex(2);
    std::vector<float> outDist(2);
    flann::SearchParams params(32);
    kdtree2.knnSearch(descriptors2.row(1), outIndex, outDist, 2, params);

    std::cout<<outIndex[0]<<std::endl;
    std::cout<<outDist[0]<<std::endl;
    /*
    //-- Matching descriptor vectors with a FLANN based matcher
    // Since SURF is a floating-point descriptor NORM_L2 is used
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    std::vector< std::vector<DMatch> > knn_matches;
    matcher->knnMatch( descriptors1, descriptors2, knn_matches, 2 );

    //-- Filter matches using the Lowe's ratio test
    const float ratio_thresh = 0.7f;
    std::vector<DMatch> good_matches;
    for (size_t i = 0; i < knn_matches.size(); i++)
    {
        if (knn_matches[i].size() > 1 && knn_matches[i][0].distance / knn_matches[i][1].distance <= ratio_thresh)
        {
            good_matches.push_back(knn_matches[i][0]);
        }
    }

     //-- Draw matches
    Mat img_matches;
    drawMatches( img1, keypoints1, img2, keypoints2, good_matches, img_matches, Scalar::all(-1),
                 Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    //-- Show detected matches

    Mat resized_img;
    resize(img_matches, resized_img, Size(), parser.get<double>("r"), parser.get<double>("r"));
    namedWindow("Good Matches", 0); 
    resizeWindow("enhanced", 100, 100);
    imshow("Good Matches", resized_img);
    waitKey();
    */

    return 0;
}
