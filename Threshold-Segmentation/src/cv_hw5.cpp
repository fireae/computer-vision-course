#include "segmentation/Ostu.hpp"
#include "segmentation/GT.hpp"
#include<string>
#include<cstdlib>
#include<vector>
#include<ctime>
using namespace std;

int main(int argc, char** argv) {
    GT gt;
    Ostu ostu;
    string inPath(argv[1]);
    string outPath1(argv[2]);
    string outPath2(argv[3]);
    int num = atoi(argv[4]);

    clock_t read1 = 0, read2 = 0;
    clock_t process1 = 0, process2 = 0;
    clock_t write1 = 0, write2 = 0;
    for (int i = 0; i < num; i++) {
        string inFile = inPath + '/' + to_string(i+1) + ".jpg";
        string outFile1 = outPath1 + '/' + to_string(i+1) + ".jpg";
        string outFile2 = outPath2 + '/' + to_string(i+1) + ".jpg";
        vector<double> cost1 = gt.segmentation(inFile.c_str(), false, outFile1.c_str(), true);
        vector<double> cost2 = ostu.segmentation(inFile.c_str(), false, outFile2.c_str(), true);
        read1 += cost1[0];
        read2 += cost2[0];
        process1 += cost1[1];
        process2 += cost2[1];
        write1 += cost1[2];
        write2 += cost2[2];
    }

    printf("Global Threshold algorithm average costs(on %d jpg image(s)):\
    \n  [read]: %f s\
    \n  [process]: %f s\
    \n  [write]: %f s\
    \n  [total]: %f s\n\n", num, (double)read1/CLOCKS_PER_SEC/num,
     (double)process1/CLOCKS_PER_SEC/num, (double)write1/CLOCKS_PER_SEC/num,
     (double)read1/CLOCKS_PER_SEC/num + (double)process1/CLOCKS_PER_SEC/num + (double)write1/CLOCKS_PER_SEC/num);
    
    printf("Ostu algorithm average costs(on %d jpg image(s)):\
    \n  [read]: %f s\
    \n  [process]: %f s\
    \n  [write]: %f s\
    \n  [total]: %f s\n\n", num, (double)read2/CLOCKS_PER_SEC/num,
     (double)process2/CLOCKS_PER_SEC/num, (double)write2/CLOCKS_PER_SEC/num,
     (double)read2/CLOCKS_PER_SEC/num + (double)process2/CLOCKS_PER_SEC/num + (double)write2/CLOCKS_PER_SEC/num);
    return 0;
}