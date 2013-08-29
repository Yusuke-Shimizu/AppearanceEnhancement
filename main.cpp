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

using namespace std;
using namespace cv;

// main method
int main(int argc, const char * argv[])
{
//    for (int i = 0; i < 400; ++ i) {
//        double d = 0.01 * i - 0.1;
//        int c1 = (char)(d * 255.0);
//        int c2 = (uchar)(d * 255.0);
//        int c3 = (int)(d * 255.0);
//        double d4 = d * 255;
//        _print6(i, d, c1, c2, c3, d4);
//    }
//    checkButton();
    //
    AppearanceEnhancement ae(PRJ_SIZE);
    ae.doAppearanceEnhancementByAmano();
    
    // プロカムの設定
//    Size prjSize(PRJ_SIZE);
//    ProCam procam(prjSize);
//    procam.allCalibration();
//    
//    procam.doRadiometricCompensation(100);
    return 0;
}
