//
//  ProCam.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/01.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <iostream>
#include "ProCam.h"
#include "myOpenCV.h"
#include "common.h"

using namespace std;
using namespace cv;

// コンストラクタ
ProCam::ProCam(const cv::Size* const cameraSize, const cv::Size* const projectorSize, const int cameraBitDepth, const int projectorBitDepth){
    init(cameraSize, projectorSize, cameraBitDepth, projectorBitDepth);
}

///////////////////////////////  set method ///////////////////////////////
// m_cameraSizeの設定
bool ProCam::setCameraSize(const cv::Size* const cameraSize){
    m_cameraSize = *cameraSize;
    return true;
}

// m_projectorSizeの設定
bool ProCam::setProjectorSize(const cv::Size* const projectorSize){
    m_projectorSize = *projectorSize;
    return true;
}

// m_accessMapCam2Proの設定
// input / accessMapCam2Pro   : セッティングしたいアクセスマップデータ
// input / mapSize            : アクセスマップの大きさ
bool ProCam::setAccessMapCam2Pro(const cv::Point* const accessMapCam2Pro, const cv::Size* const mapSize){
    // 実際のカメラのサイズを取得
    Size cameraSize;
    getCameraSize(&cameraSize);
    
    // error processing
    if (cameraSize.width != mapSize->width) {
        cerr << "error is width of size" << endl;
        ERROR_PRINT(cameraSize);
        ERROR_PRINT(mapSize);
        return false;
    } else if (cameraSize.height != mapSize->height) {
        cerr << "error is height of size" << endl;
        ERROR_PRINT(cameraSize);
        ERROR_PRINT(mapSize);
        return false;
    }
    
    // setting
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x) {
            int ptr = y * mapSize->width + x;
            *(m_accessMapCam2Pro + ptr) = *(accessMapCam2Pro + ptr);
        }
    }
    
    return true;
}

///////////////////////////////  get method ///////////////////////////////
// m_cameraSizeの取得
bool ProCam::getCameraSize(cv::Size* const cameraSize){
    *cameraSize = m_cameraSize;
    return true;
}

// m_projectorSizeの取得
bool ProCam::getProjectorSize(cv::Size* const projectorSize){
    *projectorSize = m_projectorSize;
    return true;
}

// m_videoから撮影画像を取得
// output/image : 取得した画像を入れる行列
// return       : 成功したかどうか
bool ProCam::getCaptureImage(cv::Mat* const image){
    m_video >> *image;
    return true;
}

// m_accessMapCam2Proの取得
// output / accessMapCam2Pro    : 取得したアクセスマップを格納する変数
// input / mapSize              : アクセスマップの大きさ
bool ProCam::getAccessMapCam2Pro(cv::Point* const accessMapCam2Pro, const cv::Size* const mapSize){
    // 実際のカメラのサイズを取得
    Size cameraSize;
    getCameraSize(&cameraSize);
    
    // error processing
    if (cameraSize.width != mapSize->width) {
        cerr << "error is width of size" << endl;
        ERROR_PRINT(cameraSize);
        ERROR_PRINT(mapSize);
        return false;
    } else if (cameraSize.height != mapSize->height) {
        cerr << "error is height of size" << endl;
        ERROR_PRINT(cameraSize);
        ERROR_PRINT(mapSize);
        return false;
    }

    // get
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x) {
            int ptr = y * mapSize->width + x;
            *(accessMapCam2Pro + ptr) = *(m_accessMapCam2Pro + ptr);
        }
    }
    
    return true;
}

///////////////////////////////  init method ///////////////////////////////

// ProCamクラスの初期化
bool ProCam::init(const cv::Size* const cameraSize, const cv::Size* const projectorSize, const int cameraBitDepth, const int projectorBitDepth){
    setCameraSize(cameraSize);
    setProjectorSize(projectorSize);
    initVideoCapture();
    return true;
}

// m_cameraSizeの初期化
// return   : 成功したかどうか
////////////////////////////////ここ作る！！！///////////////////////////////////////////
bool ProCam::initCameraSize(void){
    // videoCaptureからサイズを取得する
    //m_cameraSize = Size();
    return true;
}
////////////////////////////////ここ作る！！！///////////////////////////////////////////

// ビデオキャプチャーの初期化
// return   : 成功したかどうか
bool ProCam::initVideoCapture(void){
    m_video = VideoCapture(0);
    if( !m_video.isOpened() ){
        std::cerr << "ERROR : camera is not opened !!" << std::endl;
        return false;
    }
    return true;
}

// アクセスマップの初期化
bool ProCam::initAccessMapCam2Pro(cv::Point* const accessMapCam2Pro, const cv::Size* const mapSize){
    // 実際のカメラのサイズを取得
    Size cameraSize;
    getCameraSize(&cameraSize);
    
    // error processing
    if (cameraSize.width != mapSize->width) {
        cerr << "error is width of size" << endl;
        ERROR_PRINT(cameraSize);
        ERROR_PRINT(mapSize);
        return false;
    } else if (cameraSize.height != mapSize->height) {
        cerr << "error is height of size" << endl;
        ERROR_PRINT(cameraSize);
        ERROR_PRINT(mapSize);
        return false;
    }
    
    // init
    m_accessMapCam2Pro = (Point*)malloc(sizeof(Point) * mapSize->area());
    for (int y = 0; y < mapSize->height; ++ y) {
        for (int x = 0; x < mapSize->width; ++ x) {
            int ptr = y * mapSize->width + x;
            *(m_accessMapCam2Pro + ptr) = Size(0, 0);
        }
    }
    
    return true;
}

///////////////////////////////  other method ///////////////////////////////

// output / captureImage    : 撮影した画像を代入する場所
// input / projectionImage  : 投影したい画像
// return                   : 成功したかどうか
bool ProCam::captureFromLight(cv::Mat* const captureImage, const cv::Mat* const projectionImage){
    return true;
}
