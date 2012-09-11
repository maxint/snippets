#include <cv.h>
#include <highgui.h>
#include <stdio.h>

#pragma comment(lib, "cxcore.lib")
#pragma comment(lib, "cv.lib")
#pragma comment(lib, "highgui.lib")

#define SL_EPS 0.0001f

const char WIN_NAME[] = "Lena";
const char WIN_NAME_WARPED[] = "Wrapped";
const CvScalar FD_FLAG_COLOR = {255,0,0,255};

const int TRI_NUM = 4;
const CvPoint2D64d TRI_ARR1[] = {
	{0,0}, {0.5,0.5}, {1,0},
	{1,0}, {0.5,0.5}, {1,1},
	{1,1}, {0.5,0.5}, {0,1},
	{0,1}, {0.5,0.5}, {0,0},
}; // v1, v2, v3
const CvPoint2D64d TRI_ARR2[] = {
	{0,0}, {0.5,0.8}, {1,0},
	{1,0}, {0.5,0.8}, {1,1},
	{1,1}, {0.5,0.8}, {0,1},
	{0,1}, {0.5,0.8}, {0,0},
}; // v1', v2', v3'

void scanline_fill(int y, int l, int r, const double *aff_mat, const CvPoint2D64d tri1[],
                   const IplImage *imgOrig, IplImage *imgDst);


void triangle_warping(const CvPoint2D64d tri1[],
                      const CvPoint2D64d tri2[],
                      const IplImage *imgOrig,
                      IplImage *imgDst);

int main( int argc, char* argv[] )
{
    IplImage    *img = NULL;
    IplImage    *imgWrapped = NULL;
    CvPoint2D64d tri1[3], tri2[3];
    int i,k;
    CvPoint pts[3];
    CvPoint *ptsa[] = {pts};
    int npts[] = {3};

    img = cvLoadImage("lena.jpg", CV_LOAD_IMAGE_COLOR);
    imgWrapped = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);

    for (k=0; k<TRI_NUM; ++k)
    {
        for (i=0; i<3; ++i)
        {
			tri1[i].x = TRI_ARR1[k*3+i].x*(img->width-1);
			tri1[i].y = TRI_ARR1[k*3+i].y*(img->height-1);

			//pts[i].x = (int)(tri1[i].x+0.5f);
			//pts[i].y = (int)(tri1[i].y+0.5f);

            tri2[i].x = TRI_ARR2[k*3+i].x*(img->width-1);
            tri2[i].y = TRI_ARR2[k*3+i].y*(img->height-1);
        }
        triangle_warping(tri1, tri2, img, imgWrapped);
		//cvPolyLine(imgWrapped, ptsa, npts, 1, 1, FD_FLAG_COLOR, 2, 8, 0);
    }

    for (k=0; k<TRI_NUM; ++k)
    {
        for (i=0; i<3; ++i)
        {			
			pts[i].x = (int)(TRI_ARR1[k*3+i].x*(img->width-1)+0.5f);
			pts[i].y = (int)(TRI_ARR1[k*3+i].y*(img->height-1)+0.5f);
        }
		cvPolyLine(img, ptsa, npts, 1, 1, FD_FLAG_COLOR, 2, 8, 0);
        for (i=0; i<3; ++i)
        {			
			pts[i].x = (int)(TRI_ARR2[k*3+i].x*(img->width-1)+0.5f);
			pts[i].y = (int)(TRI_ARR2[k*3+i].y*(img->height-1)+0.5f);
        }
		cvPolyLine(imgWrapped, ptsa, npts, 1, 1, FD_FLAG_COLOR, 2, 8, 0);
    }

    cvNamedWindow(WIN_NAME, CV_WINDOW_AUTOSIZE);
    cvNamedWindow(WIN_NAME_WARPED, CV_WINDOW_AUTOSIZE);
    cvShowImage(WIN_NAME, img);
    cvShowImage(WIN_NAME_WARPED, imgWrapped);

    cvWaitKey(0);

    cvSaveImage("tmp.png", imgWrapped);

    cvDestroyWindow(WIN_NAME);
    cvDestroyWindow(WIN_NAME_WARPED);
    cvReleaseImage(&img);
    cvReleaseImage(&imgWrapped);

    return 0;
}

void scanline_fill(int y, int l, int r, const double *aff_mat, const CvPoint2D64d tri1[],
                   const IplImage *imgOrig, IplImage *imgDst)
{
    double newX, newY;
    double a[3]; // centroid coordinates
    int x0, x1, y0, y1;
    int x,i;
    double wx, wy;
    uchar *pSrc, *pDst;
    int channels = imgOrig->nChannels;
    int step = imgOrig->widthStep;

    for (x=l; x<=r; ++x)
    {
        a[1] = aff_mat[0]*(x+0.5f)+aff_mat[1]*(y+0.5f)+aff_mat[2];
        a[2] = aff_mat[3]*(x+0.5f)+aff_mat[4]*(y+0.5f)+aff_mat[5];
        if ((x==l || x==r) &&
            (a[1]<-SL_EPS || a[1]>1+SL_EPS || a[2]<-SL_EPS || a[2]>1+SL_EPS)
            ) continue;

        a[0] = 1.0f-a[1]-a[2];
        newX = a[0]*tri1[0].x+a[1]*tri1[1].x+a[2]*tri1[2].x;
        newY = a[0]*tri1[0].y+a[1]*tri1[1].y+a[2]*tri1[2].y;
        x0 = (int)(newX);
        y0 = (int)(newY);
        x1 = x0+1;
        y1 = y0+1;
        wx = newX-x0;
        wy = newY-y0;
        pSrc = (uchar*)(imgOrig->imageData+imgOrig->widthStep*y0+channels*x0);
        pDst = (uchar*)(imgDst->imageData+imgDst->widthStep*y+channels*x);
        for (i=0; i<channels; ++i)
        {
            pDst[i] = (uchar)((pSrc[i]*(1-wx)+pSrc[i+channels]*wx)*(1-wy)+(pSrc[i+step]*(1-wx)+pSrc[i+channels+step]*wx)*wy);
        }
    }
}

void triangle_warping(const CvPoint2D64d tri1[],
                      const CvPoint2D64d tri2[],
                      const IplImage *imgOrig,
                      IplImage *imgDst)
{
    int i, j;
    int sortedTri2Idx[3];
    CvPoint tri2Int[3];
    double aff_mat[6]; // affine matrix, row first
    double denominator;
    int yCur;
    int xl,xr;
    double dxl, dxr;
    double xld, xrd;

    // if any point is out of region, return
    for (i=0; i<3; ++i)
    {
        if (tri2[i].x<0 || tri2[i].x>=imgDst->width) return;
        if (tri2[i].y<0 || tri2[i].y>=imgDst->height) return;
    }

    //** Step1: calculate affine matrix
    denominator = -tri2[1].x*tri2[2].y+tri2[1].x*tri2[0].y+tri2[0].x*tri2[2].y-tri2[0].x*tri2[1].y+tri2[2].x*tri2[1].y-tri2[2].x*tri2[0].y;
    aff_mat[0] = (-tri2[2].y+tri2[0].y) / denominator;
    aff_mat[1] = (tri2[2].x-tri2[0].x) / denominator;
    aff_mat[2] = (-tri2[2].x*tri2[0].y+tri2[2].y*tri2[0].x) / denominator;
    aff_mat[3] = (-tri2[0].y+tri2[1].y) / denominator;
    aff_mat[4] = (tri2[0].x-tri2[1].x) / denominator;
    aff_mat[5] = (-tri2[0].x*tri2[1].y+tri2[0].y*tri2[1].x) / denominator;

    for (i=0; i<3; ++i)
    {
        tri2Int[i].x = (int)(tri2[i].x+0.5f);
        tri2Int[i].y = (int)(tri2[i].y+0.5f);
        sortedTri2Idx[i] = i;
    }

    //** Step2: sort the points of a triangle, from top(small) to bottom(large), for left to right
    for (i=0; i<3; ++i)
    {
        int p = i;
        for (j=i+1; j<3; ++j)
        {
            if (tri2Int[sortedTri2Idx[j]].y<tri2Int[sortedTri2Idx[p]].y ||
                tri2Int[sortedTri2Idx[j]].y==tri2Int[sortedTri2Idx[p]].y &&
                tri2Int[sortedTri2Idx[j]].x<tri2Int[sortedTri2Idx[p]].x)
                p = j;
        }
        if (p!=i)
        {
            int tmp = sortedTri2Idx[i];
            sortedTri2Idx[i] = sortedTri2Idx[p];
            sortedTri2Idx[p] = tmp;
        }
    }

    //** Step3: scan-line algorithm

    /*
     *  first pair, the top triangle
     */

    yCur = tri2Int[sortedTri2Idx[0]].y;
    if (tri2Int[sortedTri2Idx[1]].y==tri2Int[sortedTri2Idx[0]].y) // horizontal line
    {
        scanline_fill(yCur,
            tri2Int[sortedTri2Idx[0]].x - ((tri2[sortedTri2Idx[0]].x>tri2[sortedTri2Idx[2]].x) ? 1 : 0),
            tri2Int[sortedTri2Idx[1]].x + ((tri2[sortedTri2Idx[1]].x<tri2[sortedTri2Idx[2]].x) ? 1 : 0),
            aff_mat, tri1, imgOrig, imgDst);
    }
    else if (tri2Int[sortedTri2Idx[2]].y!=tri2Int[sortedTri2Idx[0]].y)
    {
        dxl = (tri2[sortedTri2Idx[1]].x-tri2[sortedTri2Idx[0]].x) / (tri2[sortedTri2Idx[1]].y-tri2[sortedTri2Idx[0]].y);
        dxr = (tri2[sortedTri2Idx[2]].x-tri2[sortedTri2Idx[0]].x) / (tri2[sortedTri2Idx[2]].y-tri2[sortedTri2Idx[0]].y);
        xld = xrd = tri2[sortedTri2Idx[0]].x;
        if (dxl>dxr)
        {
            double tmp=dxl;
            dxl = dxr;
            dxr = tmp;
        }
        xld += (yCur+0.5f-tri2[sortedTri2Idx[0]].y)*dxl;
        xrd += (yCur+0.5f-tri2[sortedTri2Idx[0]].y)*dxr;
        for (; yCur<tri2Int[sortedTri2Idx[1]].y && yCur<imgDst->height; ++yCur)
        {
            xl = (int)(xld+0.5f);
            xr = (int)(xrd+0.5f);
            scanline_fill(yCur, xl, xr, aff_mat, tri1, imgOrig, imgDst);
            xld += dxl;
            xrd += dxr;
        }
    }

    /*
     *  second pair, the bottom triangle
     */

    yCur = tri2Int[sortedTri2Idx[2]].y;
    if (tri2Int[sortedTri2Idx[1]].y==tri2Int[sortedTri2Idx[2]].y &&
        tri2Int[sortedTri2Idx[1]].y==tri2Int[sortedTri2Idx[0]].y)  // three vertices are collinear
    {

        scanline_fill(tri2Int[sortedTri2Idx[2]].y,
            tri2Int[sortedTri2Idx[1]].x, tri2Int[sortedTri2Idx[2]].x,
            aff_mat, tri1, imgOrig, imgDst);
    }
    else if (tri2Int[sortedTri2Idx[1]].y!=tri2Int[sortedTri2Idx[2]].y)
    {
        yCur = tri2Int[sortedTri2Idx[2]].y-1;
        dxl = (tri2[sortedTri2Idx[2]].x-tri2[sortedTri2Idx[1]].x) / (tri2[sortedTri2Idx[2]].y-tri2[sortedTri2Idx[1]].y);
        dxr = (tri2[sortedTri2Idx[2]].x-tri2[sortedTri2Idx[0]].x) / (tri2[sortedTri2Idx[2]].y-tri2[sortedTri2Idx[0]].y);
        xld = xrd = tri2[sortedTri2Idx[2]].x;
        if (dxl<dxr)
        {
            double tmp=dxl;
            dxl = dxr;
            dxr = tmp;
        }
        xld += (yCur+0.5f-tri2[sortedTri2Idx[2]].y)*dxl;
        xrd += (yCur+0.5f-tri2[sortedTri2Idx[2]].y)*dxr;
        for (; yCur>=tri2Int[sortedTri2Idx[1]].y && yCur>=0; --yCur)
        {
            xl = (int)(xld+0.5f);
            xr = (int)(xrd+0.5f);
            scanline_fill(yCur, xl, xr, aff_mat, tri1, imgOrig, imgDst);
            xld -= dxl;
            xrd -= dxr;
        }
    }
}
