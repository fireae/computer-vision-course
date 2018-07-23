#include <iostream>
#include <string>
#include <cstring>

#include "SVM_recognizer/SVM_recognizer.hpp"
using namespace std;

int main(int argc, char** argv){
    SVM_recognizer recognizer;
    // recognizer.train("input/train.csv");
    recognizer.load("models/svm_model.xml");
    recognizer.test("input/train.csv", false);
    return EXIT_SUCCESS;
}
