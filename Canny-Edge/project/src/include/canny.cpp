#include "canny.hpp"


void canny::cannyProcess() {
    if (contrastNormalised) normalizeContrast();
    DoG_filter();
    NMS();
    HysteresisThreshold();
}
void canny::normalizeContrast() {   
    // Normalize the gray image.
    grayImg = grayImg.get_equalize(256);
}
void canny::DoG_filter() {
    // Derivative of Gaussian
    float *magnitude = (float *)malloc(width * height * sizeof(float));

    // Input buffer
    unsigned char *data = grayImg.data();

    // Initialise the Gaussian kernel and Difference kernel
    float *kernel = (float *)malloc(gaussianKernelWidth * sizeof(float));
    float *diffKernel = (float *)malloc(gaussianKernelWidth * sizeof(float));
    if(!kernel || !diffKernel)
        goto error_exit;

    int kwidth;
    for (kwidth = 0; kwidth < gaussianKernelWidth; kwidth++) 
    {
        float g1, g2, g3;
        g1 = gaussian((float) kwidth, gaussianKernelRadius);
        if (g1 <= GAUSSIAN_CUT_OFF && kwidth >= 2) 
            break;
        g2 = gaussian(kwidth - 0.5f, gaussianKernelRadius);
        g3 = gaussian(kwidth + 0.5f, gaussianKernelRadius);
        kernel[kwidth] = (g1 + g2 + g3) / 3.0f / (2.0f * (float) 3.14 * gaussianKernelRadius * gaussianKernelRadius);
        diffKernel[kwidth] = g3 - g2;
    }

    // Initial positions to start and to end.
    int initX;
    int maxX;
    int initY;
    int maxY;
    initX = kwidth - 1;
    maxX = width - (kwidth - 1);
    initY = width * (kwidth - 1);
    maxY = width * (height - (kwidth - 1));
    
    /* perform convolution with the Gaussian kernel in x and y directions */
    int x, y;
    int i;
    int flag; 
    for(x = initX; x < maxX; x++) 
    {
        for(y = initY; y < maxY; y += width) 
        {
            int index = x + y;
            float sumX = data[index] * kernel[0];
            float sumY = sumX;
            int xOffset = 1;
            int yOffset = width;
            while(xOffset < kwidth) 
            {
                sumY += kernel[xOffset] * (data[index - yOffset] + data[index + yOffset]);
                sumX += kernel[xOffset] * (data[index - xOffset] + data[index + xOffset]);
                yOffset += width;
                xOffset++;
            }
            
            this->yConv[index] = sumY;
            this->xConv[index] = sumX;
        }

    }
    
    /* Perform convolution with differentce kernel in x and y directions.
    * As a result, we get gradient in x and y directions.
    */
    for (x = initX; x < maxX; x++) 
    {
        for (y = initY; y < maxY; y += width) 
        {
            float sum = 0.0f;
            int index = x + y;
            for (i = 1; i < kwidth; i++)
                sum += diffKernel[i] * (this->yConv[index - i] - this->yConv[index + i]);

            this->xGradient[index] = sum;
        }

    }

    for(x = kwidth; x < width - kwidth; x++) 
    {
        for (y = initY; y < maxY; y += width) 
        {
            float sum = 0.0f;
            int index = x + y;
            int yOffset = width;
            for (i = 1; i < kwidth; i++) 
            {
                sum += diffKernel[i] * (this->xConv[index - yOffset] - this->xConv[index + yOffset]);
                yOffset += width;
            }

            this->yGradient[index] = sum;
        }

    }

    /* Compute the gradient magnitude. */
    initX = kwidth;
    maxX = width - kwidth;
    initY = width * kwidth;
    maxY = width * (height - kwidth);
    for(x = initX; x < maxX; x++) 
    {
        for(y = initY; y < maxY; y += width) 
        {
            
            int index = x + y;
            float xGrad = this->xGradient[index];
            float yGrad = this->yGradient[index];
            magnitude[index] = hypotenuse(xGrad, yGrad);
        }
    }

    // Get magImg.
    magImg = CImg<float>(magnitude, width, height, 1, 1);

    free(kernel);
    free(diffKernel);
    free(magnitude);
    return;
error_exit:
    free(kernel);
    free(diffKernel);
    free(magnitude);
    return;
}
void canny::NMS() {
    // Non-Maximal Suppression.
    int initX = gaussianKernelWidth;
    int maxX = width - gaussianKernelWidth;
    int initY = width * gaussianKernelWidth;
    int maxY = width * (height - gaussianKernelWidth);

    float *magData = magImg.data();
    float *onePixel = (float *)malloc(width * height * sizeof(float));

    int flag;

    for(int x = initX; x < maxX; x++) {
        for(int y = initY; y < maxY; y += width) {
            int index = x + y;
            int indexN = index - width;
            int indexS = index + width;
            int indexW = index - 1;
            int indexE = index + 1;
            int indexNW = indexN - 1;
            int indexNE = indexN + 1;
            int indexSW = indexS - 1;
            int indexSE = indexS + 1;

            /* perform non-maximal supression
            * Find magnitude at eight directions: n, s, w, e, ne, se, sw, nw
            */
            float nMag = hypotenuse(this->xGradient[indexN], this->yGradient[indexN]);
            float sMag = hypotenuse(this->xGradient[indexS], this->yGradient[indexS]);
            float wMag = hypotenuse(this->xGradient[indexW], this->yGradient[indexW]);
            float eMag = hypotenuse(this->xGradient[indexE], this->yGradient[indexE]);
            float neMag = hypotenuse(this->xGradient[indexNE], this->yGradient[indexNE]);
            float seMag = hypotenuse(this->xGradient[indexSE], this->yGradient[indexSE]);
            float swMag = hypotenuse(this->xGradient[indexSW], this->yGradient[indexSW]);
            float nwMag = hypotenuse(this->xGradient[indexNW], this->yGradient[indexNW]);
            float tmp;
            /*
            * An explanation of what's happening here, for those who want
            * to understand the source: This performs the "non-maximal
            * supression" phase of the Canny edge detection in which we
            * need to compare the gradient magnitude to that in the
            * direction of the gradient; only if the value is a local
            * maximum do we consider the point as an edge candidate.
            * 
            * We need to break the comparison into a number of different
            * cases depending on the gradient direction so that the
            * appropriate values can be used. To avoid computing the
            * gradient direction, we use two simple comparisons: first we
            * check that the partial derivatives have the same sign (1)
            * and then we check which is larger (2). As a consequence, we
            * have reduced the problem to one of four identical cases that
            * each test the central gradient magnitude against the values at
            * two points with 'identical support'; what this means is that
            * the geometry required to accurately interpolate the magnitude
            * of gradient function at those points has an identical
            * geometry (upto right-angled-rotation/reflection).
            * 
            * When comparing the central gradient to the two interpolated
            * values, we avoid performing any divisions by multiplying both
            * sides of each inequality by the greater of the two partial
            * derivatives. The common comparand is stored in a temporary
            * variable (3) and reused in the mirror case (4).
            * 
            */
            float xGrad = this->xGradient[index];
            float yGrad = this->yGradient[index];
            float gradMag = magData[index];
            flag = ( (xGrad * yGrad <= 0.0f) /*(1)*/
                ? ffabs(xGrad) >= ffabs(yGrad) /*(2)*/
                    ? (tmp = ffabs(xGrad * gradMag)) >= ffabs(yGrad * neMag - (xGrad + yGrad) * eMag) /*(3)*/
                        && tmp > fabs(yGrad * swMag - (xGrad + yGrad) * wMag) /*(4)*/
                    : (tmp = ffabs(yGrad * gradMag)) >= ffabs(xGrad * neMag - (yGrad + xGrad) * nMag) /*(3)*/
                        && tmp > ffabs(xGrad * swMag - (yGrad + xGrad) * sMag) /*(4)*/
                : ffabs(xGrad) >= ffabs(yGrad) /*(2)*/
                    ? (tmp = ffabs(xGrad * gradMag)) >= ffabs(yGrad * seMag + (xGrad - yGrad) * eMag) /*(3)*/
                        && tmp > ffabs(yGrad * nwMag + (xGrad - yGrad) * wMag) /*(4)*/
                    : (tmp = ffabs(yGrad * gradMag)) >= ffabs(xGrad * seMag + (yGrad - xGrad) * sMag) /*(3)*/
                        && tmp > ffabs(xGrad * nwMag + (yGrad - xGrad) * nMag) /*(4)*/
                );
            if(flag)
            {
                onePixel[index] = (gradMag >= MAGNITUDE_LIMIT) ? MAGNITUDE_MAX : (int) (MAGNITUDE_SCALE * gradMag);
                // onePixel[index] = 1;
                /*NOTE: The orientation of the edge is not employed by this
                implementation. It is a simple matter to compute it at
                this point as: Math.atan2(yGrad, xGrad); */
            } 
            else 
            {
                onePixel[index] = 0;
            }
        }
    }
    // Get one pixel image. 
    onePixelImg = CImg<float>(onePixel, width, height, 1, 1);
    free(onePixel);
}
void canny::HysteresisThreshold() {
    // Hysteresis Threshold.
    int offset = 0;
    int x, y;
    float *onePixel = onePixelImg.data();

    memset(this->idata, 0, this->width * this->height * sizeof(int));
            
    for(y = 0; y < this->height; y++)
    {
        for(x = 0; x < this->width; x++)
        {
            if(this->idata[offset] == 0 && onePixel[offset] >= this->high) 
                follow(onePixel, x, y, offset, this->low);
            offset++;
        }
    } 
    // Convert to 1 and 0.
    for (int i = 0; i < width*height; i++)
        this->idata[i] = this->idata[i]>0 ? 255 : 0;
    
    edgeImg = CImg<unsigned char>(this->idata, width, height, 1, 1);
}
/*
recursive portion of edge follower 
*/
void canny::follow(float *onePixel, int x1, int y1, int i1, int threshold) 
{
    int x, y;
    int x0 = x1 == 0 ? x1 : x1 - 1;
    int x2 = x1 == this->width - 1 ? x1 : x1 + 1;
    int y0 = y1 == 0 ? y1 : y1 - 1;
    int y2 = y1 == this->height -1 ? y1 : y1 + 1;
            
    this->idata[i1] = onePixel[i1];
    for (x = x0; x <= x2; x++) {
        for (y = y0; y <= y2; y++) 
        {
        int i2 = x + y * this->width;
        if ((y != y1 || x != x1) && this->idata[i2] == 0 && onePixel[i2] >= threshold) 
            follow(onePixel, x, y, i2, threshold);
        }
    }
}

/*
buffer allocation
*/
void canny::allocatebuffers() {   
    this->idata = (int *)malloc(width * height * sizeof(int));
    this->xConv = (float *)malloc(width * height * sizeof(float));
    this->yConv = (float *)malloc(width * height * sizeof(float));
    this->xGradient = (float *)malloc(width * height * sizeof(float));
    this->yGradient = (float *)malloc(width * height * sizeof(float));
    if(!this->idata || 
        !this->xConv || !this->yConv || 
        !this->xGradient || !this->yGradient)
        goto error_exit;
    return;
error_exit:
    killbuffers();
    return;
}

/*
buffers destructor
*/
void canny::killbuffers() {
    free(this->idata);
    free(this->xConv);
    free(this->yConv);
    free(this->xGradient);
    free(this->yGradient);
}

// Default: 2.5f, 7.5f, 2.0f, 16, 0
canny::canny(const string imgPath, 
    float lowThreshold, float highThreshold, 
    float gaussianKernelRadius, int gaussianKernelWidth,  
    int contrastNormalised) {
    
    this->imgPath = imgPath;

    // Get grayImg.
    rgbImg = CImg<unsigned char>(imgPath.c_str());
    if (rgbImg.spectrum() == 1) grayImg = rgbImg;
    else grayImg = rgb2gray(rgbImg);

    // Base canny parameters;
    this->lowThreshold = lowThreshold;
    this->highThreshold = highThreshold;
    this->gaussianKernelRadius = gaussianKernelRadius;
    this->gaussianKernelWidth = gaussianKernelWidth;
    this->contrastNormalised = contrastNormalised;

    this->low = (int) (lowThreshold * MAGNITUDE_SCALE + 0.5f);
    this->high = (int) ( highThreshold * MAGNITUDE_SCALE + 0.5f);
    this->width = this->grayImg.width();
    this->height = this->grayImg.height();

    // Initial buffer.
    allocatebuffers();

    // Canny processing.
    cannyProcess();
}

canny::~canny() {
    killbuffers();
}

CImg<unsigned char> canny::rgb2gray(CImg<unsigned char> &img) {
    CImg<unsigned char> rImg = img.get_channel(0)/3;
    CImg<unsigned char> gImg = img.get_channel(1)/3;
    CImg<unsigned char> bImg = img.get_channel(2)/3;
    return (rImg + gImg + bImg);
}

float canny::hypotenuse(float x, float y) {
    return (float) sqrt(x*x +y*y);
}

float canny::gaussian(float x, float sigma) {
    return (float) exp(-(x * x) / (2.0f * sigma * sigma));
}
void canny::displayRgb() {
    rgbImg.display();
}
void canny::displayGray() {
    grayImg.display();
}
void canny::displayMagnitude() {
    magImg.display();
}
void canny::displayOnePixel() {
    onePixelImg.display();
}
void canny::displayEdge() {
    edgeImg.display();
}
void canny::displayAll() {
    (rgbImg, edgeImg).display();
    // (magImg, onePixelImg, edgeImg).display();
}

void canny::resetParams(float lowThreshold, float highThreshold, 
			float gaussianKernelRadius, int gaussianKernelWidth,  
			int contrastNormalised) {
    // Reset parameters and canny process again.
    // Base canny parameters;
    this->lowThreshold = lowThreshold;
    this->highThreshold = highThreshold;
    this->gaussianKernelRadius = gaussianKernelRadius;
    this->gaussianKernelWidth = gaussianKernelWidth;
    this->contrastNormalised = contrastNormalised;

    this->low = (int) (lowThreshold * MAGNITUDE_SCALE + 0.5f);
    this->high = (int) ( highThreshold * MAGNITUDE_SCALE + 0.5f);

    cannyProcess();
}
void canny::save(string filePath, string name, string option) {
    string path = filePath+"/"+name+"-"+option+".bmp";

    if (option == "rgb") rgbImg.save(path.c_str());
    else if (option == "gray") grayImg.save(path.c_str());
    else if (option == "mag") magImg.save(path.c_str());
    else if (option == "one") onePixelImg.save(path.c_str());
    else if (option == "edge") edgeImg.save(path.c_str());
}