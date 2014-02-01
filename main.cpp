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
    std::vector<cv::Mat> imgV;
    imgV.push_back(imread("./img/project/onlyK/l_K9.png"));
    imgV.push_back(imread("./img/project/onlyK/l_errorOfProjection9.png"));
    imgV.push_back(imread("./img/project/onlyK/l_C_9.png"));
    imgV.push_back(imread("./img/project/onlyK/l_desireC9.png"));
    
    imgV.push_back(imread("./img/project/both/changeK/l_K12.png"));
    imgV.push_back(imread("./img/project/both/changeK/l_errorOfProjection12.png"));
    imgV.push_back(imread("./img/project/both/changeK/l_C_12.png"));
    imgV.push_back(imread("./img/project/both/changeK/l_desireC12.png"));
    
    imgV.push_back(imread("./img/project/both/changeBoth/l_K13.png"));
    imgV.push_back(imread("./img/project/both/changeBoth/l_errorOfProjection13.png"));
    imgV.push_back(imread("./img/project/both/changeBoth/l_C_13.png"));
    imgV.push_back(imread("./img/project/both/changeBoth/l_desireC13.png"));
    imgV.push_back(imread("./img/project/both/changeBoth/l_F13.png"));
    
    for (vector<Mat>::iterator itr = imgV.begin();
         itr != imgV.end();
         ++ itr) {
        itr->convertTo(*itr, CV_64FC3, 1.0 / 255.0);
    }

    MouseParam mparam;
	mparam.x = 0; mparam.y = 0; mparam.event = 0; mparam.flags = 0;
    mparam.image = &imgV[0]; mparam.color = Vec3b(0,0,0);
    mparam.v_image = &imgV;
    
	//ウインドウの作成
	namedWindow( "l_answerK", CV_WINDOW_AUTOSIZE );
	//ウインドウへコールバック関数とコールバック関数からイベント情報を受け取る変数を渡す。
	setMouseCallback( "l_answerK", &mfunc, &mparam );
    imshow("l_answerK", imgV[0]);
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
