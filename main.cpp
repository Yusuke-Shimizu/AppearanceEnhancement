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

#ifdef LINUX
#define EXPERIMENT_FLAG
#endif

using namespace std;
using namespace cv;

// main method
int main(int argc, const char * argv[])
{
    // 64F, channles=1, 2x2x3x4 の4次元配列
    const int sizes[] = {2, 2, 3, 3};
    cv::Mat m3(sizeof(sizes)/sizeof(int), sizes, CV_64FC1);
    
    std::cout << "mat3"<< std::endl;
    std::cout << "  dims: " << m3.dims << ", depth(byte/channel):" << m3.elemSize1() \
    << ", channels: " << m3.channels() << std::endl;
    _print_mat_propaty(m3);
//    for (MatIterator_<double> l_matItr = m3.begin<double>();
//         l_matItr != m3.end<double>();
//         ++ l_matItr) {
//        _print(*l_matItr);
//    }
    MatIterator_<double> l_matItr = m3.begin<double>();
    for (int y = 0; y < m3.size[0]; ++ y) {
        for (int x = 0; x < m3.size[1]; ++ x) {
            for (int ny = 0; ny < m3.size[2]; ++ ny) {
                for (int nx = 0 ; nx < m3.size[3]; ++ nx, ++ l_matItr) {
//                    cout << m3.at<double>(x, y, nx, ny) << " ";
                    cout << *l_matItr << " ";
                }
                cout << endl;
            }
            cout << endl;
        }
        cout << endl;
    }
    
    Vec3d l_color[3]={{0,1, 2},{3,4, 5},{6,7, 8}};
    std::vector<cv::Mat> l_vecMat;
    for (int i = 0; i < 3; ++ i) {
        Mat l_mat(l_color[i]);
        l_vecMat.push_back(l_mat);
    }
    Mat l_merge;
    cv::merge(l_vecMat, l_merge);
    _print(l_merge);
//#ifdef EXPERIMENT_FLAG
//    // experimentation
//    AppearanceEnhancement ae(PRJ_SIZE);
//    ae.doAppearanceEnhancementByAmano();
//#else
//    // simulation
//    AppearanceEnhancement ae(0.8, 0.2);
////    ae.test_RadiometricModel();
////    ae.test_calcReflectanceAtPixel();
////    ae.test_calcReflectanceAndAmbientLightAtPixel();
//    ae.test_calcNextProjectionImageAtPixel();
//#endif
    return 0;
}
