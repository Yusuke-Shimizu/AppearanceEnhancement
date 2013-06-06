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

// main method
int main(int argc, const char * argv[])
{
    // プロカムの設定
    //ProCam procam = ProCam(<#const cv::Size *const cameraSize#>, <#const cv::Size *const projectorSize#>, <#const int cameraBitDepth#>, <#const int projectorBitDepth#>);

    // 幾何キャリブレーションを行う
    GeometricCalibration gc = GeometricCalibration();
    if (!gc.doCalibration()) {
        return -1;
    }
    
    return 0;
}
