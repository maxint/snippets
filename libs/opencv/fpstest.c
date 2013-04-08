#include <highgui.h>
#include <stdio.h>

const char* WIN_NAME = "Camera FPS Test";
const char* WIN_NAME_EX = "Padding";
const CvScalar FD_FLAG_COLOR = {0,0,255,255};

int main( int argc, char* argv[] )
{
    IplImage *img = NULL;
	CvCapture *cap = NULL;
	LARGE_INTEGER tFreq = {0};
	LARGE_INTEGER tLast = {0};
	LONGLONG tAcc = 0;
	int accCount = 0;

	QueryPerformanceFrequency(&tFreq);
	
    cvNamedWindow(WIN_NAME, CV_WINDOW_AUTOSIZE);
    cvShowImage(WIN_NAME, img);

	cap = cvCreateCameraCapture(0);
	while (cap)
	{
		LARGE_INTEGER tCur;
		img = cvQueryFrame(cap);
		if (!img) continue;

		QueryPerformanceCounter(&tCur);
		if (tLast.QuadPart != 0) {
			tAcc += tCur.QuadPart - tLast.QuadPart;
			++accCount;
			if (tAcc > tFreq.QuadPart) {
				tAcc = tCur.QuadPart - tLast.QuadPart;
				accCount = 1;
			}
			// average is 15fps, 8~10fps in low illumination
			printf("FPS: %.3f\n", tFreq.QuadPart * accCount * 1.0f / tAcc);
		}
		tLast = tCur;
		
		cvShowImage(WIN_NAME, img);
		
		if (cvWaitKey(5) == 27)
			break;
	}
	
    cvDestroyWindow(WIN_NAME);
	cvReleaseCapture(&cap);
	
    return 0;
}
