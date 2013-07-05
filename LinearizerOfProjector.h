//
//  LinearizerOfProjector.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/07.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#ifndef __cameraBase03__LinearizerOfProjector__
#define __cameraBase03__LinearizerOfProjector__

#include <iostream>
#include "myOpenCV.h"
#include "ProCam.h"
#include <vector>

// 先攻宣言
class ProCam;

class LinearizerOfProjector{
private:
    ProCam* m_procam;
//    cv::Mat** m_aColorMixingMatrix;  // 全カメラ画素分のC=VPのV, プロジェクタからカメラへの色変換行列
    std::vector<cv::Mat> m_aColorMixingMatrix;  // 全カメラ画素分のC=VPのV, プロジェクタからカメラへの色変換行列
    
    LinearizerOfProjector(const LinearizerOfProjector& _lop);   // コピーコンストラクタ隠し（プログラムで１つしか存在しない為）
public:
    // constructor
    LinearizerOfProjector(void);
    LinearizerOfProjector(ProCam* procam);
    // destructor
    ~LinearizerOfProjector(void);
    // set method
//    bool setColorMixingMatrix(cv::Mat** colMix);
    bool setProCam(ProCam* procam);
    bool setColorMixMat(const cv::Mat& mat, const int index);
    bool setColorMixMat(const std::vector<cv::Mat>* _aMat);
    // get method
//    bool getColorMixingMatrix(cv::Mat** colMix);
    bool getProCam(ProCam* const procam);
    ProCam* getProCam(void);
    cv::Mat* getColorMixMat(const int index);
    const std::vector<cv::Mat>* getColorMixMat(void);
    // init method
//    bool initColorMixingMatrix(const cv::Size& _mixMatSize);
    bool initColorMixingMatrix(const int _mixMatLength);
    // other method
    bool linearlize(char* const responseOfProjector);
    bool calcColorMixingMatrix(void);
    bool createVMap(const cv::Mat& _normalR2BL, const cv::Mat& _normalG2BL, const cv::Mat& _normalB2BL);
    bool calcResponseFunction(char* const _responseOfProjector);
    bool getResponse(cv::Vec3b* const _response, const cv::Vec3b& _I, const cv::Vec3b& _C, const cv::Mat& _V);
    bool getResponseOfAllPixel(cv::Mat_<cv::Vec3b>* const _response, const cv::Vec3b& _I, const cv::Mat_<cv::Vec3b>& _CImage);
};

#endif /* defined(__cameraBase03__LinearizerOfProjector__) */
