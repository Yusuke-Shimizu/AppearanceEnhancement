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

// 先攻宣言
class ProCam;

class LinearizerOfProjector{
private:
    ProCam* m_procam;
    cv::Mat m_ColorMixingMatrix;  // C=VPのV, プロジェクタからカメラへの色変換行列
public:
    // constructor
    LinearizerOfProjector(void);
    LinearizerOfProjector(ProCam* procam);
    // set method
    bool setColorMixingMatrix(const cv::Mat* const colMix);
    bool setProCam(ProCam* procam);
    // get method
    bool getColorMixingMatrix(cv::Mat* const colMix);
    bool getProCam(ProCam* const procam);
    ProCam* getProCam(void);
    // init method
    bool initColorMixingMatrix(void);
    // other method
    bool linearlize(double* const responseOfProjector);
    bool calcColorMixingMatrix(void);
};

#endif /* defined(__cameraBase03__LinearizerOfProjector__) */
