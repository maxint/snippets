// ======================================================================================
// File         : main.cpp
// Author       : maxint <lnychina@gmail.com> 
// Last Change  : 08/21/2010 | 21:52:25 PM | Saturday,August
// Description  : CPU implementation of Anisotropic Kuwahara filtering
//                Paper Home: http://www.kyprianidis.com/pg2009.html
// ======================================================================================

#include <cv.h>
#include <highgui.h>
#include <vector>
#include <iostream>
#include "filter.h"

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
    vector<char*> varg(argv, argv+argc);

    char imgfile[256] = "images/lion.png";
    float segma_r = 3.0;
    float segma_s = 1.0;
    int N = 4;

    if (varg.size()>1)
        strcpy(imgfile, varg[1]);
    else
        cout << "Help:\n"
            << "\tfilter <image> [sections [segma_r [segma_s] ] ]\n\n";

    if (varg.size()>2) N = atoi(varg[2]);
    if (varg.size()>3) segma_r = float(atof(varg[3]));
    if (varg.size()>4) segma_s = float(atof(varg[4]));


    Mat img = imread(imgfile, CV_LOAD_IMAGE_COLOR);
    if (img.empty())
        return -1;

    //////////////////////////////////////////////////////////////////////////
    // Kuwahara滤波
    CFilter filter;
    filter.init(segma_r, segma_s, N);
    Mat res;
    filter.apply(img, res);
    imwrite("filter_result.png", res);


    //////////////////////////////////////////////////////////////////////////
    // 高斯滤波，用于对比
    Mat img_g;
    GaussianBlur(img, img_g, Size(), 2.0);


    //////////////////////////////////////////////////////////////////////////
    // 显示结果
    namedWindow("Kuwahera", CV_WINDOW_AUTOSIZE);
    imshow("Kuwahera", res);

    namedWindow("original", CV_WINDOW_AUTOSIZE);
    imshow("original", img);

    namedWindow("Gaussian Blur", CV_WINDOW_AUTOSIZE);
    imshow("Gaussian Blur", img_g);

    waitKey();

    return 0;
}
