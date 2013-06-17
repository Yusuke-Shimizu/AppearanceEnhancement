//
//  ProCam.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/01.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <iostream>
#include "ProCam.h"
#include "LinearizerOfProjector.h"
#include "myOpenCV.h"
#include "common.h"

//using namespace std;
//using namespace cv;

///////////////////////////////  constructor ///////////////////////////////
// コンストラクタ
ProCam::ProCam(const cv::Size* const projectorSize){
    init(projectorSize);
}

///////////////////////////////  destructor ///////////////////////////////
// デストラクタ
ProCam::~ProCam(void){
    free(m_accessMapCam2Pro);
    free(m_cameraResponse);
    free(m_projectorResponse);
    m_video.release();
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
    cv::Size cameraSize;
    getCameraSize(&cameraSize);
    
    // error processing
    if (cameraSize.width != mapSize->width) {
        std::cerr << "error is width of size" << std::endl;
        ERROR_PRINT(cameraSize);
        ERROR_PRINT(mapSize);
        return false;
    } else if (cameraSize.height != mapSize->height) {
        std::cerr << "error is height of size" << std::endl;
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

// カメラ応答特性サイズ（m_cameraResponseSize）の設定
// input / camResSize   : 設定したい大きさ
// return               : 成功したかどうか
bool ProCam::setCameraResponseSize(const int camResSize){
    if (camResSize <= 0) return false;
    m_cameraResponseSize = camResSize;
    return true;
}

// プロジェクタ応答特性サイズ（m_projectorResponseSize）の設定
// input / prjResSize   : 設定したい大きさ
// return               : 成功したかどうか
bool ProCam::setProjectorResponseSize(const int prjResSize){
    if (prjResSize <= 0) return false;
    m_projectorResponseSize = prjResSize;
    return true;
}

bool ProCam::setCameraResponse(const double* const camRes, const int camResSize){
    // error processing
    if (camResSize <= 0) return false;
    
    // setting
    for (int i = 0; i < camResSize; ++ i) {
        *(m_cameraResponse + i) = *(camRes + i);
    }

    return true;
}
bool ProCam::setProjectorResponse(const double* const prjRes, const int prjResSize){
    // error processing
    if (prjResSize <= 0) return false;
    
    // setting
    for (int i = 0; i < prjResSize; ++ i) {
        *(m_projectorResponse + i) = *(prjRes + i);
    }
    _print(prjResSize);
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
    cv::Size cameraSize;
    getCameraSize(&cameraSize);
    
    // error processing
    if (cameraSize.width != mapSize->width) {
        std::cerr << "error is width of size" << std::endl;
        ERROR_PRINT(cameraSize);
        ERROR_PRINT(mapSize);
        return false;
    } else if (cameraSize.height != mapSize->height) {
        std::cerr << "error is height of size" << std::endl;
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

// カメラ応答特性サイズの取得
// return   : 応答特性の大きさ
int ProCam::getCameraResponseSize(void){
    return m_cameraResponseSize;
}

// カメラ応答特性サイズの取得
// return   : 応答特性の大きさ
int ProCam::getProjectorResponseSize(void){
    return m_projectorResponseSize;
}

///////////////////////////////  init method ///////////////////////////////

// ProCamクラスの初期化
bool ProCam::init(const cv::Size* const projectorSize){
    // カメラの初期化
    if ( !initVideoCapture() ) return false;
    if ( !initCameraSize() ) return false;
    if ( !initCameraResponseSize() ) return false;
    if ( !initCameraResponse(getCameraResponseSize()) ) return false;

    // プロジェクタの初期化
    if ( !setProjectorSize(projectorSize) ) return false;
    if ( !initProjectorResponseSize() ) return false;
    if ( !initProjectorResponse(getProjectorResponseSize()) ) return false;
    if ( !linearlizeOfProjector() ) return false;
    
    // アクセスマップの初期化
    if (!initAccessMapCam2Pro() ) return false;
    
    return true;
}

// m_cameraSizeの初期化
// return   : 成功したかどうか
bool ProCam::initCameraSize(void){
    // videoCaptureからサイズを取得する
    cv::Size videoSize(m_video.get(CV_CAP_PROP_FRAME_WIDTH), m_video.get(CV_CAP_PROP_FRAME_HEIGHT));
    _print(videoSize);
    // setting
    if( !setCameraSize(&videoSize) ) return false;

    return true;
}

// ビデオキャプチャーの初期化
// return   : 成功したかどうか
bool ProCam::initVideoCapture(void){
    m_video = cv::VideoCapture(0);
    if( !m_video.isOpened() ){
        std::cerr << "ERROR : camera is not opened !!" << std::endl;
        return false;
    }
    return true;
}

// アクセスマップの初期化
bool ProCam::initAccessMapCam2Pro(void){
    // 実際のカメラのサイズを取得
    cv::Size cameraSize;
    getCameraSize(&cameraSize);
    
    // init
    m_accessMapCam2Pro = (cv::Point*)malloc(sizeof(cv::Point) * cameraSize.area());
    for (int y = 0; y < cameraSize.height; ++ y) {
        for (int x = 0; x < cameraSize.width; ++ x) {
            int ptr = y * cameraSize.width + x;
            *(m_accessMapCam2Pro + ptr) = cv::Size(0, 0);
        }
    }
    
    return true;
}

// カメラ応答特性サイズ（m_cameraResponseSize）の初期化
// return   : 成功したかどうか
bool ProCam::initCameraResponseSize(void){
    if ( !setCameraResponseSize(RESPONSE_SIZE) ) return false;
    return true;
}

// プロジェクタ応答特性サイズ（m_projectorResponseSize）の初期化
// return   : 成功したかどうか
bool ProCam::initProjectorResponseSize(void){
    if ( !setProjectorResponseSize(RESPONSE_SIZE) ) return false;
    return true;
}

// カメラ応答特性の初期化
// input / camResSize   : 生成する応答特性の大きさ
// return               : 成功したかどうか
bool ProCam::initCameraResponse(const int camResSize){
    if (camResSize <= 0) return false;
    m_cameraResponse = (double*)malloc(sizeof(double) * camResSize);
    memset(m_cameraResponse, 0, sizeof(double) * camResSize);
    return true;
}

// プロジェクタ応答特性の初期化
// input / prjResSize   : 生成する応答特性の大きさ
// return               : 成功したかどうか
bool ProCam::initProjectorResponse(const int prjResSize){
    if (prjResSize <= 0) return false;
    m_projectorResponse = (double*)malloc(sizeof(double) * prjResSize);
    memset(m_projectorResponse, 0, sizeof(double) * prjResSize);
    return true;
}

///////////////////////////////  other method ///////////////////////////////

// output / captureImage    : 撮影した画像を代入する場所
// input / projectionImage  : 投影したい画像
// return                   : 成功したかどうか
bool ProCam::captureFromLight(cv::Mat* const captureImage, const cv::Mat* const projectionImage){
    // 投影
    imshow("projection", *projectionImage);
    cv::waitKey(SLEEP_TIME);
    
    // 撮影
    cv::Mat image;
    for (int i = 0; i < CAPTURE_NUM; ++ i) {
        getCaptureImage(&image);
    }
    *captureImage = image.clone();
    //imshow(W_NAME_GEO_CAMERA, *captureImage);
	//cvMoveWindow(W_NAME_GEO_CAMERA, projectionImage->rows, 0);
    cv::waitKey(SLEEP_TIME);

    return true;
}

// プロジェクタの線形化を行う
// return   : 成功したかどうか
bool ProCam::linearlizeOfProjector(void){
    // 線形化配列を一旦ローカルに落とす
    int prjResSize = getProjectorResponseSize();
    double* responsePrj = (double*)malloc(sizeof(double) * prjResSize);
    if ( !linearPrj->linearlize(responsePrj) ) return false;
    
    // 落とした配列をメンバ配列に代入する
    if ( !setProjectorResponse(responsePrj, prjResSize) ) return false;
    
    // 後処理
    free(responsePrj);
    return true;
}

