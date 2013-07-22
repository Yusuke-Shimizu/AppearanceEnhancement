//
//  main.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/03/11.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <iostream>
#include "ProCam.h"
#include "GeometricCalibration.h"
#include "myOpenCV.h"
#include "myMath.hpp"
#include "common.h"
#include "AppearanceEnhancement.h"

using namespace cv;

// main method
int main(int argc, const char * argv[])
{
    // 見えの強調クラスの設定
//    AppearanceEnhancement ae = AppearanceEnhancement();
//    ae.test_RadiometricModel();
    // プロカムの設定
    Size prjSize(PRJ_SIZE_WIDTH, PRJ_SIZE_HEIGHT);
    ProCam procam(prjSize);
//    procam.allCalibration();
//    procam.test_interpolationProjectorResponseP2I();
    Mat test(prjSize.height, prjSize.width, CV_8UC3, Scalar(0, 0, 0));
    procam.initProjectorResponseP2I(&test);
    procam.printProjectorResponseP2I(Point(100, 100), test);
    return 0;
}
