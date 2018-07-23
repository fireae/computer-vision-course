#include <iostream>
#include <string>
#include <cstring>
using namespace std;

#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>
using namespace cv;


#include<CImg.h>
using namespace cimg_library;


#include "SVM_recognizer/SVM_recognizer.hpp"

int main(int argc, char** argv){
    /*
    printf("Testing MNIST number\n");
    Ptr<ml::TrainData> train_data;
	train_data = ml::TrainData::loadFromCSV("input/train.csv", 1, 0, 1);
	Mat m = train_data->getSamples();
    Mat labels = train_data->getTrainNormCatResponses();
    m.convertTo(m, CV_8UC1);

    printf("%d %d\n", m.rows, m.cols);
    printf("%d %d\n", labels.rows, labels.cols);
    // cout<<m<<endl;
    
    Mat p(28, 28, CV_8UC1, Scalar(0));
    for (int h = 0; h < 28; h++) {
        for (int w = 0; w < 28; w++) {
            p.at<uchar>(h, w) = m.at<uchar>(atoi(argv[1]), h*28 + w);
            if (p.at<uchar>(h, w) > 0) p.at<uchar>(h, w) = 255;
            // printf("%3d ", (int)p.at<uchar>(h, w));
            printf("%3d ", (int)m.at<uchar>(atoi(argv[1]), h*28 + w));
        }
        cout<<endl;
    }
    printf("number = %d\n", (int)labels.at<uchar>(atoi(argv[1]), 0));

    Size dsize = Size(20, 20);
    resize(p, p, dsize);
    HOGDescriptor hog(cvSize(20,20),cvSize(10,10),cvSize(5,5),cvSize(5,5),9);
    vector<float> HOG_;
    hog.compute(p, HOG_, Size(1, 1), Size(0, 0));
    cout<<HOG_.size()<<endl;

    dsize = Size(500, 500);
    resize(p, p, dsize);
    // imshow("p", p);
    // waitKey();
    */


    // test croped number
    printf("Testing croped number\n");
    CImg<unsigned char> croped_img(argv[2]);
    // croped_img.display();
    Mat temp;
    SVM_recognizer recognizer;
    recognizer.load("models/svm_model.xml");
    // recognizer.img_preprocessing(croped_img, temp);

    float scale[] = {0.8, 0.9, 1.0, 1.1, 1.2};
    int bin[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int number;
            recognizer.score_single_img(croped_img.get_resize(croped_img.width()*scale[i], croped_img.height()*scale[j]), number);
            bin[number]++;
            printf("(%f, %f) %d\n", scale[i], scale[j], number);
        }
    }

    for (int i = 0; i < 10; i++) printf("%d: %d\n", i, bin[i]);
   

    return EXIT_SUCCESS;
}
