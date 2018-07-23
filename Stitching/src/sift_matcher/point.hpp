#ifndef POINT_HPP
#define POINT_HPP

struct mPoint{
    double x;
    double y;
    mPoint(double x = 0, double y = 0) {
        this->x = x;
        this->y = y;
    }
};

struct mPoint_pair{
    mPoint p1;
    mPoint p2;
    mPoint_pair(mPoint p1, mPoint p2) {
        this->p1 = p1;
        this->p2 = p2;
    }
    mPoint_pair(double x1 = 0, double y1 = 0, double x2 = 0, double y2 = 0) {
        this->p1 = mPoint(x1, y1);
        this->p2 = mPoint(x2, y2);
    }
};


struct mTriangle {
    mPoint v1, v2, v3;
    mTriangle(mPoint v1, mPoint v2, mPoint v3) {
        this->v1 = v1;
        this->v2 = v2;
        this->v3 = v3;
    }
};


#endif