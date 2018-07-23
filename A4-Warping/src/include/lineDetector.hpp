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

// hough line space.
class houghLineSpace {
    private:
		struct houghPeek {
			double radius;
			double theta;
			int value;
			int vote;
			bool operator < (const houghPeek &h) const{
				return value > h.value;
			}
			houghPeek(double r, double t, double v) {
				radius = r;
				theta = t;
				value = v;
				vote = 0;
			}
		};
		struct crossPoint {
			double width;
			double height;
			crossPoint(double w, double h) {
				width = w;
				height = h;
			}
		};
		priority_queue<houghPeek>  peeks;
		vector<houghPeek> lines;
		vector<crossPoint> cPoints;
		int maxPLen;
		int radiusSize;	/* radius range (imge cross line length) */
		int thetaSize;	/* theta range (0-360) */
		double scaleR;	/* radius's scale */
		double scaleT;	/* theta's scale */
		unsigned char *buffer;	/* hough space buffer */
		int width;
		int height;
    public:
		houghLineSpace(int radiusSize, int thetaSize=360, int scaleR=1, int scaleT=1, int maxPLen = 16) {
			this->radiusSize = radiusSize;
			this->thetaSize = thetaSize;
			this->scaleR = scaleR;
			this->scaleT = scaleT;
			this->maxPLen = maxPLen;

			// malloc buffer memory
			int bufferSize = (int)(radiusSize*scaleR) * (int)(thetaSize*scaleT) * sizeof(int);
			buffer = (unsigned char *)malloc(bufferSize);
			memset(buffer, 0, bufferSize);
		}
		void cleanHough() {
			int bufferSize = (int)(radiusSize*scaleR) * (int)(thetaSize*scaleT) * sizeof(unsigned char);
			memset(buffer, 0, bufferSize);
		}
		void convertImg(unsigned char *data, int width, int height) {
			// for every pixel with a value bigger than zero, print its correlate hough space pixels.
			for (int h = 0; h < height; h++) {
				for (int w = 0; w < width; w++) {
					int index = h*width + w;
					if (data[index] > 0) {
						updateHough(w, h);
					}
				}
			}
			this->width = width;
			this->height = height;
			clusterLines(data);
			updateCrossPoint();
		}
		// Update hough space by one pixel in image.
		void updateHough(double imgW, double imgH) {
			int rLen = (int)(radiusSize*scaleR);
			int tLen = (int)(thetaSize*scaleT);
			for (int t = 0; t < tLen; t++) {
				double theta = t/scaleT;
				double radius = imgW*cos(theta) + imgH*sin(theta);
				int r = (int)(radius*scaleR);
				int index = r*tLen + t;
				if (r >= 0 && buffer[index] < 255) {
					buffer[index]++;
				}
			}
		}
		// Update peeks
		void updatePeeks() {
			int rLen = (int)(radiusSize*scaleR);
			int tLen = (int)(thetaSize*scaleT);
			for (int t = 0; t < tLen; t++) {
				for (int r = 0; r < rLen; r++) { 
					int index = r*tLen + t;
					if (buffer[index] >= 50) {
						double theta = t/scaleT;
						double radius = r/scaleR;
						peeks.push(houghPeek(radius, theta, buffer[index]));
						if (peeks.size() > maxPLen) peeks.pop();
					}
				}
			}
		}
		void clusterLines(unsigned char *data) {
			// collect all peeks
			updatePeeks();

			// cluster the closer line.
			priority_queue<houghPeek> peeks = this->peeks;
			vector<int> vote;
			while(!peeks.empty()) {
				houghPeek pTop = peeks.top();
				peeks.pop();
				lines.push_back(pTop);
			}

			//calculate point near the line.
			int biasLen = 30;
			for (int i = 0; i < lines.size(); i++) {
				// count point.
				double sinT = sin(lines[i].theta);
				double cosT = cos(lines[i].theta);
				double r = lines[i].radius;
				if (sinT != 0) {
					for (int w = 0; w < width; w++) {
						int h = (r - w*cosT) / sinT;
						if (h >= 0 && h < height) {
							// fine point near the (w, h)
							voteLine(data, i, w, h, cosT, sinT, biasLen);
						}
					}
				}
				if (cosT != 0) {
					for (int h = 0; h < height; h++) {
						int w = (r - h*sinT) / cosT;
						if (w >= 0 && w < width) {
							voteLine(data, i, w, h, cosT, sinT, biasLen);
						}
					}
				}
				// remove less vote line.
				if (lines[i].vote < 100) {
					lines.erase(lines.begin()+i);
					i--;
				}
			}
			
			// remove some line if the number of lines more than 4
			if (lines.size() > 4) {
				int wHalf = width/2;
				int hHalf = height/2;
				double maxR = 0;
				int indexL;
				for (int l = 0; l < lines.size(); l++) {
					double sinT = sin(lines[l].theta);
					double cosT = cos(lines[l].theta);
					double r = lines[l].radius;
					double dr = abs(sinT*hHalf + cosT*wHalf - r);
					if (dr > maxR) {
						indexL = l;
						maxR = dr;
					}
				}
				lines.erase(lines.begin()+indexL);
			}
		}
		// calculate cross points
		void updateCrossPoint() {
			cPoints.clear();
			// cal line1 and line2 cross line
			for (int l1 = 0; l1 < lines.size(); l1++) {
				for (int l2 = l1+1; l2 < lines.size(); l2++) {
					// cross point on line1.
					double t1 = lines[l1].theta,
							t2 = lines[l2].theta,
							r1 = lines[l1].radius,
							r2 = lines[l2].radius;

					double w1 = abs((sin(t1)*r2 - sin(t2)*r1) / sin(t1-t2));
					double h1 = abs((cos(t2)*r1 - cos(t1)*r2) / sin(t1-t2));
					if (w1 >= 0 && w1 < width && h1 >= 0 && h1 < height)
						cPoints.push_back(crossPoint(w1, h1));
				}
			}
		}
		void voteLine(unsigned char* data, int i, int w, int h, double cosT, double sinT, int biasLen) {
			// fine point near the (w, h)
			for (int b = -biasLen; b <= biasLen; b++) {
				int hNear = h + b*sinT;
				int wNear = w + b*cosT;
				if (hNear >= 0 && hNear < height && wNear >= 0 && wNear < width) {
					int index = hNear*width + wNear;
					if (data[index] > 0) {
						data[index] = 0;
						lines[i].vote++;
					}
				}
			}
		}
		void displayHough() {
			CImg<unsigned char>(buffer, (int)(thetaSize*scaleT), (int)(radiusSize*scaleR), 1, 1).display();
		}
		void getCrossPoint(vector<double> *w_point, vector<double> *h_point, CImg<unsigned char> *rgbImg) {
			double scaleW = (double)(width)/rgbImg->width();
			double scaleH = (double)(height)/rgbImg->height();
			for (int i = 0; i < cPoints.size(); i++) {
				w_point->push_back(cPoints[i].width / scaleW);
				h_point->push_back(cPoints[i].height / scaleH);
			}
		}
		void displayLine(const CImg<unsigned char> *originImg) {
			CImg<unsigned char> img = CImg<unsigned char>(*originImg);
			int widthI = img.width();
			int heightI = img.height();
			double scaleW = (double)(width)/widthI;
			double scaleH = (double)(height)/heightI;

			// Print red line.
			int margin = 100;
			for (int p = 0; p < lines.size(); p++) {
				printf("%d %f %f %d %d\n\n", p, lines[p].theta, lines[p].radius, lines[p].value, lines[p].vote);
				double sinT = scaleH * sin(lines[p].theta);
				double cosT = scaleW * cos(lines[p].theta);
				double r = lines[p].radius;
				// Print line.
				if (sinT != 0) {
					for (int w = 0; w < widthI; w++) {
						int h = (r - w*cosT) / sinT;
						if (h >= 0 && h < heightI) {
							for (int b = -margin; b <= margin; b++) {
								int wi = w + cosT*b;
								int hi = h + sinT*b;
								if (wi >= 0 && wi < widthI && hi >= 0 && hi < heightI) {
									img(wi, hi, 0) = 255;
									img(wi, hi, 1) = 0;
									img(wi, hi, 2) = 0;
								}
							}
						}
					}
				}
				if (cosT != 0) {
					for (int h = 0; h < heightI; h++) {
						int w = (r - h*sinT) / cosT;
						if (w >= 0 && w < widthI) {
							for (int b = -margin; b <= margin; b++) {
								int wi = w + cosT*b;
								int hi = h + sinT*b;
								if (wi >= 0 && wi < widthI && hi >= 0 && hi < heightI) {
									img(wi, hi, 0) = 255;
									img(wi, hi, 1) = 0;
									img(wi, hi, 2) = 0;
								}
							}
						}
					}
				}
			}

			// print cross points
			for (int p = 0; p < cPoints.size(); p++){
				int w = cPoints[p].width / scaleW;
				int h = cPoints[p].height / scaleH;
				for (int wb = -margin/4; wb <= margin/4; wb++) {
					for (int hb = -margin/4; hb <= margin/4; hb++) {
						int wi = w + wb;
						int hi = h + hb;
						if (wi >= 0 && wi < widthI && hi >= 0 && hi < heightI) {
							img(wi, hi, 0) = 0;
							img(wi, hi, 1) = 0;
							img(wi, hi, 2) = 255;
						}
					}
				}
			}
			img.display();
		}
		void printResult() {
			printf("\n[ Lines formulation ]\n");
			for (int i = 0; i < lines.size(); i++) {
				double sinT = sin(lines[i].theta);
				double cosT = cos(lines[i].theta);
				printf("line %d : (%+f·w %+f·h = %f)\n", i+1, cosT, sinT, lines[i].radius);
			}
			printf("\n[ Cross points position (w, h) ]\n");
			for (int i = 0; i < cPoints.size(); i++) {
				printf("cross point %d : (%f, %f)\n", i+1, cPoints[i].width, cPoints[i].height);
			}
			printf("\n");
		}
		~houghLineSpace() {
			free(buffer);
		}
};



// line detector
class lineDetector {
	private:
		double scale;
		int height, width;
		unsigned char* data;
		vector<double> w_point;
		vector<double> h_point;
		void toGray(unsigned char* data, CImg<unsigned char> *rgbImg) {
			/* convert rgb image to gray linear data */
			CImg<unsigned char> img = (*rgbImg);
			img.resize(width, height);
			for (int h = 0; h < height; h++) {
				for (int w = 0; w < width; w++) {
					int index = h*width + w;
					data[index] = (img(w, h, 0) + img(w, h, 1) + img(w, h, 2))/3;
				}
			}
		}
		void toEdge(unsigned char* data) {
			// detect image edge.
			cannylit myCanny(data, width, height, 2, 6, 3.5, 16);
			memcpy(data, myCanny.edgeData(), width * height);
		}
	public:
		lineDetector(CImg<unsigned char> *rgbImg) {
			this->scale = 450.0/(double)(rgbImg->height());
			if (rgbImg->height() < rgbImg->width())
				this->scale = 450.0/(double)(rgbImg->width());
			
			width = rgbImg->width()*scale;
			height = rgbImg->height()*scale;
			data = (unsigned char*)malloc(height * width * sizeof(unsigned char));
			memset(data, 0, height * width * sizeof(unsigned char));

			// preprocess the image.
			toGray(data, rgbImg);
			toEdge(data);

			int maxRadius = ceil(sqrt(width*width + height*height));
			houghLineSpace hSpace(maxRadius, 360, 2, 3);
			// CImg<unsigned char>(data, width, height).display();
			hSpace.convertImg(data, width, height);
			
			hSpace.getCrossPoint(&w_point, &h_point, rgbImg);
			// hSpace.displayLine(rgbImg);
			// hSpace.printResult();

			// printf("display....\n");
			// hSpace.displayHough();
		}
		void getCrossPoint(vector<double> *wp, vector<double> *hp) {
			for (int i = 0; i < 4; i++) {
				wp->push_back(w_point[i]);
				hp->push_back(h_point[i]);
			}
		}
		~lineDetector() {
			free(this->data);
		}
};