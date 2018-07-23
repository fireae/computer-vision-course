#ifndef GAMMA_HPP
#define GAMMA_HPP

#include <math.h>
#include<CImg.h>
using namespace cimg_library;
 
typedef unsigned char UNIT8; //用 8 位无符号数表示 0～255 之间的整数
UNIT8 g_GammaLUT[256];//全局数组：包含256个元素的gamma校正查找表
//Buildtable()函数对0-255执行如下操作：
//①归一化、预补偿、反归一化;
//②将结果存入 gamma 查找表。
//从公式得fPrecompensation=1/gamma
void BuildTable(float fPrecompensation )
{
	int i;
	float f;
	for( i=0;i<256;i++)
	{
		f=(i+0.5F)/256;//归一化
		f=(float)pow(f,fPrecompensation);
		g_GammaLUT[i]=(UNIT8)(f*256-0.5F);//反归一化
	}
}
 
void GammaCorrectiom(CImg<unsigned char> &src, float fGamma)
{
	BuildTable(1/fGamma);//gamma校正查找表初始化
	//对图像的每个像素进行查找表矫正
    for (int h = 0; h < src.height(); h++) {
        for (int w = 0; w < src.width(); w++) {
            src(w, h) = g_GammaLUT[src(w, h)];
        }
    }
}

#endif