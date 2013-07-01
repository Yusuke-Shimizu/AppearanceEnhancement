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
    ProCam procam = ProCam(prjSize);
    _print_name(procam);
    procam.allCalibration();
//    procam.loadAccessMapCam2Pro();
    
    // 幾何キャリブレーションを行う
//    GeometricCalibration gc = GeometricCalibration();
//    if (!gc.doCalibration()) {
//        return -1;
//    }
    
    return 0;
}
