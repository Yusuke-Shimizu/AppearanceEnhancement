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
    std::vector<Scalar> l_vMean;
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
//                cout<<l_mean;
                l_vMean.push_back(l_mean);
            }
//            cout<<endl;
            for (int c = 0; c < 3; ++ c) {
                for (vector<Scalar>::const_iterator itr = l_vMean.begin();
                     itr != l_vMean.end();
                     ++ itr) {
                    const Scalar l_meanItr(*itr);
                    cout<<l_meanItr[c]<<"\t";
                }
            }
            cout<<endl;
            
            break;
        default:
            break;
    }
}

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
// rgbで変化させる
bool divideImage3_(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const cv::Scalar& _rate){
    int rows = _src.rows, cols = _src.cols;
    double l_diff = 0;
    if (isContinuous(*_dst1, *_dst2, _src)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        Vec3d* l_pDst1 = _dst1->ptr<Vec3d>(y);
        Vec3d* l_pDst2 = _dst2->ptr<Vec3d>(y);
        const Vec3d* l_pSrc = _src.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < 3; ++ c) {
                double l_srcNum = l_pSrc[x][c];
                l_diff = (std::min(l_srcNum, 1.0 - l_srcNum)) * _rate[c];
                l_pDst1[x][c] = l_srcNum + l_diff;
                l_pDst2[x][c] = l_srcNum - l_diff;
            }
        }
    }
    return true;
}
bool divideImage3_(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const double& _rate = 1.0){
    return divideImage3_(_dst1, _dst2, _src, Scalar(_rate, _rate, _rate));
}
// main method
int main(int argc, const char * argv[])
{
    
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
