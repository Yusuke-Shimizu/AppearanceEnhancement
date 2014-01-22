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

bool divideImage_(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const cv::Scalar& _distance){
    Mat l_src, l_srcLab, l_dstLab1, l_dstLab2;
    _src.convertTo(l_src, CV_32FC3);
    cv::cvtColor(l_src, l_srcLab, CV_BGR2Lab);
    l_dstLab1 = l_srcLab.clone() + _distance;
    l_dstLab2 = l_srcLab.clone() - _distance;
    cv::cvtColor(l_dstLab1, *_dst1, CV_Lab2BGR);
    cv::cvtColor(l_dstLab2, *_dst2, CV_Lab2BGR);
    _dst1->convertTo(*_dst1, CV_64FC3);
    _dst2->convertTo(*_dst2, CV_64FC3);
    return true;
}
bool divideImage_(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const double& _distance=10){
    return divideImage_(_dst1, _dst2, _src, Scalar(0, _distance, _distance));
}

// 片方をLabのab平面を移動させ，もう一方は先ほどの移動させたものをRGBに持ってきて
// 対象に移動する
bool divideImage2_(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const cv::Scalar& _distance){
    Mat l_srcRGB, l_srcLab;
    _src.convertTo(l_srcRGB, CV_32FC3);
    cv::cvtColor(l_srcRGB, l_srcLab, CV_BGR2Lab);
    l_srcLab += _distance;
    Mat l_dstRGB1, l_dstRGB2 = l_srcRGB.clone();
    cv::cvtColor(l_srcLab, l_dstRGB1, CV_Lab2BGR);
    l_dstRGB2 += l_srcRGB - l_dstRGB1;
    
    l_dstRGB1.convertTo(*_dst1, CV_64FC3);
    l_dstRGB2.convertTo(*_dst2, CV_64FC3);
    return true;
}
bool divideImage2_(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const double& _distance=10){
//    return divideImage2_(_dst1, _dst2, _src, Scalar(0, _distance, _distance));
    return divideImage2_(_dst1, _dst2, _src, Scalar(_distance, _distance, _distance));
}

// main method
int main(int argc, const char * argv[])
{
//    VideoCapture video(0);
//    Mat frame;
//    video >> frame;
//    const Size camSize(frame.cols, frame.rows);
//    Mat divFrame1(camSize, CV_64FC3, CV_SCALAR_D_BLACK);
//    Mat divFrame2(camSize, CV_64FC3, CV_SCALAR_D_BLACK);
//    ostringstream oss;
//    while (true) {
//        video>>frame;
//        frame.convertTo(frame, CV_64FC3, 1/255.0);
//        
//        divideImage2_(&divFrame1, &divFrame2, frame);
//        MY_IMSHOW3(frame, divFrame1, divFrame2);
//    }
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
