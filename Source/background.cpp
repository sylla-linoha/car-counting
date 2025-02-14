/*
* background.c
*
*  Created on: 17/10/30
*      Author: LinhVQ
*/
#include "background.h"

/*FUNCTION**********************************************************************
 *
 * Function Name : Background
 * Description   :
 *END**************************************************************************/
IplImage *Background(IplImage *frame_gray_now, IplImage *frame_bkg,
                     IplImage *frame_gray_pass, int frame_count,
                     int StopReflesh)
{
    CvMat *temp = 0; //NULL;
    temp = cvCreateMat(frame_gray_now->height, frame_gray_now->width, CV_32FC1);

    /* At the first time, using first image as the background */
    if (!frame_bkg)
    {
        frame_bkg = cvCreateImage(cvSize(frame_gray_now->width, frame_gray_now->height), IPL_DEPTH_8U, frame_gray_now->nChannels);
        frame_bkg->origin = frame_gray_now->origin;
        cvCopy(frame_gray_now, frame_bkg, 0);
    }
    /* Using difference image to refresh the background image*/
    /* If all the objects didn't moved, stop updating the background */
    else if(!StopReflesh)
    {
        IplImage *frame_mask = 0;
        frame_mask = cvCreateImage(cvSize(frame_gray_now->width, frame_gray_now->height), IPL_DEPTH_8U, 1);
        frame_mask->origin = frame_gray_now->origin;

        /* Get the difference between the frame_gray_pass and frame_gray_now */
        cvAbsDiff(frame_gray_now, frame_gray_pass, frame_mask);

        /* Make the binary image as mask, and define static background as 1*/
        cvThreshold(frame_mask, frame_mask, 0, 255, CV_THRESH_BINARY_INV);
        //cvShowImage("background", frame_mask);

        cvConvert(frame_bkg, temp);
        /* Reduce the effect of the first image, descend Weight of input image gradually */
        if(frame_count < 20)
        {
            cvRunningAvg(frame_gray_now, temp, (0.503 - (frame_count) / 110.0), frame_mask);
            if (frame_count < 10)
            {
                cvSmooth(temp, temp, CV_GAUSSIAN, 3, 0, 0);
            }
        }
        else
        {
            cvRunningAvg(frame_gray_now, temp, 0.003, frame_mask);
        }
        cvConvert(temp, frame_bkg);
        cvReleaseImage(&frame_mask);
        cvReleaseMat(&temp);
    }
    return frame_bkg;
}
