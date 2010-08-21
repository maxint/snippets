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
    // Ԥ���������˲��ֿ�
    void init(float s_r, float s_s, int N);

    // Ӧ��Kuwahara �˲�
    void apply(const Mat &src, Mat &dest, float alpha = 1.0, float q = 8.0);

private:
    // ����ṹ����������ÿ�����ص�ı߽緽��͸������Գ̶�
    bool calculateStuctureTenser();

    // ����һ����˹���󣬰뾶Ϊradius����׼��Ϊsigmal
    Matd createGaussianMatrix(int radius, double sigma1);

    void printMatrix(const Mat &m, const char* file);

    void normalizeMatrix(const Mat &m);

    void calMeanAndStd2(const Mat &roi);

    Mat getTransformMatrix2D( Point2f center, Point2f offset, double radian = 0, double scale1 = 1, double scale2 = 1 );

private:
    int _N;                 // �˲��˷ֿ�������ȡ4��8��16
    double _segma_r;        // ��������Ԥ�����˲��ֿ��Gaussian�˲�����
    double _segma_s;
    Mat _Msrc;              // ԭͼ��
    Mat _Mdest;             // Ŀ��ͼ��
    Mat_<Vec2d> _Mest;      // ���������˲���������

    vector<Vec3d> _meanK;   // �ֿ�ľ�ֵ
    vector<Vec3d> _segma2K; // �ֿ�ķ���

    MatVec _MKi;            // Ԥ�������˲��ֿ�
    int _Ksize;             // �˲��ֿ��С
};

#endif//_FILTER_H_
