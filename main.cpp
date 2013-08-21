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
