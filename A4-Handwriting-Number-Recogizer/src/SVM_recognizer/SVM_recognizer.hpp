#ifndef SVM_RECOGNIZER_HPP
#define SVM_RECOGNIZER_HPP

#include<string>
#include<queue>
#include<vector>
#include<algorithm>
#include<cmath>
#include<climits>
#include<cstdlib>
#include<ctime>
using namespace std;


#include <opencv2/opencv.hpp>
#include <opencv2/ml/ml.hpp>
using namespace cv;
using namespace ml;

#include<CImg.h>
using namespace cimg_library;



typedef std::vector<int> BBOX;

class SVM_recognizer{
    private:
        HOGDescriptor hog;
        Ptr<SVM> svm;
    public:
        SVM_recognizer() {
            hog = HOGDescriptor(cvSize(20,20), cvSize(10,10), cvSize(5,5), cvSize(5,5), 9);
            svm = SVM::create();
            svm->setType(SVM::C_SVC);
            svm->setKernel(SVM::RBF);
            svm->setDegree(10);
            svm->setGamma(0.09);
            svm->setCoef0(1.0);
            svm->setC(10.0);
            svm->setNu(0.5);
            svm->setP(1.0);
            // svm->setClassWeights(NULL);
            svm->setTermCriteria(TermCriteria(CV_TERMCRIT_EPS, 1000, 0.001));
        }
        // 计算HOG
        void computeHOG(Mat &samples, Mat &HOG_Mat) {
            // Compute HOG
            std::vector<std::vector<float> > HOGs;
            for (int r = 0; r < samples.rows; r++) {
                Mat p(28, 28, CV_8UC1, Scalar(0));
                for (int h = 0; h < 28; h++) {
                    for (int w = 0; w < 28; w++) {
                        p.at<uchar>(h, w) = samples.at<uchar>(r, h*28 + w);
                        // if (p.at<uchar>(h, w) > 0) p.at<uchar>(h, w) = 255;
                    }
                }
                // resize to 20, 20
                Size dsize = Size(20, 20);
                resize(p, p, dsize);

                std::vector<float> HOG_;
                printf("\rPrecocessing %d/%d", r+1, samples.rows);
                this->hog.compute(p, HOG_, Size(1, 1), Size(0, 0));
                HOGs.push_back(HOG_); 
            }
            printf("\n");

            // Convert to Mat.          
            HOG_Mat = Mat(HOGs.size(), HOGs[0].size(), CV_32FC1);
            for (int i = 0; i < HOGs.size(); i++) {
                for (int j = 0; j< HOGs[0].size(); j++) {
                    HOG_Mat.at<float>(i, j) = HOGs[i][j];
                }
            }
        }
        void preprocessing(string &inputfile, Mat &hog_data, Mat &labels) {
            // load data
            Ptr<TrainData> train_data;
            train_data = TrainData::loadFromCSV(inputfile.c_str(), 1, 0, 1);
            Mat raws_data = train_data->getSamples();
            labels = train_data->getTrainNormCatResponses();
            raws_data.convertTo(raws_data, CV_8UC1);

            // convert to hog feature
            computeHOG(raws_data, hog_data);
        }

        // 训练模型
        void train(string inputfile, bool silent=true) {
            // data preprocessing
            printf("Preprocessing...\n");
            Mat hog_data;
            Mat labels;
            this->preprocessing(inputfile, hog_data, labels);

            // train
            printf("Training...\n");
            Ptr<TrainData> td = TrainData::create(hog_data, ROW_SAMPLE, labels);
            this->svm->trainAuto(td);
            
            // get accuracy
            printf("Predicting...\n");
            this->test_accuracy(hog_data, labels, silent);

            this->save();
        }

        // 测试模型
        void test(string inputfile, bool silent=true) {
            // data preprocessing
            printf("Preprocessing...\n");
            Mat hog_data;
            Mat labels;
            this->preprocessing(inputfile, hog_data, labels);

            // get accuracy
            printf("Predicting...\n");
            this->test_accuracy(hog_data, labels, silent);
        }

        // 测试准确性
        void test_accuracy(Mat &hog_data, Mat &labels, bool silent=true) {
            if (!silent) {
                int acc_num = 0;
                for (int i = 0; i < labels.rows; i++) {
                    Mat labels_;
                    this->svm->predict(hog_data(Range(i, i+1), Range(0, hog_data.cols)), labels_);
                    if ((float)labels.at<uchar>(i, 0) == labels_.at<float>(0, 0)) acc_num++;
                    printf("\r%d/%d accumulated acc = %f", i+1, labels.rows, (float)acc_num/(i+1));
                }
                printf("\n");
            }
            else {
                Mat labels_;
                this->svm->predict(hog_data, labels_);
                int acc_num = 0;
                for (int i = 0; i < labels.rows; i++) {
                    if ((float)labels.at<uchar>(i, 0) == labels_.at<float>(i, 0)) acc_num++;
                }
                printf("acc = %f\n", (float)acc_num/labels.rows);
            }
        }
        // 保存模型
        void save() {
            // save SVM models
            this->svm->save("models/svm_model.xml");
        }
        // 加载以保存模型
        void load(string filepath) {
            svm = SVM::load(String(filepath.c_str()));
        }
        // 预处理图片格式 => 28x28 MNIST 膨胀+缩放
        void img_preprocessing(CImg<unsigned char> &in_img, Mat &out_hog_vector) {
            // convert to Mat
            Mat p(in_img.height(), in_img.width(), CV_8UC1, Scalar(0));
            for (int r = 0; r < p.rows; r++) {
                for (int c = 0; c < p.cols; c++) {
                    p.at<uchar>(r, c) = in_img(c, r);
                }
            }

            // // resize to 28
            // double r_ = 1;
            // if (p.rows > p.cols) r_ = 28.0/p.rows;
            // else r_ = 28.0/p.cols;
            // resize(p, p, Size(), r_, r_, 5);
            // add padding and make dilate
            // int v_padding = 4, h_padding = 12;
            // Mat p_temp(p.rows + v_padding*2, p.cols + h_padding*2, CV_8UC1, Scalar(0));
            // for (int r = 0; r < p.rows; r++) {
            //     for (int c = 0; c < p.cols; c++) {
            //         int c_ = c + h_padding, r_ = r + v_padding;
            //         p_temp.at<uchar>(r_, c_)  = p.at<uchar>(r, c);
            //     }
            // }
       	    // Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(2, 2));
            // dilate(p_temp, p_temp, kernel);
            // // resize to 28x28
            // resize(p_temp, p_temp, Size(28, 28), 0, 0, 5);


            Mat p_temp(28, 28, CV_8UC1, Scalar(0));
            double r_ = 1;
            if (p.rows > p.cols) r_ = 20.0/p.rows;
            else r_ = 20.0/p.cols;
            resize(p, p, Size(), r_, r_, 5);
            int v_padding = (28 - p.rows)/2, h_padding = (28 - p.cols)/2;
            for (int r = 0; r < p.rows; r++) {
                for (int c = 0; c < p.cols; c++) {
                    int c_ = c + h_padding, r_ = r + v_padding;
                    p_temp.at<uchar>(r_, c_)  = p.at<uchar>(r, c);
                }
            }
       	    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(2, 2));
            dilate(p_temp, p_temp, kernel);
            // resize to 28x28
            resize(p_temp, p_temp, Size(28, 28), 0, 0, 5);
            
            // print test
            // for (int r = 0; r < p_temp.rows; r++) {
            //     for (int c = 0; c < p_temp.cols; c++) {
            //         printf("%3d ", (int)p_temp.at<uchar>(r, c));
            //     }
            //     printf("\n");
            // }

            resize(p_temp, p_temp, Size(20, 20));
            vector<float> hog_vector;
            this->hog.compute(p_temp, hog_vector, Size(1, 1), Size(0, 0));
            out_hog_vector = Mat(1, hog_vector.size(), CV_32FC1);
            for (int i = 0; i < hog_vector.size(); i++) {
                out_hog_vector.at<float>(0, i) = hog_vector[i];
            }
        }
        // 测试图片
        void score_single_img(CImg<unsigned char> img, int &number) {
            // convert to 28x28 img
            Mat hog_vector;
            Mat labels_;
            img_preprocessing(img, hog_vector);
            this->svm->predict(hog_vector, labels_);
            
            number = (int)labels_.at<float>(0, 0);

            if (number == 1 || number == 7) {
                // repredict
                int crop_height = img.width()*2;
                if (crop_height > img.height()) crop_height = img.height();
                img_preprocessing(img.crop(0, 0, 0, 0, img.width(), crop_height, 0, 0), hog_vector);
                this->svm->predict(hog_vector, labels_);
                
                number = (int)labels_.at<float>(0, 0);
            }
        }
        // test multi
        void vote_number(CImg<unsigned char> img, int &number) {
            // convert to 28x28 img
            Mat hog_vector;
            Mat labels_;
            float scale[] = {0.8, 0.9, 1.0, 1.1, 1.2};
            int bin[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


            pair<int, int> p1(-1, 0);
            pair<int, int> p2(-1, 0);
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 5; j++) {
                    int number;
                    score_single_img(img.get_resize(img.width()*scale[i], img.height()*scale[j]), number);
                    bin[number]++;
                    if (bin[number] > p1.second) {
                        p1.first = number;
                        p1.second = bin[number];
                    }
                    else if (bin[number] > p2.second) {
                        p2.first = number;
                        p2.second = bin[number];
                    }
                }
            }

            number = p1.first;
            if ((p1.first == 1 || p1.first == 7) && p1.second < 20) {
                number = p2.first;
            }
        }
};



#endif