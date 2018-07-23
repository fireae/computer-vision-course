#ifndef TRANS_MATRIX_HPP
#define TRANS_MATRIX_HPP
#include <CImg.h>
#include <cmath>
#include "point.hpp"
using namespace cimg_library;
struct trans_matrix {
	double a, b, c;
	double d, e, f;
	double g, h, i;
	trans_matrix(double a=1, double b=0, double c=0, 
	double d=0, double e=1, double f=0, 
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
	mPoint mul(mPoint x) {
		mPoint p;
		double z = x.x*g + x.y*h + i;
		p.x = (x.x*a + x.y*b + c) / z;
		p.y = (x.x*d + x.y*e + f) / z;
		return p;
	}
	trans_matrix invert() {
		CImg<double> A(3, 3, 1, 1,
			this->a, this->b, this->c, 
			this->d, this->e, this->f,
			this->g, this->h, this->i);
		A = A.invert();
		return trans_matrix(A(0), A(1), A(2), A(3), A(4), A(5), A(6), A(7), A(8));
	}
	trans_matrix mul(trans_matrix m) {
		CImg<double> A(3, 3, 1, 1,
			this->a, this->b, this->c, 
			this->d, this->e, this->f,
			this->g, this->h, this->i);
		CImg<double> B(3, 3, 1, 1,
			m.a, m.b, m.c, 
			m.d, m.e, m.f,
			m.g, m.h, m.i);
		A *= B;
		return trans_matrix(A(0), A(1), A(2), A(3), A(4), A(5), A(6), A(7), A(8));
	}
	double dist(trans_matrix m) {
		return sqrt(pow(this->a-m.a, 2) + pow(this->b-m.b, 2) + pow(this->c-m.c, 2)+ 
			pow(this->d-m.d, 2) + pow(this->e-m.e, 2) + pow(this->f-m.f, 2)+
			pow(this->g-m.g, 2) + pow(this->h-m.h, 2) + pow(this->i-m.i, 2));
	}
	bool equal(trans_matrix m) {
		return this->dist(m) < 5;
		// printf("%f ", f);
		// return f < 20;
	}
	void solveAffine(mTriangle &tx, mTriangle &ty) {
		solveAffine(tx.v1, tx.v2, tx.v3, ty.v1, ty.v2, ty.v3);
	}
	void solveAffine(mPoint &x1, mPoint &x2, mPoint &x3, mPoint &y1, mPoint &y2, mPoint &y3) {
		CImg<double> X(3, 3, 1, 1, 
					x1.x, x1.y, 1.0,
					x2.x, x2.y, 1.0,
					x3.x, x3.y, 1.0);
		CImg<double> Y1(1, 3, 1, 1,
					y1.x, y2.x, y3.x);
		CImg<double> Y2(1, 3, 1, 1,
					y1.y, y2.y, y3.y);
		
		CImg<double> b = Y1.solve(X);
		this->a = b(0);
		this->b = b(1);
		this->c = b(2);

		b = Y2.solve(X);
		this->d = b(0);
		this->e = b(1);
		this->f = b(2);
	}
	void solveProjective(mPoint &x1, mPoint &x2, mPoint &x3, mPoint &x4, mPoint &y1, mPoint &y2, mPoint &y3, mPoint &y4) {
		double r = 0;
		CImg<double> X(8, 8, 1, 1, 
					x1.x+r, x1.y, 1.0, 0.0, 0.0, 0.0, -x1.x*y1.x, -x1.y*y1.x,
					0.0, r, 0.0, x1.x, x1.y, 1.0, -x1.x*y1.y, -x1.y*y1.y,
					x2.x, x2.y, 1.0+r, 0.0, 0.0, 0.0, -x2.x*y2.x, -x2.y*y2.x,
					0.0, 0.0, 0.0, x2.x+r, x2.y, 1.0, -x2.x*y2.y, -x2.y*y2.y,
					x3.x, x3.y, 1.0, 0.0, r, 0.0, -x3.x*y3.x, -x3.y*y3.x,
					0.0, 0.0, 0.0, x3.x, x3.y, 1.0+r, -x3.x*y3.y, -x3.y*y3.y,
					x4.x, x4.y, 1.0, 0.0, 0.0, 0.0, -x4.x*y4.x+r, -x4.y*y4.x,
					0.0, 0.0, 0.0, x4.x, x4.y, 1.0, -x4.x*y4.y, -x4.y*y4.y+r);
		CImg<double> Y(1, 8, 1, 1,
					y1.x, y1.y, y2.x, y2.y, y3.x, y3.y, y4.x, y4.y);
	
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