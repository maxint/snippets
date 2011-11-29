
int selectedRange = 0;
Range rgSelected[6] = 
{
	Range(40,164),Range(56,149),
	Range(192,241),Range(32,185),
	Range(16,169),Range(163,206)
};

void on_mouse( int event, int x, int y, int flags, void* param )
{
	static bool draw = false;
	Mat* pm = (Mat*)param;
	switch( event )
	{
	case CV_EVENT_LBUTTONDOWN: 
		{
			rgSelected[selectedRange*2].start = min(max(0,y),pm->rows-1);
			rgSelected[selectedRange*2+1].start = min(max(0,x),pm->cols-1);
			cout << "Begin " << x << "," << y << endl;
			draw = true;
		}
		break;
	case CV_EVENT_RBUTTONDOWN: 

		break;
	case CV_EVENT_LBUTTONUP:
		{
			rgSelected[selectedRange*2].end = min(max(0,y),pm->rows-1);
			rgSelected[selectedRange*2+1].end = min(max(0,x),pm->cols-1);
			cout << "End " << x << "," << y << endl;
			draw = false;
		}
		break;
	case CV_EVENT_RBUTTONUP:

		break;
	case CV_EVENT_MOUSEMOVE:
		if(draw) {
			Mat _tmp; (*((Mat*)param)).copyTo(_tmp);
			rgSelected[selectedRange*2].end = min(max(0,y),pm->rows-1);
			rgSelected[selectedRange*2+1].end = min(max(0,x),pm->cols-1);
			static const Scalar kColors[] = {Scalar(255,0,0), Scalar(0,255,0), Scalar(0,0,255)};
			for (int k=0; k<3; ++k)
				rectangle(_tmp,Point(rgSelected[k*2+1].start,rgSelected[k*2].start),Point(rgSelected[k*2+1].end,rgSelected[k*2].end),kColors[k],2);
			
			imshow("tmp1",_tmp);
		}
		break;
	}
}

int main(int argc, char** argv) {
	Mat origImg = imread("child.jpg");
	Mat img;
	resize(origImg,img,Size(origImg.cols/2,origImg.rows/2));

	rgSelected[4] = Range(0,img.rows-1);
	rgSelected[5] = Range(0,img.cols-1);

	namedWindow("tmp");
	namedWindow("tmp1");
	cvSetMouseCallback( "tmp1", on_mouse, &img );
	namedWindow("tmp2");

	return 0;
}

