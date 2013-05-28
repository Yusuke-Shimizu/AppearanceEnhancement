//
//  main.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/03/11.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <iostream>
#include "common.h"
#include "myOpenCV.h"
//#include "myOpenGL.h"
#include "GeometricCalibration.h"

using namespace std;
using namespace cv;

// main method
int main(int argc, const char * argv[])
{
    // 幾何キャリブレーションを行う
    GeometricCalibration gc = GeometricCalibration();
    gc.test_calcBitCodeNumber();
    if (!gc.doCalibration()) {
        return -1;
    }
    
    return 0;
}
