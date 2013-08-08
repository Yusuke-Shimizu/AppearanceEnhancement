//
//  main.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/03/11.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <iostream>
#include "ProCam.h"
#include "common.h"
#include "myOpenCV.h"

using namespace std;
using namespace cv;

// main method
int main(int argc, const char * argv[])
{
    // P = V^{-1} * (A^{-1} * C - F)
    Vec3b l_vecP(0, 0, 0), l_vecC(30, 90, 150), l_vecF(50, 40, 30);
//    Vec9d l_vecV(0.50, 0.51, 0.52,
//                 0.53, 0.54, 0.55,
//                 0.56, 0.57, 0.58);
//    Vec9d l_vecV(0.5, 0.5, 0.5,
//                 0.5, 0.5, 0.5,
//                 0.5, 0.5, 0.5);
    Vec9d l_vecV(0.9, 0.2, 0.1,
                 0.1, 0.9, 0.1,
                 0.2, 0.2, 0.9);
    Mat l_matP(3, 1, CV_64FC1), l_matC(3, 1, CV_64FC1), l_matF(3, 1, CV_64FC1), l_matV(3, 3, CV_64FC1);
    // Vec -> Mat
    Vec3d tmp = Vec3d(l_vecC);
    l_matC = Mat(tmp);
    tmp = Vec3d(l_vecF);
    l_matF = Mat(tmp);
    convertVecToMat(&l_matV, l_vecV);
    
    // calculation (P = V^{-1} * (A^{-1} * C - F))
    l_matP = l_matV.inv() * (l_matC - l_matF);
    cout << "l_mat.inv = " << l_matV.inv() << endl;
    cout << "C - F = " << l_matC - l_matF << endl;
    
    // Mat -> Vec
    tmp = Vec3d(l_matP);
    l_vecP = Vec3b(tmp);
    _print3(l_matC, l_matF, l_matV);
    _print(l_matP);
    _print(l_vecP);

    // プロカムの設定
    Size prjSize(PRJ_SIZE);
    ProCam procam(prjSize);
    procam.allCalibration();
    
    procam.doRadiometricCompensation(100);
    return 0;
}
