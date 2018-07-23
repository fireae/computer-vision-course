#ifndef WARPING_HPP
#define WARPING_HPP

#include<stdio.h>
#include<CImg.h>
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


struct point {
	double w;
	double h;
	double z;
	point(double w=0, double h=0) {
		this->w = w;
		this->h = h;
		this->z = 1;
	}
};
struct trans_matrix {
	double a, b, c;
	double d, e, f;
	double g, h, i;
	trans_matrix(double a=0, double b=0, double c=0, 
	double d=0, double e=0, double f=0, 
	double g=0, double h=0, double i=1) {
		this->a = a;
		this->b = b;
		this->c = c;
		this->d = d;
		this->e = e;
		this->f = f;
		this->g = g;
		this->h = h;
		this->i = i;
	}
	point mul(point x) {
		point p;
		p.z = x.w*g + x.h*h + i;
		p.w = (x.w*a + x.h*b + c) / p.z;
		p.h = (x.w*d + x.h*e + f) / p.z;
		p.z = 1;
		return p;
	}
	void solveAffine(point &x1, point &x2, point &x3, point &y1, point &y2, point &y3) {
		CImg<double> X(3, 3, 1, 1, 
					x1.w, x1.h, 1.0,
					x2.w, x2.h, 1.0,
					x3.w, x3.h, 1.0);
		CImg<double> Y1(1, 3, 1, 1,
					y1.w, y2.w, y3.w);
		CImg<double> Y2(1, 3, 1, 1,
					y1.h, y2.h, y3.h);
		
		CImg<double> b = Y1.solve(X);
		this->a = b(0);
		this->b = b(1);
		this->c = b(2);

		b = Y2.solve(X);
		this->d = b(0);
		this->e = b(1);
		this->f = b(2);
		
	}
	void solveProjective(point &x1, point &x2, point &x3, point &x4, point &y1, point &y2, point &y3, point &y4) {
		double r = 0;
		CImg<double> X(8, 8, 1, 1, 
					x1.w+r, x1.h, 1.0, 0.0, 0.0, 0.0, -x1.w*y1.w, -x1.h*y1.w,
					0.0, r, 0.0, x1.w, x1.h, 1.0, -x1.w*y1.h, -x1.h*y1.h,
					x2.w, x2.h, 1.0+r, 0.0, 0.0, 0.0, -x2.w*y2.w, -x2.h*y2.w,
					0.0, 0.0, 0.0, x2.w+r, x2.h, 1.0, -x2.w*y2.h, -x2.h*y2.h,
					x3.w, x3.h, 1.0, 0.0, r, 0.0, -x3.w*y3.w, -x3.h*y3.w,
					0.0, 0.0, 0.0, x3.w, x3.h, 1.0+r, -x3.w*y3.h, -x3.h*y3.h,
					x4.w, x4.h, 1.0, 0.0, 0.0, 0.0, -x4.w*y4.w+r, -x4.h*y4.w,
					0.0, 0.0, 0.0, x4.w, x4.h, 1.0, -x4.w*y4.h, -x4.h*y4.h+r);
		CImg<double> Y(1, 8, 1, 1,
					y1.w, y1.h, y2.w, y2.h, y3.w, y3.h, y4.w, y4.h);
	
		CImg<double> b = Y.solve(X);
		this->a = b(0);
		this->b = b(1);
		this->c = b(2);

		this->d = b(3);
		this->e = b(4);
		this->f = b(5);

		this->g = b(6);
		this->h = b(7);	
	}
};
void getNSEW(vector<double> wp, vector<double> hp, point &nw, point &ne, point &sw, point &se) {
	double maxLen = 0;
	double minLen = INT_MAX;
	int order[4] = {0, 1, 2, 3};
	// Find Northwest point.
	for (int i = 0; i < 4; i++) {
		double len = pow(wp[i], 2) + pow(hp[i], 2);
		if (len < minLen) {
			minLen = len;
			nw.w = wp[i];
			nw.h = hp[i];
		}
	}
	// Find Other point.
	double len[4];
	for (int i = 0; i < 4; i++) {
		len[i] = pow(wp[i]-nw.w, 2) + pow(hp[i]-nw.h, 2);
		double tmp = len[i];
		int o = order[i];
		int j;
		for (j = i-1; j >= 0; j--) {
			if (tmp < len[j]) {
				len[j+1] = len[j];
				order[j+1] = order[j];
				if (j == 0) {
					len[0] = tmp;
					order[0] = o;
				}
			}
			else {
				len[j+1] = tmp;
				order[j+1] = o;
				break;
			}
		}
	}

	ne = point(wp[order[1]], hp[order[1]]);
	sw = point(wp[order[2]], hp[order[2]]);
	se = point(wp[order[3]], hp[order[3]]);
}
// Bilinear Filter
void getColor(CImg<unsigned char> *rgbImg, point &v, CImg<unsigned char> *warpedImg, int &w, int &h) {
	if (v.w < 0 || v.w+1 > rgbImg->width() || v.h < 0 || v.h+1 > rgbImg->height()) return;

	double a = v.w - floor(v.w);
	double b = v.h - floor(v.h);
	double nw, ne, sw, se, color;
	for (int c = 0; c < rgbImg->spectrum(); c++) {
		nw = (*rgbImg)(floor(v.w), floor(v.h), c);
		ne = (*rgbImg)(ceil(v.w), floor(v.h), c);
		sw = (*rgbImg)(floor(v.w), ceil(v.h), c);
		se = (*rgbImg)(ceil(v.w), ceil(v.h), c);

		color = (1-a)*(1-b)*nw + a*(1-b)*sw + (1-a)*b*ne + a*b*se;

		(*warpedImg)(w, h, c) = (unsigned char)color;
	}
}
void A4WarpingShear(CImg<unsigned char> *rgbImg, CImg<unsigned char> *warpedImg, vector<double> wp, vector<double> hp) {
	// Get four corners and warped paper size.
	point nw(0, 0), ne(0, 0), sw(0, 0), se(0, 0);
	getNSEW(wp, hp, nw, ne, sw, se);

	double height = sqrt(pow(nw.w-sw.w, 2) + pow(nw.h-sw.h, 2));
	double width = height*210/297;
	int h_size = ceil(height);
	int w_size = ceil(width);

	(*warpedImg) = CImg<unsigned char>(w_size, h_size, 1, rgbImg->spectrum());

	// find every point's color by interpolate to orgin image.
	point v1(ne.w - nw.w, ne.h - nw.h), v2(sw.w - nw.w, sw.h - nw.h);
	point v3(ne.w - se.w, ne.h - se.h), v4(sw.w - se.w, sw.h - se.h);
	point v_(0, 0);

	unsigned char rgb[3] = {0, 0, 0};
	double scale_w = 0, scale_h = 0;
	int crossLine = 0;
	for (int h = 0; h < h_size; h++) {
		crossLine = (double)w_size/h_size * (h_size - h);

		scale_h = (double)h/h_size;
		for (int w = 0; w < crossLine; w++) {
			scale_w = (double)w/w_size;
			v_.w = (scale_w*v1.w + scale_h*v2.w) + nw.w;
			v_.h = (scale_w*v1.h + scale_h*v2.h) + nw.h;
			getColor(rgbImg, v_, warpedImg, w, h);
		}

		scale_h = (double)(h_size - h)/h_size;
		for (int w = crossLine; w < w_size; w++) {
			scale_w = (double)(w_size - w)/w_size;
			v_.w = (scale_h*v3.w + scale_w*v4.w) + se.w;
			v_.h = (scale_h*v3.h + scale_w*v4.h) + se.h;
			getColor(rgbImg, v_, warpedImg, w, h);
		}
	}
}
void A4WarpingProjective(CImg<unsigned char> *rgbImg, CImg<unsigned char> *warpedImg, vector<double> wp, vector<double> hp) {
	// Get four corners and warped paper size.
	point nw, ne, sw, se;
	getNSEW(wp, hp, nw, ne, sw, se);
	if (ne.h > sw.h) {
		swap(nw, ne);
		swap(sw, se);
	}


	double height = sqrt(pow(nw.w-sw.w, 2) + pow(nw.h-sw.h, 2));
	double width = sqrt(pow(nw.w-ne.w, 2) + pow(nw.h-ne.h, 2));

	// initial result point
	point nw_(0, 0), ne_(width, 0), sw_(0, height), se_(width, height);
	trans_matrix T;
	T.solveProjective(nw_, ne_, sw_, se_, nw, ne, sw, se);
	// T.c += nw.w;
	// T.f += nw.h;

	// initial result image
	int h_size = ceil(height);
	int w_size = ceil(width);
	(*warpedImg) = CImg<unsigned char>(w_size, h_size, 1, rgbImg->spectrum());

	int crossLine = 0;
	for (int h = 0; h < h_size; h++) {
		for (int w = 0; w < w_size; w++) {
			point f_point = T.mul(point(w, h));
			getColor(rgbImg, f_point, warpedImg, w, h);
		}
	}
}

#endif