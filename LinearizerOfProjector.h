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
    cv::Mat** m_aColorMixingMatrix;  // 全カメラ画素分のC=VPのV, プロジェクタからカメラへの色変換行列
//    std::vector<cv::Mat> m_aColorMixingMatrix;  // 全カメラ画素分のC=VPのV, プロジェクタからカメラへの色変換行列
public:
    // constructor
    LinearizerOfProjector(void);
    LinearizerOfProjector(ProCam* procam);
    // destructor
    ~LinearizerOfProjector(void);
    // set method
    bool setColorMixingMatrix(cv::Mat** colMix);
    bool setProCam(ProCam* procam);
    // get method
    bool getColorMixingMatrix(cv::Mat** colMix);
    bool getProCam(ProCam* const procam);
    ProCam* getProCam(void);
    // init method
    bool initColorMixingMatrix(const cv::Size& _mixMatSize);
    // other method
    bool linearlize(double* const responseOfProjector);
    bool calcColorMixingMatrix(void);
};

#endif /* defined(__cameraBase03__LinearizerOfProjector__) */
