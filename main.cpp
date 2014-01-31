//
//  main.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/03/11.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <iostream>
#include "AppearanceEnhancement.h"
#include "ProCam.h"
#include "common.h"
#include "myOpenCV.h"

#ifdef LINUX
#define EXPERIMENT_FLAG
#endif

using namespace std;
using namespace cv;

//コールバック関数からのイベント情報を取得するための構造体
typedef struct MouseParam{
	unsigned int x;
	unsigned int y;
    Point start, end;
	int event;
	int flags;
    cv::Mat* image;
    cv::Vec3b color;
    std::vector<cv::Mat>* v_image;
} MouseParam;

//setMouseCallbackへ渡すコールバック関数。
//この関数内でマウスからのイベント情報をMouseParamへ渡しています。
void mfunc(int event, int x, int y, int flags, void  *param){
    
	MouseParam *mparam = (MouseParam*)param;
	mparam->x = x;
	mparam->y = y;
	mparam->event = event;
	mparam->flags = flags;
    
    switch (event) {
        case 1:
            mparam->start = Point(x, y);
//            cout<<"start at "<<mparam->start<<endl;
            break;
        case 4:
            mparam->end = Point(x, y);
            for (vector<Mat>::const_iterator itr = mparam->v_image->begin();
                 itr != mparam->v_image->end();
                 ++ itr) {
                Mat part = (*itr)(Range(mparam->start.y, mparam->end.y), Range(mparam->start.x, mparam->end.x));
                Scalar l_mean, l_stddev;
                meanStdDev(part, l_mean, l_stddev);
                cout<<l_mean;
            }
            cout<<endl;
            
            
            break;
        default:
            break;
    }
}

// main method
int main(int argc, const char * argv[])
{
#ifdef MAC
    Mat l_errorOfK[6];
    l_errorOfK[0] = imread("./img/estimateKF/changeKF/K/l_errorOfEstimateK000.png");
    l_errorOfK[1] = imread("./img/estimateKF/changeKF/K/l_errorOfEstimateK050.png");
    l_errorOfK[2] = imread("./img/estimateKF/changeKF/K/l_errorOfEstimateK100.png");
    l_errorOfK[3] = imread("./img/estimateKF/changeKF/K/l_errorOfEstimateK150.png");
    l_errorOfK[4] = imread("./img/estimateKF/changeKF/K/l_errorOfEstimateK200.png");
    l_errorOfK[5] = imread("./img/estimateKF/changeKF/K/l_errorOfEstimateK250.png");
    Mat l_answerK = imread("./img/estimateKF/changeKF/K/l_answerK.png");
    l_answerK.convertTo(l_answerK, CV_64FC3, 1.0 / 255.0);
    std::vector<cv::Mat> imgV;
    imgV.push_back(l_answerK);
    for (int i = 0; i < 6; ++ i) {
        l_errorOfK[i].convertTo(l_errorOfK[i], CV_64FC3, 1.0 / 255.0);
        imgV.push_back(l_errorOfK[i]);
    }
    // F
    Mat l_errorOfF[6];
    l_errorOfF[0] = imread("./img/estimateKF/changeKF/F/l_errorOfEstimateF000.png");
    l_errorOfF[1] = imread("./img/estimateKF/changeKF/F/l_errorOfEstimateF050.png");
    l_errorOfF[2] = imread("./img/estimateKF/changeKF/F/l_errorOfEstimateF100.png");
    l_errorOfF[3] = imread("./img/estimateKF/changeKF/F/l_errorOfEstimateF150.png");
    l_errorOfF[4] = imread("./img/estimateKF/changeKF/F/l_errorOfEstimateF200.png");
    l_errorOfF[5] = imread("./img/estimateKF/changeKF/F/l_errorOfEstimateF250.png");
    Mat l_answerF = imread("./img/estimateKF/changeKF/F/l_answerF.png");
    l_answerF.convertTo(l_answerF, CV_64FC3, 1.0 / 255.0);
    imgV.push_back(l_answerF);
    for (int i = 0; i < 6; ++ i) {
        l_errorOfF[i].convertTo(l_errorOfF[i], CV_64FC3, 1.0 / 255.0);
        imgV.push_back(l_errorOfF[i]);
    }
    
    MouseParam mparam;
	mparam.x = 0; mparam.y = 0; mparam.event = 0; mparam.flags = 0;
    mparam.image = &l_answerK; mparam.color = Vec3b(0,0,0);
    mparam.v_image = &imgV;
    
	//ウインドウの作成
	namedWindow( "l_answerF", CV_WINDOW_AUTOSIZE );
	//ウインドウへコールバック関数とコールバック関数からイベント情報を受け取る変数を渡す。
	setMouseCallback( "l_answerF", &mfunc, &mparam );
    imshow("l_answerF", l_answerF);
    MY_IMSHOW(l_answerK);
    MY_WAIT_KEY();
    
    // make position
    Range l_rowRanges[6*4], l_colRanges[6*4];
    const int rows = l_errorOfK[0].rows, cols = l_errorOfK[0].cols;
    const int startRows = rows * 0.143, startCols = 10;
    const int l_chartRows = rows * 0.5, l_chartCols = cols*0.75;
    const int partRows = l_chartRows / 4, partCols = l_chartCols / 6;
//    _print4(rows, cols, partRows, partCols);
    Point colsP(startCols, startCols+partCols), rowsP(startRows, startRows+partRows);
    const int marginCols = 23, marginRows=23;
    for (int y = 0; y < 4; ++ y, rowsP += Point(marginRows+partRows, marginRows+partRows)) {
        colsP = Point(startCols, startCols+partCols);
        for (int x = 0; x < 6; ++ x, colsP += Point(marginCols+partCols, marginCols+partCols)) {
            int xyPos = y*6+x;
            const Range l_rowsRange(rowsP.x, rowsP.y);
            const Range l_colsRange(colsP.x, colsP.y);
            l_rowRanges[xyPos] = l_rowsRange;
            l_colRanges[xyPos] = l_colsRange;
        }
    }
    // modify position
    l_colRanges[11] = Range(l_colRanges[11].start+5,l_colRanges[11].end+5);
    l_colRanges[12] = Range(l_colRanges[12].start-3,l_colRanges[12].end-3);
    l_colRanges[15] = Range(l_colRanges[15].start+7,l_colRanges[15].end+7);
    l_colRanges[16] = Range(l_colRanges[16].start+10,l_colRanges[16].end+10);
    l_colRanges[18] = Range(l_colRanges[18].start-5,l_colRanges[18].end-5);
    l_colRanges[17] = Range(l_colRanges[17].start+14,l_colRanges[17].end+14);
    l_rowRanges[21] = Range(l_rowRanges[21].start+4,l_rowRanges[21].end+4);
    l_colRanges[21] = Range(l_colRanges[21].start+13,l_colRanges[21].end+13);
    l_rowRanges[22] = Range(l_rowRanges[22].start+4,l_rowRanges[22].end+4);
    l_colRanges[22] = Range(l_colRanges[22].start+13,l_colRanges[22].end+13);
    l_rowRanges[23] = Range(l_rowRanges[23].start+12,l_rowRanges[23].end+12);
    l_colRanges[23] = Range(l_colRanges[23].start+24,l_colRanges[23].end+24);
    
    // get colorful image
    Mat l_part[6*4];
    for (int y = 0; y < 4; ++ y) {
        for (int x = 0; x < 6; ++ x) {
            int xyPos = y*6+x;
            
            // answer
            l_part[xyPos]=l_answerK(l_rowRanges[xyPos], l_colRanges[xyPos]);
            ostringstream oss;
            oss << "part" << xyPos << endl;
            imshow(oss.str().c_str(), l_part[xyPos]);
//            MY_WAIT_KEY();
            
            // error image
            Mat l_errorPart[6];
            for (int i = 0; i < 6; ++ i) {
                l_errorPart[i] = l_errorOfK[i](l_rowRanges[xyPos], l_colRanges[xyPos]);
            }
            
            // calc average
            Scalar l_mean, l_stddev;
            meanStdDev(l_part[xyPos], l_mean, l_stddev);
            Scalar l_errorMean[6], l_errorStddev[6];
            for (int i = 0; i < 6; ++ i) {
                meanStdDev(l_errorPart[i], l_errorMean[i], l_errorStddev[i]);
            }
//            _print3(xyPos, l_mean, l_stddev);
            _print7(l_mean, l_errorMean[0], l_errorMean[1], l_errorMean[2], l_errorMean[3], l_errorMean[4], l_errorMean[5]);
//            _print_mat_content_propaty(0.0, l_part[xyPos]);
        }
    }
    
    MY_WAIT_KEY();
    
#endif
    
#ifdef EXPERIMENT_FLAG
    // experimentation
    AppearanceEnhancement ae(PRJ_SIZE);
    ae.doAppearanceEnhancementByAmano();
#else
    // simulation
    AppearanceEnhancement ae(0.8, 0.2);
    ae.test_calcNextProjectionImageAtPixel();
#endif
    return 0;
}
