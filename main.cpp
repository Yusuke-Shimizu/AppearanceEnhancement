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
    _print_name(procam);
    procam.allCalibration();
//    procam.loadAccessMapCam2Pro();
    
    // 幾何キャリブレーションを行う
//    GeometricCalibration gc = GeometricCalibration();
//    if (!gc.doCalibration()) {
//        return -1;
//    }
//    double *dtest = new double();
//    *dtest = 3.01;
//    _print(*dtest);
//    delete dtest;
//    
//    cv::Mat* m = new Mat();
//    *m = Mat::eye(3, 3, CV_64FC1);
//    _print(*m);
//    delete m;
//    
//    Mat* m2 = new Mat[3];
//    for (int i = 0; i < 3; ++ i) {
//        m2[i] = Mat::ones(3, 3, CV_64FC1) * i;
//    }
//    for (int i = 0; i < 3; ++ i) {
//        _print2(i, m2[i]);
//    }
//    _print(*(m2 + 1));
//    delete [] m2;
    
    return 0;
}
