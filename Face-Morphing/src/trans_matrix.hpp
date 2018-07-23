#ifndef TRANS_MATRIX_HPP
#define TRANS_MATRIX_HPP
#include "./facePose.hpp"
#include "lib/CImg.h"

using namespace cimg_library;
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
	pose mul(pose x) {
		pose p;
		double z = x.w*g + x.h*h + i;
		p.w = (x.w*a + x.h*b + c) / z;
		p.h = (x.w*d + x.h*e + f) / z;
		return p;
	}
	void solveAffine(triangle &tx, triangle &ty) {
		solveAffine(tx.v1, tx.v2, tx.v3, ty.v1, ty.v2, ty.v3);
	}
	void solveAffine(pose &x1, pose &x2, pose &x3, pose &y1, pose &y2, pose &y3) {
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
	void solveProjective(pose &x1, pose &x2, pose &x3, pose &x4, pose &y1, pose &y2, pose &y3, pose &y4) {
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

#endif