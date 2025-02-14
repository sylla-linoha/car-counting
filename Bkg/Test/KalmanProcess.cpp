#include <opencv\cv.h>
#include <opencv2\highgui\highgui.hpp>
#include <ctype.h>
#include "struct.h"
#include <math.h>

PointSeqList KalmanProcess(KalmanPoint *Kalmanfilter, PointSeqList Points,
    IplImage *temp, IplImage *final, int *StopReflesh, int frame_count) {
    //use kalman_filter to predict the state of the centroid
    /* Su dung bo loc Kalman de du doan vi tri cua trong tam*/
    CvPoint Centroid;
    CvPoint Point_Find;
    const CvMat *Prediction, *correction;
    CvMat measurement;
    int delta = 10;

    Point_Find.x = Point_Find.y = 0;

    const CvMat *state_ptMat_pre = Kalmanfilter->Kalman->state_post;
    Prediction = cvKalmanPredict(Kalmanfilter->Kalman, 0);

    Centroid = cvPoint(cvRound(cvmGet(Prediction, 0, 0)), cvRound(cvmGet(Prediction, 1, 0)));

    // out of the screen,release the filter
    if (Centroid.x > temp->width + delta || Centroid.x < 0 - delta || Centroid.y > temp->height + delta || Centroid.y < 0 - delta)
    {
        cvReleaseKalman(&(Kalmanfilter->Kalman));
        Kalmanfilter->Kalman = NULL;
        return Points;
    }

    //*********** search the PointSeqList Directly ***************
    PointSeq *find = NULL;
    PointSeq *pt = Points;
    //int direction = 320 * 240; //ori
    int direction = 1280 * 720; //video HD
    //int direction = 640 * 480;//webcam logitech

    //search for the minimal direction Point
    while (pt) {
        int x = pt->Point.x - Centroid.x;
        int y = pt->Point.y - Centroid.y;
        int t = x * x + y * y;
        if (t < direction) {
            direction = t;

            find = pt;
            Point_Find = pt->Point;
        }
        pt = pt->next;
    }

   /* printf("direction %f\n",sqrt(direction));*/
    if (sqrt(direction) > 30) {
        Point_Find = Centroid;
        find = NULL;
        Kalmanfilter->Loss++;
    }

    // delete the Point found
    if (find) {
        if (find == Points) {
            Points = Points->next;
            delete[] find;
            find = Points;
        }
        else {
            if (find->next == NULL) {
                find = find->pre;
                delete[] find->next;
                find->next = NULL;
            }
            else {
                PointSeq *s;
                s = find->next;
                find->pre->next = find->next;
                find->next->pre = find->pre;
                delete[] find;
                find = s;
            }
        }
    }
    else {
        Point_Find = Centroid;
        Kalmanfilter->Loss++;
    }

    Kalmanfilter->Point_pre = Kalmanfilter->Point_now;
    Kalmanfilter->Point_now = Point_Find;
    Kalmanfilter->lastFrame = frame_count;//tambahan 25 januari 2016 speed measurement

    int delta_x = abs(Kalmanfilter->Point_now.x - Kalmanfilter->Point_pre.x);
    int delta_y = abs(Kalmanfilter->Point_now.y - Kalmanfilter->Point_now.y);
    //errrorrrr stop reflesh
    if (delta_x <= 2 && delta_y <= 2)
    {
        *StopReflesh = 1;
    }
    else
    {
        *StopReflesh = 0;
    }

    float measure[2] = { (float)Point_Find.x, (float)Point_Find.y };
    measurement = cvMat(2, 1, CV_32FC1, measure);
    correction = cvKalmanCorrect(Kalmanfilter->Kalman, &measurement);

    //************************************************************

    //Ve quy dao di cua xe
    const CvMat *state_ptMat_now = Kalmanfilter->Kalman->state_post;
    const CvPoint state_pt_now = cvPoint(cvRound(cvmGet(state_ptMat_now, 0, 0)), cvRound(cvmGet(state_ptMat_now, 1, 0)));
    cvLine(final, Kalmanfilter->firstPoint, state_pt_now, CV_RGB(255, 255, 0), 1, CV_AA, 0);

    // Print the ID of the object on the screen
    char text[10];
    //sprintf(text, "ID:%d", Kalmanfilter->ID);
    //sprintf(text, "x:%d", state_pt_now.x);
    /* Print countour */
    sprintf(text, "%d", Kalmanfilter->contourArea);//test 26 januari 2016
    sprintf(text, "%d", Kalmanfilter->jenis); //test 26 januari 2016
    CvFont font;
    cvInitFont(&font, CV_FONT_VECTOR0, 0.5f, 0.5f, 0, 2);
    cvPutText(final, text, state_pt_now, &font, CV_RGB(255, 0, 0));

    //Them thu tran bo nho

    //char jns[10];
    /*
    if (Kalmanfilter->jenis == 1) {
    sprintf(jns, "mobil");
    } else if (Kalmanfilter->jenis == 2) {
    sprintf(jns, "truk");
    } else {
    sprintf(jns, "undefined");
    }*/

    ////nilai 1 = motor;nilai 2 = mobil;nilai 3 = truk sedang;nilai 4 = truk besar;nilai 0 = undefined;
    //if (Kalmanfilter->jenis == 2) {
    //   // sprintf(jns, "mobil");  // Oto con
    //}
    //else if (Kalmanfilter->jenis == 3) { // Xe tai hang trung
    //    //sprintf(jns, "truk sedang");
    //}
    //else if (Kalmanfilter->jenis == 4) {  // Xe tai hang nang
    //  //  sprintf(jns, "truk besar");
    //}
    //else {
    //  //  sprintf(jns, "undefined");  // Undefined
    //}

    //cvPutText(final, jns, cvPoint(state_pt_now.x, state_pt_now.y + 15), &font,
       // CV_RGB(255, 0, 0));
    return Points;
}
