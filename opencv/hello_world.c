#include <cv.h>
#include <highgui.h>
#include <stdio.h>

#ifndef _DEBUG
#pragma comment(lib, "cxcore210.lib")
#pragma comment(lib, "cv210.lib")
#pragma comment(lib, "highgui210.lib")
#else
#pragma comment(lib, "cxcore210.lib")
#pragma comment(lib, "cv210.lib")
#pragma comment(lib, "highgui210.lib")
#endif

const char WIN_NAME[] = "Lena";
const char WIN_NAME_EX[] = "Padding";
const CvScalar FD_FLAG_COLOR = {0,0,255,255};

int main( int argc, char* argv[] )
{
	IplImage		*img = NULL;

	img = cvLoadImage("lena.jpg", CV_LOAD_IMAGE_COLOR);

	cvNamedWindow(WIN_NAME, CV_WINDOW_AUTOSIZE);
	cvShowImage(WIN_NAME, img);

	cvWaitKey(0);

	cvDestroyWindow(WIN_NAME);
	cvReleaseImage(&img);

	return 0;
}
