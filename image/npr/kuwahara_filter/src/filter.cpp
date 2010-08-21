// ======================================================================================
// File         : filter.cpp
// Author       : maxint <lnychina@gmail.com> 
// Last Change  : 08/21/2010 | 21:52:45 PM | Saturday,August
// Description  : CPU implementation of Anisotropic Kuwahara filtering
//                Paper Home: http://www.kyprianidis.com/pg2009.html
// ======================================================================================

#include "filter.h"
#include <highgui.h>
#include <cassert>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

using namespace cv;
using namespace std;

static clock_t t_cal_mean_and_std2 = 0;

//CFilter::CFilter(void)
//{
//}

CFilter::~CFilter(void)
{
    _Mdest.release();
    _Msrc.release();
    _Mest.release();
}

void CFilter::init(float s_r, float s_s, int N)
{
    clock_t tt = clock();

    _segma_r = s_r;
    _segma_s = s_s;
    _N = N;
    int h = int(_segma_r * 2);
    _Ksize = h;
    //_Ksize = 2*h+1;

    Mat KK(2*_Ksize, 2*_Ksize, CV_64FC1, 0.0);
    _meanK.resize(N);
    _segma2K.resize(N);

    vector<Point> pts;
    pts.push_back(Point2d( _Ksize, _Ksize-1));
    pts.push_back(Point2d(2*_Ksize-1, _Ksize-1-int(tan(CV_PI/N)*_Ksize) ) );
    pts.push_back(Point2d(2*_Ksize-1, _Ksize+int(tan(CV_PI/N)*_Ksize) ) );
    pts.push_back(Point2d(_Ksize, _Ksize));
    fillConvexPoly(KK, &pts[0], 4, Scalar(1.0f));

    Matd Mg = createGaussianMatrix(_Ksize, _segma_r);
    GaussianBlur(KK, KK, Size(), _segma_s);
    multiply(KK, Mg, KK);
    normalizeMatrix(KK);
    Mat K;
    KK.convertTo(K, CV_32F);   // 因为warpAffine只支持float

    _MKi.resize(_N);
    int i;
    for (i=0; i<_N; ++i)
    {
        Mat rotateM = getRotationMatrix2D(Point2f(_Ksize-0.5f, _Ksize-0.5f), i * 360/_N, 1);
        Mat dest;
        warpAffine(K, dest, rotateM, K.size());
        _MKi[i] = dest;
    }

    cout << "CFilter::init()\n\tFinished in " << clock()-tt << " ms.\n";
}

bool CFilter::calculateStuctureTenser()
{
    clock_t tt = clock();
    int x, y;

    //////////////////////////////////////////////////////////////////////////
    // Step 1: 计算差分
    // x, y derivative
    Mat_<Vec3d> img_dx, img_dy;
    Sobel(_Msrc, img_dx, CV_64F, 1, 0, CV_SCHARR, 1./16, 0, BORDER_REFLECT);
    Sobel(_Msrc, img_dy, CV_64F, 0, 1, CV_SCHARR, 1./16, 0, BORDER_REFLECT);


    //////////////////////////////////////////////////////////////////////////
    // Step 2: 计算结构张量
    // 1 -> dxx, 2 -> dxy, 3 -> dyy
    Mat_<Vec3d> tensor(_Msrc.size());
    for (y=0; y<_Msrc.rows; ++y)
    {
        for (x=0; x<_Msrc.cols; ++x)
        {
            tensor(y, x)[0] = img_dx(y, x).dot(img_dx(y, x));
            tensor(y, x)[1] = img_dx(y, x).dot(img_dy(y, x));
            tensor(y, x)[2] = img_dy(y, x).dot(img_dy(y, x));
        }
    }
    // 线性结构张量
    Mat_<Vec3d> tensorBlur;
    GaussianBlur(tensor, tensorBlur, Size(), 2.0, 0, BORDER_REFLECT);


    //////////////////////////////////////////////////////////////////////////
    // Step 3:　通过数据张量估计像素方向和各项异性程度
    // 1 -> local orientation, 2 -> anisotropy
    _Mest.create(tensor.size());
    Mat M(2, 2, CV_64FC1);
    Mat_<double> D, V;
    for (y=0; y<_Msrc.rows; ++y)
    {
        for (x=0; x<_Msrc.cols; ++x)
        {
            M = (Mat_<double>(2, 2) << tensorBlur(y,x)[0], tensorBlur(y,x)[1], tensorBlur(y,x)[1], tensorBlur(y,x)[2]);
            if (!eigen(M, D, V)) return false;
            _Mest(y, x)[0] = std::atan2(V(1,1), V(1,0));

            if ( (_Mest(y, x)[1] = (D(0,0)-D(1,0)) / (std::abs(D(0,0)+D(1,0))+0.001)) >1 ) return false;
            //double tmp4 = est(y, x)[1];
        }
    }

    cout << "CFilter::calculateStuctureTenser()\n\tFinished in " << clock()-tt << " ms.\n";

    return true;
}

template<typename T1, int n> static inline
Vec<T1, n> mul(const Vec<T1, n>& a, const Vec<T1, n>& b)
{
    Vec<T1, n> c;
    for (int i=0; i<n; ++i)
    {
        c.val[i] = saturate_cast<T1>(a.val[i]*b.val[i]);
    }
    return c;
}


void CFilter::apply(const Mat &src, Mat &dest, float alpha/* = 1.0*/, float q /*= 8.0*/ )
{
    clock_t tt = clock();

    _Msrc = src;

    if (!calculateStuctureTenser())
        return;

    dest.create(src.size(), src.type());
    _Mdest = dest;
    int i, j;
    Mat transM(2, 3, CV_64FC1);
    Mat roi(2*_Ksize, 2*_Ksize, src.type());        // 存储从原图像变换到标准空间上的图像块
    double alphaX, radian;
    Vec2d estX;
    for (i=0; i<_Msrc.rows; ++i)
    {
        for (j=0; j<_Msrc.cols; ++j)
        {
            estX = _Mest.at<Vec2d>(i, j);
            alphaX = alpha / (alpha+estX[1]);    // anisotropic
            radian = estX[0];     // orientation
            transM = getTransformMatrix2D( Point2f(float(j)+0.5f, float(i)+0.5f),
                Point2f(float(_Ksize), float(_Ksize)), radian, alphaX, 1./alphaX);

            // 计算标准空间上的图像块
            warpAffine(src, roi, transM, roi.size(), INTER_LINEAR, BORDER_REFLECT);

            // 计算平均值和方差
            calMeanAndStd2(roi);

            double sum_a_m, sum_a;
            Vec3b pi;
            for (int ii=0; ii<3; ++ii)
            {
                sum_a_m = sum_a = 0;
                for (int n=0; n<_N; ++n)
                {
                    _segma2K[n][ii] = 1/(1+pow(_segma2K[n][ii], (double)q/2.0f));
                    sum_a += _segma2K[n][ii];
                    sum_a_m += _segma2K[n][ii] * _meanK[n][ii];
                }
                pi[ii] = saturate_cast<uchar>(sum_a_m / sum_a);
                //cout << i*_Msrc.cols+j << ":" << (int)pi[0] << "," << (int)pi[1] << "," << (int)pi[2] << endl;
            }
            dest.at<Vec3b>(i, j) = pi;
        }//forj
    }//fori

    cout << "CFilter::apply()\n\tFinished in " << clock()-tt << " ms.\n";
    cout << "CFilter::calMeanAndStd2()\n\tFinished accumulatively in " << t_cal_mean_and_std2 << " ms.\n";
    cout << "\n== Anisotropic Kuwahara Filtering parameters:\n"
        << "   Image Size:\t" << _Msrc.rows << "(rows) X " << _Msrc.cols << "(cols)\n"
        << "   Structure Tenser segma:\t0.2 (can't custom now)\n"
        << "   Fliter segma_r:\t" << _segma_r << "\n"
        << "   Filter segma_s:\t" << _segma_s << "\n"
        << "   Number of sections N:\t" << _N << "\n";

}

void CFilter::calMeanAndStd2(const Mat &roi)
{
    clock_t tt = clock();

    _meanK.assign(_N, Vec3d(0,0,0));
    _segma2K.assign(_N, Vec3d(0,0,0));
    int sqrf;
    byte bf;
    for (int y=0; y<2*_Ksize; ++y)
    {
        for (int x=0; x<2*_Ksize; ++x)
        {
            for (int i=0; i<3; ++i)
            {
                bf = roi.at<Vec3b>(y, x)[i];
                sqrf = bf * bf;
                for (int k=0; k<_N; ++k)
                {
                    _meanK[k][i] += bf * _MKi[k](y, x);
                    _segma2K[k][i] += sqrf * _MKi[k](y, x);
                }
            }
        }
    }
    for (int i=0; i<3; ++i)
    {
        for (int k=0; k<_N; ++k)
        {
            _segma2K[k][i] -= _meanK[k][i] * _meanK[k][i];
        }
    }

    //s2 -= mul(m, m);
    t_cal_mean_and_std2 += clock()-tt;
}

Mat_<double> CFilter::createGaussianMatrix(int radius, double sigma1)
{
    Matd kernel(2*radius, 2*radius);
    Matd cd(radius, 1);
    double radiusX = cvRound(sigma1*3);
    double scale2X = -0.5/(sigma1*sigma1);
    double sum = 0;

    int i, j;
    for (i=0; i<radius; ++i)
    {
        double x = (i+1) * radiusX / radius;
        double t = std::exp(scale2X*x*x);
        cd(i, 0) = t;
        sum += t;
    }
    sum = 1./sum;
    for (i=0; i<radius; ++i)
    {
        cd(i, 0) *= sum;
    }
    // copy to kernel
    for (i=0; i<radius; ++i)
    {
        for (j=0; j<radius; ++j)
        {
            kernel(i, radius+j) = cd(radius-1-i, 0) * cd(j, 0);
            // copy to other 3 points
            kernel(i, radius-1-j) = kernel(i, radius+j);
            kernel(2*radius-1-i, radius-1-j) = kernel(i, radius+j);
            kernel(2*radius-1-i, radius+j) = kernel(i, radius+j);
        }
    }
    return kernel;
}

void CFilter::printMatrix(const Mat &m, const char* file)
{
    std::ofstream of;
    of.open(file);
    of.precision(6);
    of.width(3);
    for (int i=0; i<m.rows; ++i)
    {
        for (int j=0; j<m.cols; ++j)
        {
            of << m.at<float>(i, j) << "\t";
        }
        of << std::endl;
    }
    of.close();
}

void CFilter::normalizeMatrix(const Mat &m)
{
    double sum = 0;
    MatConstIterator_<double> it = m.begin<double>(), it_end = m.end<double>();
    for (; it!=it_end; ++it)
        sum += *it;
    m /= sum;
}

//  [12/2/2009 maxint]
// offsetTranMatrix * scaleMatrix * rotateMatrix * transformMatrix * X = X'
// X = (x, y, 1)'
Mat CFilter::getTransformMatrix2D( Point2f center, Point2f offset, double radian, double scale1, double scale2 )
{
    double alpha = cos(radian);
    double beta = sin(radian);

    Mat M(2, 3, CV_64F);
    double* m = (double*)M.data;

    m[0] = scale1*alpha;
    m[1] = scale1*beta;
    m[2] = (-alpha*center.x - beta*center.y)*scale1 + offset.x;
    m[3] = -beta*scale2;
    m[4] = alpha*scale2;
    m[5] = (beta*center.x - alpha*center.y)*scale2 + offset.y;

    return M;
}
