//
//  ProCam.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/01.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <iostream>
#include <fstream>  // save and load method
#include <string>   // use string in load method
#include "ProCam.h"
#include "LinearizerOfProjector.h"
#include "myOpenCV.h"
#include "GeometricCalibration.h"
#include "common.h"

using namespace std;
using namespace cv;

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
//    _print(videoSize);
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
bool ProCam::setAccessMapCam2Pro(const cv::Point* const accessMapCam2Pro, const cv::Size& mapSize){
    // setting
    for (int y = 0; y < mapSize.height; ++ y) {
        for (int x = 0; x < mapSize.width; ++ x) {
            int ptr = y * mapSize.width + x;
            *(m_accessMapCam2Pro + ptr) = *(accessMapCam2Pro + ptr);
        }
    }
    
    return true;
}
// 上のアクセスマップを自動で用いる関数
bool ProCam::setAccessMapCam2Pro(const cv::Point* const accessMapCam2Pro){
    // set projector size
    Size cameraSize;
    getCameraSize(&cameraSize);
    
    // setting
    if ( !setAccessMapCam2Pro(accessMapCam2Pro, cameraSize)) {
        cerr << "error of setting access map camera to projector" << endl;
        ERROR_PRINT(cameraSize);
        return false;
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
    return true;
}

///////////////////////////////  get method ///////////////////////////////
// m_cameraSizeの取得
bool ProCam::getCameraSize(cv::Size* const cameraSize){
    *cameraSize = m_cameraSize;
    return true;
}

// カメラのピクセル数を返す
// return   : カメラの全ピクセル数
int ProCam::getPixelsOfCamera(void){
    Size cameraSize;
    getCameraSize(&cameraSize);
    return cameraSize.area();
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
bool ProCam::getAccessMapCam2Pro(cv::Point* const accessMapCam2Pro, const cv::Size& mapSize){
    // get
    for (int y = 0; y < mapSize.height; ++ y) {
        for (int x = 0; x < mapSize.width; ++ x) {
            int ptr = y * mapSize.width + x;
            *(accessMapCam2Pro + ptr) = *(m_accessMapCam2Pro + ptr);
        }
    }
    
    return true;
}
bool ProCam::getAccessMapCam2Pro(cv::Point* const accessMapCam2Pro){
    // 実際のカメラのサイズを取得
    cv::Size cameraSize;
    getCameraSize(&cameraSize);
    
    // get
    getAccessMapCam2Pro(accessMapCam2Pro, cameraSize);
    
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

///////////////////////////////  save method ///////////////////////////////
bool ProCam::saveAccessMapCam2Pro(void){
    // init
    cout << "saving look up table..." << endl;
    Size cameraSize;
    getCameraSize(&cameraSize);
    const int cameraPixels = cameraSize.area();
    Point *accessMapCam2Pro = (Point*)malloc(sizeof(Point) * cameraPixels);
    getAccessMapCam2Pro(accessMapCam2Pro);
    
    // バイナリ出力モード（初期化を行う）
    ofstream ofs;
    ofs.open(LOOK_UP_TABLE_FILE_NAME, ios_base::out | ios_base::trunc | ios_base::binary);
    
    // write look up table size
    _print(cameraSize);
    ofs.write((char*)&cameraSize.width, sizeof(int));
    ofs.write((char*)&cameraSize.height, sizeof(int));
    
    // write table
    for (int y = 0; y < cameraSize.height; ++ y) {
        for (int x = 0; x < cameraSize.width; ++ x) {
            int ptr = y * cameraSize.width + x, tmpX = 0, tmpY = 0;
            getPoint(&tmpX, &tmpY, *(accessMapCam2Pro + ptr));
            ofs.write((char*)&tmpX, sizeof(int));
            ofs.write((char*)&tmpY, sizeof(int));
        }
    }

    cout << "finish saving!" << endl;
    return true;
}

///////////////////////////////  load method ///////////////////////////////
bool ProCam::loadAccessMapCam2Pro(void){
    cout << "loading look up table..." << endl;
    // init
    Size cameraSize;
    getCameraSize(&cameraSize);
    const int cameraPixels = cameraSize.area();
    Point *accessMapCam2Pro = (Point*)malloc(sizeof(Point) * cameraPixels);
    initPoint(accessMapCam2Pro, cameraPixels);
    ifstream ifs(LOOK_UP_TABLE_FILE_NAME);
    string str;
    
    // load size
    Size tableSize;
    ifs.read((char*)&tableSize.width, sizeof(int));
    ifs.read((char*)&tableSize.height, sizeof(int));
    _print(tableSize);
    if (tableSize != cameraSize) {
        cerr << "loaded size is different from current camera size" << endl;
        ERROR_PRINT2(cameraSize, tableSize);
        return false;
    }
    
    // load 
    for (int y = 0; y < cameraSize.height; ++ y) {
        for (int x = 0; x < cameraSize.width; ++ x) {
            int ptr = y * cameraSize.width + x, tmpX = 0, tmpY = 0;
            ifs.read((char*)&tmpX, sizeof(int));
            ifs.read((char*)&tmpY, sizeof(int));
            setPoint(accessMapCam2Pro + ptr, tmpX, tmpY);
        }
    }
    
    // check
    Size projectorSize(0, 0);
    getProjectorSize(&projectorSize);
//    GeometricCalibration gs;
//    gs.test_accessMap(accessMapCam2Pro, cameraSize, projectorSize, "load access map image");
//    gs.test_geometricCalibration(accessMapCam2Pro, &m_video, &cameraSize, &projectorSize);
    Point *true_accessMapCam2Pro = (Point*)malloc(sizeof(Point) * cameraPixels);
    getAccessMapCam2Pro(true_accessMapCam2Pro);
    if (isEqualPoint(accessMapCam2Pro, true_accessMapCam2Pro, cameraPixels)) {
        cout << "load is succeed!!!" << endl;
    } else {
        return false;
    }

    // setting look up table
    setAccessMapCam2Pro(accessMapCam2Pro);

    cout << "finish loading!" << endl;
    waitKey();
    return true;
}

///////////////////////////////  calibration method ///////////////////////////////
// 全てのキャリブレーションを行う
bool ProCam::allCalibration(void){
    // geometri calibration
    if ( !geometricCalibration() ) {
        cerr << "geometric calibration error" << endl;
        return false;
    }
    
    // linearized projector
    if ( !linearlizeOfProjector() ) {
        cerr << "linearized error of projector" << endl;
        return false;
    }
    
    // color caliration
    if ( !colorCalibration() ) {
        cerr << "color caliration error" << endl;
        return false;
    }
    
    return true;
}

// 幾何キャリブレーションを行う
bool ProCam::geometricCalibration(void){
    // init
    const int cameraPixels = getPixelsOfCamera();
    Point *accessMapCam2Pro = (Point*)malloc(sizeof(Point) * cameraPixels);
    initPoint(accessMapCam2Pro, cameraPixels);
    
    // geometric calibration
    GeometricCalibration gc = GeometricCalibration();
    if (!gc.doCalibration(accessMapCam2Pro)) {
        cerr << "error of geometric calibration" << endl;
        return false;
    }
    
    // 上で得たプロカム間のルックアップテーブルをクラス変数に代入
    // set class variable
    setAccessMapCam2Pro(accessMapCam2Pro);
    
    // save
    saveAccessMapCam2Pro();
    loadAccessMapCam2Pro();

    return true;
}

// 光学キャリブレーションを行う
bool ProCam::colorCalibration(void){
    
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
    if ( !setProjectorResponse(responsePrj, prjResSize) ) {ERROR_PRINT(prjResSize); return false;}
    
    // 後処理
    free(responsePrj);
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
