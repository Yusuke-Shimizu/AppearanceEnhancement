//
//  LinearizerOfProjector.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/07.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//
/*

 以下の論文を参考にプロジェクタの線形化を行うクラス
 Nayar, Shree K., et al. "A projection system with radiometric compensation for screen imperfections." ICCV Workshop on Projector-Camera Systems (PROCAMS). Vol. 3. 2003.
 
 １．色変換行列を算出
 １．１．３x３の行列を準備
 １．２．対角成分を１に設定
 １．３．RGBと黒を投影・撮影
 １．４．それらの差分より他の成分を決定
 2．プロジェクタの線形化
 ２．１．０から２５５までの値を投影・撮影
 ２．２．それらに対する値のルックアップテーブルを作成

 */

#include "LinearizerOfProjector.h"
#include "myOpenCV.h"
#include "ProCam.h"
#include "common.h"

using namespace std;
using namespace cv;

///////////////////////////////  constructor ///////////////////////////////
LinearizerOfProjector::LinearizerOfProjector(void){
//    initColorMixingMatrix();
}

LinearizerOfProjector::LinearizerOfProjector(ProCam* procam){
    _print_name(*procam);
    setProCam(procam);
    _print_name(*m_procam);
    const cv::Size* cameraSize = m_procam->getCameraSize();
    initColorMixingMatrix(*cameraSize);
}

///////////////////////////////  destructor ///////////////////////////////
LinearizerOfProjector::~LinearizerOfProjector(void){
//    free(m_procam);
    m_procam->~ProCam();
    free(m_aColorMixingMatrix);
    cout << "LinearizerOfProjector is deleted (" << this << ")" << endl;
}

///////////////////////////////  set method ///////////////////////////////
// m_ColorMixingMatrix（色変換行列）の設定
// input / colMix   : 設定する行列
// return           : 成功したかどうか
bool LinearizerOfProjector::setColorMixingMatrix(cv::Mat** colMix){
    m_aColorMixingMatrix = colMix;
    return true;
}

// プロカムのセッティング
bool LinearizerOfProjector::setProCam(ProCam* procam){
    m_procam = procam;
    _print_name(*procam);
    return true;
}

///////////////////////////////  get method ///////////////////////////////
// m_ColorMixingMatrix（色変換行列）の取得
// output / colMix  : 色変換行列を入れる行列
// return           : 成功したかどうか
bool LinearizerOfProjector::getColorMixingMatrix(cv::Mat** colMix){
    colMix = m_aColorMixingMatrix;
    return true;
}

//
bool LinearizerOfProjector::getProCam(ProCam* const procam){
    *procam = *m_procam;
    return true;
}
ProCam* LinearizerOfProjector::getProCam(void){
    _print_name(*m_procam);
    return m_procam;
}

///////////////////////////////  init method ///////////////////////////////
// init method
bool LinearizerOfProjector::initColorMixingMatrix(const cv::Size& _mixMatSize){
    // 3x3で対角成分が１、残りが０の行列を生成
    m_aColorMixingMatrix = (cv::Mat**)malloc(sizeof(cv::Mat*) * _mixMatSize.area());
    
    
    for (int i = 0; i < _mixMatSize.area(); ++ i) {
//        m_aColorMixingMatrix[i]->create(3, 3, CV_64FC1);
    }
    _print(m_aColorMixingMatrix[0]);
    
    Mat test = Mat::zeros(3, 3, CV_64FC1);
    test.create(3, 3, CV_64FC1);
    _print(test);
    
    Mat* p_test = (cv::Mat*)malloc(sizeof(cv::Mat) * _mixMatSize.area());
    if (p_test == NULL) {
        _print_name(p_test);
        cout << "Malloc Error of Mat*" << endl;
        exit(-1);
    }
    _print_name(*p_test);
    for (int i = 0; i < _mixMatSize.area(); ++ i) {
//        _print(i);
//        _print_name(p_test[i]);
//        p_test[i] = Mat::zeros(3, 3, CV_64FC1);
//        p_test[i].create(3, 3, CV_64FC1);
    }
    Vector<Mat_<double>> a_test;
    for (int i = 0; i < 3; ++ i) {
        Mat mat = Mat::zeros(3, 3, CV_64FC1);
        a_test.push_back(mat);
    }
    for (int i = 0; i < 3; ++ i) {
        _print(a_test[i]);
    }
    
//    p_test
//    p_test->create(3, 3, CV_64FC1);
//    *p_test = Mat::zeros(3, 3, CV_64FC1);
//    p_test = Mat::zeros(3, 3, CV_64FC1);
//    p_test = Mat::create(3, 3, CV_64FC1);
//    *p_test = Mat::create(3, 3, CV_64FC1);
    
    
    _print_name(p_test);

    
    return true;
}

///////////////////////////////  other method ///////////////////////////////
// プロジェクタの線形化を行うメソッド
// input / responseOfProjector  : 線形化のルックアップテーブルを入れる配列
// return   : 成功したかどうか
bool LinearizerOfProjector::linearlize(double* const responseOfProjector){
    // 色変換行列の生成
    if( !calcColorMixingMatrix() ) return false;
    
    return true;
}

// 色変換行列の算出
bool LinearizerOfProjector::calcColorMixingMatrix(void){
    // init
    ProCam *procam = getProCam();
    _print_name(*procam);
    Size *cameraSize = procam->getCameraSize(), *projectorSize = procam->getProjectorSize();
    
    // init capture image
    uchar depth8x3 = CV_8UC3;
    cv::Mat black_cap   (*cameraSize, depth8x3, cv::Scalar(0, 0, 0));
    cv::Mat red_cap     (*cameraSize, depth8x3, cv::Scalar(0, 0, 255));
    cv::Mat green_cap   (*cameraSize, depth8x3, cv::Scalar(0, 255, 0));
    cv::Mat blue_cap    (*cameraSize, depth8x3, cv::Scalar(255, 0, 0));
    {
        // init projection image
        cv::Mat black_img   (*projectorSize, depth8x3, cv::Scalar(0, 0, 0));
        cv::Mat red_img     (*projectorSize, depth8x3, cv::Scalar(0, 0, 255));
        cv::Mat green_img   (*projectorSize, depth8x3, cv::Scalar(0, 255, 0));
        cv::Mat blue_img    (*projectorSize, depth8x3, cv::Scalar(255, 0, 0));

        // projection and capture image
        procam->captureFromLight(&black_cap, black_img);
        procam->captureFromLight(&red_cap, red_img);
        procam->captureFromLight(&green_cap, green_img);
        procam->captureFromLight(&blue_cap, blue_img);
    }
    
    // show image
    imshow("black_cap", black_cap);
    imshow("red_cap", red_cap);
    imshow("green_cap", green_cap);
    imshow("blue_cap", blue_cap);
    
    // translate bit depth (uchar[0-255] -> double[0-1])
    uchar depth64x3 = CV_64FC3;
    black_cap.convertTo(black_cap, depth64x3, 1.0/255.0);
    red_cap.convertTo(red_cap, depth64x3, 1.0/255.0);
    green_cap.convertTo(green_cap, depth64x3, 1.0/255.0);
    blue_cap.convertTo(blue_cap, depth64x3, 1.0/255.0);
    
    // calc difference[-1-1]
    Mat diffRedAndBlack = red_cap - black_cap;
    Mat diffGreenAndBlack = green_cap - black_cap;
    Mat diffBlueAndBlack = blue_cap - black_cap;
    black_cap.release();
    red_cap.release();
    green_cap.release();
    blue_cap.release();
    
    // show difference image
    imshow("diffRedAndBlack", diffRedAndBlack);
    imshow("diffGreenAndBlack", diffGreenAndBlack);
    imshow("diffBlueAndBlack", diffBlueAndBlack);
    
    // image divided by any color element
    Point point(0, 0);
    const Vec3d* redV = getPixelNumd(diffRedAndBlack, point);
    _print_vector(*redV);
    divMatByRedElm(&diffRedAndBlack, RED);
    divMatByRedElm(&diffGreenAndBlack, GREEN);
    divMatByRedElm(&diffBlueAndBlack, BLUE);
    imshow("diffRedAndBlack2", diffRedAndBlack);
    imshow("diffGreenAndBlack2", diffGreenAndBlack);
    imshow("diffBlueAndBlack2", diffBlueAndBlack);
    const Vec3d* redV2 = getPixelNumd(diffRedAndBlack, point);
    _print_vector(*redV2);
    
    waitKey(-1);
    
    return true;
}
