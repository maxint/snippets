// ======================================================================================
// File         : filter.h
// Author       : maxint <lnychina@gmail.com> 
// Last Change  : 08/21/2010 | 21:52:42 PM | Saturday,August
// Description  : CPU implementation of Anisotropic Kuwahara filtering
//                Paper Home: http://www.kyprianidis.com/pg2009.html
// ======================================================================================

#ifndef _FILTER_H_
#define _FILTER_H_

#include <cv.h>
using namespace cv;

class CFilter
{
private:
    typedef Mat_<double> Matd;
    typedef Mat_<float> Matf;
    typedef vector<Matf> MatVec;

public:
    //CFilter(const Mat &src, Mat &dest, const Mat &est, double s_r, double s_s, int N);
    ~CFilter(void);

public:
    // 预处理，生成滤波分块
    void init(float s_r, float s_s, int N);

    // 应用Kuwahara 滤波
    void apply(const Mat &src, Mat &dest, float alpha = 1.0, float q = 8.0);

private:
    // 计算结构张量，估量每个像素点的边界方向和各向异性程度
    bool calculateStuctureTenser();

    // 生成一个高斯矩阵，半径为radius，标准差为sigmal
    Matd createGaussianMatrix(int radius, double sigma1);

    void printMatrix(const Mat &m, const char* file);

    void normalizeMatrix(const Mat &m);

    void calMeanAndStd2(const Mat &roi);

    Mat getTransformMatrix2D( Point2f center, Point2f offset, double radian = 0, double scale1 = 1, double scale2 = 1 );

private:
    int _N;                 // 滤波核分块数，可取4、8、16
    double _segma_r;        // 两个用来预处理滤波分块的Gaussian滤波参数
    double _segma_s;
    Mat _Msrc;              // 原图像
    Mat _Mdest;             // 目标图像
    Mat_<Vec2d> _Mest;      // 各项异性滤波参数矩阵

    vector<Vec3d> _meanK;   // 分块的均值
    vector<Vec3d> _segma2K; // 分块的方差

    MatVec _MKi;            // 预处理后的滤波分块
    int _Ksize;             // 滤波分块大小
};

#endif//_FILTER_H_
