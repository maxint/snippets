#include <highgui.h>
#include <cv.h>
#include <stdio.h>

#define VDO_MODE 1

static const char* WIN_NAME = "Capture Demo";

static const CvScalar GRAY = cvScalar(128,128,128);
static const CvScalar DARKGRAY = cvScalar(64,64,64);
static const CvScalar BLACK = cvScalar(0,0,0);
static const CvScalar RED = cvScalar(0,0,255);
static const CvScalar GREEN = cvScalar(0,255,0);
static const CvScalar BLUE = cvScalar(255,0,0);
static const CvScalar CYAN = cvScalar(255,255,0);
static const CvScalar YELLOW = cvScalar(0,255,255);
static const CvScalar MAGENTA = cvScalar(255, 0,255);

int main(int argc, char** argv)
{
    int width = 320, height = 240;

    // Open video
    bool quit = false;
    bool playing = true;

    IplImage *frame = NULL;
    IplImage *grey = NULL;
    IplImage *draw = NULL;

#if VDO_MODE
    const char *VDO_FNAME = "data/mark.avi";
    CvCapture* cap = cvCreateFileCapture(VDO_FNAME);
    if (!cap) {
        printf("[E] Failed to open video %s.\n", VDO_FNAME);
        return -1;
    }
#else
    CvCapture* cap = cvCreateCameraCapture(0);
    if (!cap) {
        printf("[E] Failed to open web camera.\n");
        return -1;
    }
    cvQueryFrame(cap); // query one frame to start the dshow filter for web camera
#endif

    width = (int)cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH);
    height = (int)cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT);

#if VDO_MODE
    CvFont font = cvFont(1, 1);
    char msg[256];
#endif // VDO_MODE
    cvNamedWindow(WIN_NAME);

    // create draw image buffer and grey image
    draw = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
    grey = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

#if VDO_MODE
    int frameIdx = 0;
    int framecount = (int)cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_COUNT);
#endif // VDO_MODE

    while (!quit) {

#if VDO_MODE
        if (frameIdx == framecount - 1) {
            cvSetCaptureProperty(cap, CV_CAP_PROP_POS_FRAMES, 0);
            frameIdx = 0;
        }

        if (!playing)
            cvSetCaptureProperty(cap, CV_CAP_PROP_POS_FRAMES, frameIdx);
        else
            ++frameIdx;
#endif // VDO_MODE

        frame = cvQueryFrame(cap);
        if (!frame) {
#if VDO_MODE
            printf("[E] Failed to capture frame #%d.\n", frameIdx);
#else
            printf("[E] Failed to capture frame.\n");
#endif
            break;
        }

#if VDO_MODE
        printf("== Frame #%d ==\n", frameIdx);
#else
        printf("== New frame ==\n");
#endif // VDO_MODE

        //cvSet(draw, GRAY);
        cvCopy(frame, draw);
        //cvFlip(frame, draw, 1);

        cvCvtColor(draw, grey, CV_BGR2GRAY);

        // processing
        // ...

        // drawing
        // ...


        // display frame information for video
#if VDO_MODE
        sprintf(msg, "#%d/%d", frameIdx, framecount);
        cvPutText(draw, msg, cvPoint(10,20), &font, GREEN);
#endif // VDO_MODE
        cvShowImage(WIN_NAME, draw);

        // control video by keyboard
        int key = cvWaitKey(playing ? 30 : 0);
        switch (key) {
        case 27: case 'q': quit = true; break; // Esc or 'q'
        case 'p': case ' ': playing = !playing; printf("[C] %s\n", playing ? "Play" : "Pause"); break;
#if VDO_MODE
        case '[': frameIdx = max(0, frameIdx-1); break;
        case ']': frameIdx = min(framecount-1, frameIdx+1); break;
#endif // VDO_MODE
        }
    }

    cvDestroyAllWindows();
    cvReleaseImage(&draw);
    cvReleaseImage(&grey);
    cvReleaseCapture(&cap);

    return 0;
}