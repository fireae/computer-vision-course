#include "cannylit.hpp"
#include "../lib/CImg.h"
#include<string>
#include<queue>
#include<vector>
#include<algorithm>
#include<cmath>
#include<cstdlib>
using namespace std;
using namespace cimg_library;

// hough Circle space.
class houghCircleSpace {
    private:
		struct houghPeek {
            double wc;
			double hc;
			double radius;
			int value;
			int vote;
			bool operator < (const houghPeek &h) const{
				return value > h.value;
			}
			houghPeek(double w, double h, double r, double v) {
				wc = w;
				hc = h;
                radius = r;
				value = v;
				vote = 0;
			}
		};
		
		priority_queue<houghPeek>  peeks;
		vector<houghPeek> circles;
		int maxPLen;
        int wcSize;    /* the length of circle center vector*/
		int hcSize;	/* theta range (0-360) */
		int radiusSize;	/* radius of circle */
        double scaleC;
		double scaleR;	/* radius's scale */
		unsigned char *buffer;	/* hough space buffer */
		int width;  /* width of input image */
		int height;     /* height of input image */
    public:
		houghCircleSpace(int wcSize, int hcSize, int radiusSize, double scaleC=1, double scaleR=1, int maxPLen = 60) {
			this->wcSize = wcSize;
			this->hcSize = hcSize;
            this->radiusSize = radiusSize;
            this->scaleC = scaleC;
			this->scaleR = scaleR;
			this->maxPLen = maxPLen;

			// malloc buffer memory
			int bufferSize = (int)(scaleC*hcSize) * (int)(scaleC*wcSize) * (int)(radiusSize*scaleR) * sizeof(int);
			buffer = (unsigned char *)malloc(bufferSize);
			memset(buffer, 0, bufferSize);
		}
		void cleanHough() {
			int bufferSize = (int)(wcSize*scaleC) * (int)(hcSize*scaleC) * (int)(radiusSize*scaleR) * sizeof(unsigned char);
			memset(buffer, 0, bufferSize);
		}
		void convertImg(unsigned char *data, int width, int height) {
			// for every pixel with a value bigger than zero, print its correlate hough space pixels.
			this->width = width;
			this->height = height;
			for (int h = 0; h < height; h++) {
				for (int w = 0; w < width; w++) {
					int index = h*width + w;
					if (data[index] > 0) {
						updateHough(w, h);
					}
				}
			}
			printf("cluster\n\n");
			clusterCircles(data);
		}
		// Update hough space by one pixel in image.
		void updateHough(double imgW, double imgH) {
            int wLen = (int)(wcSize*scaleC);
			int hLen = (int)(hcSize*scaleC);
			int rLen = (int)(radiusSize*scaleR);

			int wBegin = imgW*scaleC - rLen,
				wEnd = imgW*scaleC + rLen,
				hBegin = imgH*scaleC - rLen,
				hEnd =  imgH*scaleC + rLen;
			wBegin = max(wBegin, (int)(0.2*wLen));
			wEnd = min(wEnd, (int)(0.8*wLen));
			hBegin = max(hBegin, (int)(0.2*hLen));
			hEnd = min(hEnd, (int)(0.8*hLen));

            for (int w = wBegin; w <= wEnd; w++) {
				for (int h = hBegin; h <= hEnd; h++) {
					double wc_double = w/scaleC;
					double hc_double = h/scaleC;
					double radius = sqrt(pow(imgW - wc_double, 2) + pow(imgH - hc_double, 2));
					if (radius < 0.07*radiusSize || radius > radiusSize) continue;
					
					if (wc_double - radius >= 0
						&& wc_double + radius < width
						&& hc_double - radius >= 0
						&& hc_double + radius < height) {
						int r = (int)(radius*scaleR);
						int index = w*hLen*rLen + h*rLen + r;
						if (r >= 0 && buffer[index] < 255) {
							buffer[index]++;
						}
					}
				}
            }
		}
		// Update peeks
		void updatePeeks() {
            int wLen = (int)(wcSize*scaleC);
			int hLen = (int)(hcSize*scaleC);
			int rLen = (int)(radiusSize*scaleR);
            for (int w = 0.2*wLen; w < 0.8*wLen; w++) {
                for (int h = 0.2*hLen; h < 0.8*hLen; h++) {
                    for (int r = 0; r < rLen; r++) { 
                        int index = w*hLen*rLen + h*rLen + r;
                        if (buffer[index] >= 20 && (peeks.empty() || buffer[index] > peeks.top().value)) {
                            double w_p = w/scaleC;
                            double h_p = h/scaleC;
                            double radius = r/scaleR;
                            peeks.push(houghPeek(w_p, h_p, radius, buffer[index]));
                            if (peeks.size() > maxPLen) peeks.pop();
                        }
                    }
                }
            }
		}
		void clusterCircles(unsigned char *data) {
			// collect all peeks
			updatePeeks();
            // Convert queue data to vector.
			priority_queue<houghPeek> peeks = this->peeks;
			vector<int> vote;
			while(!peeks.empty()) {
				houghPeek pTop = peeks.top();
				peeks.pop();
				circles.push_back(pTop);
			}

			//calculate point near the Circle.
			int biasLen = 3;
			for (int i = 0; i < circles.size(); i++) {
				// count point.
				double r = circles[i].radius;
				double r2 = pow(r, 2);
                double wc = circles[i].wc;
				double hc = circles[i].hc;
                for (int w = wc - r; w <= wc + r; w++) {
					if (w >= 0 && w < width) {
						int h1 = sqrt(r2 - pow(w-wc, 2)) + hc;
						int h2 = -sqrt(r2 - pow(w-wc, 2)) + hc;
						if (h1 >= 0 && h1 < height) {
							// fine point near the (w, h)
							voteCircle(data, i, w, h1, biasLen);
						}
						if (h2 >= 0 && h2 < height) {
							// fine point near the (w, h)
							double r = sqrt(w*w + h2*h2);
							voteCircle(data, i, w, h2, biasLen);
						}
					}
                }

                for (int h = hc - r; h <= hc + r; h++) {
					if (h >= 0 && h < height) {
						int w1 = sqrt(r2 - pow(h-hc, 2)) + wc;
						int w2 = -sqrt(r2 - pow(h-hc, 2)) + wc;
						if (w1 >= 0 && w1 < width) {
							voteCircle(data, i, w1, h, biasLen);
						}
						if (w2 >= 0 && w2 < width) {
							voteCircle(data, i, w2, h, biasLen);
						}
					}
                }
				// remove less vote Circle.
				if (circles[i].vote < 500) {
					circles.erase(circles.begin()+i);
					i--;
				}
			}

			// make cluster
			vector<houghPeek> centers;
			for (int i = 0; i < circles.size(); i++) {
				if (centers.empty()) {
					centers.push_back(circles[i]);
					continue;
				}
				bool collected = false;
				double minErr = 1000;
				int minJ = 0;
				for (int j = 0; j < centers.size(); j++) {
					int dw = circles[i].wc - centers[j].wc;
					int dh = circles[i].hc - centers[j].hc;
					double err = sqrt(pow(dw, 2) + pow(dh, 2));
					if (err < minErr) {
						minErr = err;
						minJ = j;
					}
				}
				if (minErr < 30) centers[minJ] = circles[i];
				else centers.push_back(circles[i]);
			}
			circles = centers;
		}
	
		void voteCircle(unsigned char* data, int i, int w, int h, int biasLen) {
			// fine point near the (w, h)
			for (int bw = -biasLen; bw <= biasLen; bw++) {
                for (int bh = -biasLen; bh <= biasLen; bh++) {
                    int wNear = w + bw;
                    int hNear = h + bh;
                    if (hNear >= 0 && hNear < height && wNear >= 0 && wNear < width) {
                        int index = hNear*width + wNear;
                        if (data[index] > 0) {
                            // data[index] = 0;
                            circles[i].vote++;
                        }
                    }
                }
			}
		}
		void displayHough() {
			CImg<unsigned char>(buffer, (int)(hcSize*scaleC), (int)(wcSize*scaleC), 1, 1).display();
		}
		void displayCircle(const CImg<unsigned char> *originImg) {
			CImg<unsigned char> img = CImg<unsigned char>(*originImg);
			int widthI = img.width();
			int heightI = img.height();
			double scaleW = (double)(width)/widthI;
			double scaleH = (double)(height)/heightI;

			// Print red Circle.
			int margin = 2 / scaleW;
			for (int p = 0; p < circles.size(); p++) {
				double r = circles[p].radius;
				double r2 = pow(circles[p].radius, 2);
                double wc = circles[p].wc;
				double hc = circles[p].hc;
                for (int w = (wc - r)/scaleW; w <= (wc + r)/scaleW; w++) {
					if (w >= 0 && w < widthI) {
						int h1 = (sqrt(r2 - pow(scaleW*w-wc, 2)) + hc) / scaleH;
						int h2 = (-sqrt(r2 - pow(scaleW*w-wc, 2)) + hc) / scaleH;
						if (h1 >= 0 && h1 < heightI) {
							// fine point near the (w, h)
							printPoint(w, h1, img, margin);
						}
						if (h2 >= 0 && h2 < heightI) {
							// fine point near the (w, h)
							printPoint(w, h2, img, margin);
						}
					}
                }

                for (int h = (hc - r)/scaleH; h <= (hc + r)/scaleH; h++) {
					if (h >= 0 && h < heightI) {
						int w1 = (sqrt(r2 - pow(scaleH*h-hc, 2)) + wc) / scaleW;
						int w2 = (-sqrt(r2 - pow(scaleH*h-hc, 2)) + wc) / scaleW;
						if (w1 >= 0 && w1 < widthI) {
							printPoint(w1, h, img, margin);
						}
						if (w2 >= 0 && w2 < widthI) {
							printPoint(w2, h, img, margin);
						}
					}
                }
			}		
			img.display();
		}
        void printPoint(int w, int h, CImg<unsigned char>& img, int margin=2) {
            for (int bh = -margin; bh <= margin; bh++) {
                for (int bw = -margin; bw <= margin; bw++) {
                    int wi = w + bw;
                    int hi = h + bh;
                    if (wi >= 0 && hi >= 0 && wi < img.width() && hi < img.height()) {
                        img(wi, hi, 0) = 255;
                        img(wi, hi, 1) = 0;
                        img(wi, hi, 2) = 0;
                    }
                }
            }
        }
		void printResult() {
			printf("\n[ circles counter ]\n");
			printf("\n Counter finds %lu circle(s).\n\n", circles.size());
		}
		~houghCircleSpace() {
			free(buffer);
		}
};



// Circle detector
class circleDetector {
	private:
		double scale;
		int height, width;
		unsigned char* data;
		CImg<unsigned char> toGray(CImg<unsigned char> *rgbImg) {
			/* convert rgb image to gray Circlear data */
			double scale = 1000.0/(double)(rgbImg->height());
			if (rgbImg->height() < rgbImg->width())
				scale = 1000.0/(double)(rgbImg->width());
			
			int width = rgbImg->width()*scale;
			int height = rgbImg->height()*scale;

			CImg<unsigned char> img = (*rgbImg);
			img.resize(width, height);
			return (img.get_channel(0)*0.3 + img.get_channel(1)*0.59 + img.get_channel(2)*0.11);
		}
		void toEdge(unsigned char* data, CImg<unsigned char> *grayImg) {
			// detect image edge.
			cannylit myCanny(grayImg->data(), grayImg->width(), grayImg->height(), 0.1, 1.5, 7.6, 9);
			CImg<unsigned char> img = CImg<unsigned char>(myCanny.edgeData(), grayImg->width(), grayImg->height());
			img.resize(width, height);
			memcpy(data, img.data(), width * height);
		}
	public:
		circleDetector(CImg<unsigned char> *rgbImg) {
			// RGB to Gray
			CImg<unsigned char> grayImg = toGray(rgbImg);

			// scale grayImg
			this->scale = 500.0/(double)(grayImg.height());
			if (grayImg.height() < grayImg.width())
				this->scale = 500.0/(double)(grayImg.width());
			width = grayImg.width()*scale;
			height = grayImg.height()*scale;

			data = (unsigned char*)malloc(height * width * sizeof(unsigned char));
			memset(data, 0, height * width * sizeof(unsigned char));

			// preprocess the image.
			toEdge(data, &grayImg);

			int minRadius = width/2;
			if (minRadius > height/2) minRadius = height/2;
			houghCircleSpace hSpace(width, height, 0.7*minRadius);
			CImg<unsigned char>(data, width, height).display();
			hSpace.convertImg(data, width, height);
			hSpace.displayCircle(rgbImg);
			hSpace.printResult();
			// hSpace.displayHough();
		}
		~circleDetector() {
			free(this->data);
		}
};