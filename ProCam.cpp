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
ProCam::ProCam(void)
    : m_accessMapCam2Pro(NULL), m_cameraResponse(NULL)
{
    init();
}

ProCam::ProCam(const cv::Size& projectorSize)
    : m_accessMapCam2Pro(NULL), m_cameraResponse(NULL)
{
    init(projectorSize);
}

ProCam::ProCam(const int _width, const int _height)
    : m_accessMapCam2Pro(NULL), m_cameraResponse(NULL)
{
    init(_width, _height);
}

ProCam::ProCam(const int _size)
    : m_accessMapCam2Pro(NULL), m_cameraResponse(NULL)
{
    init(_size);
}

///////////////////////////////  destructor ///////////////////////////////
// デストラクタ
ProCam::~ProCam(void){
    cout << "deleting ProCam (" << this <<")" << endl;
    delete [] m_accessMapCam2Pro;
    delete [] m_cameraResponse;
//    m_video.release();
    destroyWindow(WINDOW_NAME);     // 投影に使用したウィンドウを削除
    cout << "ProCam is deleted (" << this <<")" << endl;
}

///////////////////////////////  init method ///////////////////////////////

// ProCamクラスの初期化
bool ProCam::init(const cv::Size& projectorSize){
    // カメラの初期化
    if ( !initVideoCapture() ) return false;
    if ( !initCameraSize() ) return false;
//    if ( !initCameraResponseSize() ) return false;
//    if ( !initCameraResponse(getCameraResponseSize()) ) return false;
    
    // プロジェクタの初期化
    if ( !setProjectorSize(projectorSize) ) return false;
//    cv::Size* cameraSize = getCameraSize();
//    if ( !initProjectorResponseSize(*cameraSize) ) return false;
//    if ( !initProjectorResponseSize() ) return false;
    if ( !initProjectorResponse() ) return false;
//    if ( !linearlizeOfProjector() ) return false;
    
    // アクセスマップの初期化
    if (!initAccessMapCam2Pro() ) return false;
    
    return true;
}
bool ProCam::init(const int _width, const int _height){
    Size projectorSize(_width, _height);
    return init(projectorSize);
}
bool ProCam::init(const int _size) { return init(_size, _size); }
bool ProCam::init(void) { return init(100); }

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
    _print_name(m_video);
    return true;
}

// アクセスマップの初期化
bool ProCam::initAccessMapCam2Pro(void){
    // 実際のカメラのサイズを取得
    cv::Size cameraSize;
    getCameraSize(&cameraSize);
    
    // init
//    m_accessMapCam2Pro = (cv::Point*)malloc(sizeof(cv::Point) * cameraSize.area());
    m_accessMapCam2Pro = new cv::Point[cameraSize.area()];
    for (int y = 0; y < cameraSize.height; ++ y) {
        for (int x = 0; x < cameraSize.width; ++ x) {
            int ptr = y * cameraSize.width + x;
            *(m_accessMapCam2Pro + ptr) = cv::Size(0, 0);
        }
    }
    
    return true;
}

// カメラ応答特性の初期化
// input / camResSize   : 生成する応答特性の大きさ
// return               : 成功したかどうか
bool ProCam::initCameraResponse(const int camResSize){
    // error processing
    if (camResSize <= 0) return false;
    
    m_cameraResponse = new double[camResSize];
    for (int i = 0; i < camResSize; ++ i) {
        m_cameraResponse[i] = i;
    }
    return true;
}

// プロジェクタ応答特性の初期化
// return   : 成功したかどうか
bool ProCam::initProjectorResponse(void){
    // init
    const cv::Size* prjSize = getProjectorSize();
    const cv::Size prjResSize(prjSize->width * 256, prjSize->height);
    
    // set projector response
    m_projectorResponse = cv::Mat_<cv::Vec3b>(prjResSize);
    int rows = m_projectorResponse.rows, cols = m_projectorResponse.cols;
    for (int y = 0; y < rows; ++ y) {
        Vec3b* p_projectorResponse = m_projectorResponse.ptr<Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            // [0-255]の値を生成し代入
            int val = x % 256;
            Vec3b color(val, val, val);
            p_projectorResponse[x] = color;
//            _print2(x, color);
        }
    }
    return true;
}

///////////////////////////////  set method ///////////////////////////////
// m_cameraSizeの設定
bool ProCam::setCameraSize(const cv::Size* const cameraSize){
    m_cameraSize = *cameraSize;
    return true;
}

// m_projectorSizeの設定
bool ProCam::setProjectorSize(const cv::Size& projectorSize){
    m_projectorSize = projectorSize;
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

// プロジェクタ応答特性（m_projectorResponse）の設定
// input / prjResSize   : 設定したい大きさ
// return               : 成功したかどうか
bool ProCam::setProjectorResponse(const cv::Mat_<cv::Vec3b>* const _response){
    // error processing
    if ( !isEqualSizeAndType(*_response, m_projectorResponse)) {
        ERROR_PRINT("_response Type of Size is different from m_projectorResponse");
        return false;
    }
    
    // deep copy
    m_projectorResponse = _response->clone();
    return true;
}

///////////////////////////////  get method ///////////////////////////////
// m_cameraSizeの取得
bool ProCam::getCameraSize(cv::Size* const cameraSize){
    *cameraSize = m_cameraSize;
    return true;
}
cv::Size* ProCam::getCameraSize(void){
    return &m_cameraSize;
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
cv::Size* ProCam::getProjectorSize(void){
    return &m_projectorSize;
}

// m_videoから撮影画像を取得
// output/image : 取得した画像を入れる行列
// return       : 成功したかどうか
bool ProCam::getCaptureImage(cv::Mat* const image){
    m_video >> *image;
    return true;
}

// m_videoの取得
// output / _video
bool ProCam::getVideoCapture(cv::VideoCapture* _video){
    *_video = m_video;
    return true;
}
cv::VideoCapture* ProCam::getVideoCapture(void){
    return &m_video;
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
    if (!ofs) {
        ERROR_PRINT2(LOOK_UP_TABLE_FILE_NAME, "is Not Found");
        exit(-1);
    }
    
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

    free(accessMapCam2Pro);
    cout << "finish saving!" << endl;
    return true;
}

// m_projectorResponseを保存
bool ProCam::saveProjectorResponse(const char* fileName){
    const Mat_<Vec3b>* const l_proRes = &m_projectorResponse;
    ofstream ofs;
//    ofs.open(fileName, ios_base::out | ios_base::trunc | ios_base::binary);
    ofs.open(fileName, ios_base::out | ios_base::trunc);
    if (!ofs) {
        ERROR_PRINT2(fileName, "is Not Found");
        exit(-1);
    }
    
    // write
    int rows = l_proRes->rows, cols = l_proRes->cols;
    ofs << rows << ", " << cols << endl;
    for (int y = 0; y < rows; ++ y) {
        const Vec3b* p_proRes = l_proRes->ptr<Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            ofs << p_proRes[x] << ", ";
        }
        ofs << endl;
    }
    
    return true;
}

bool ProCam::saveProjectorResponse(const char* fileName, const uchar index, const uchar color){
    const Mat_<Vec3b>* const l_proRes = &m_projectorResponse;
    ofstream ofs;
    //    ofs.open(fileName, ios_base::out | ios_base::trunc | ios_base::binary);
    ofs.open(fileName, ios_base::out | ios_base::trunc);
    if (!ofs) {
        ERROR_PRINT2(fileName, "is Not Found");
        exit(-1);
    }
    
    // write
    int rows = l_proRes->rows, cols = l_proRes->cols;
    ofs << rows << ", " << cols << endl;
    const Vec3b* p_proRes = l_proRes->ptr<Vec3b>(0);
    for (int x = 256 * index; x < 256 * (index + 1); ++ x) {
        ofs << (int)p_proRes[x][color] << ", ";
    }
    ofs << endl;
    
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
        free(true_accessMapCam2Pro);
    } else {
        free(true_accessMapCam2Pro);
        return false;
    }

    // setting look up table
    setAccessMapCam2Pro(accessMapCam2Pro);
    free(accessMapCam2Pro);
    
    cout << "finish loading!" << endl;
    return true;
}

///////////////////////////////  calibration method ///////////////////////////////
// 全てのキャリブレーションを行う
bool ProCam::allCalibration(void){
    Mat projectionImage = Mat::zeros(PRJ_SIZE_HEIGHT, PRJ_SIZE_WIDTH, CV_8UC3);
    imshow(WINDOW_NAME, projectionImage);
    cvMoveWindow(WINDOW_NAME, -300, -1 * projectionImage.rows);
    cv::waitKey(1);
    // geometri calibration
//    if ( !geometricCalibration() ) {
//        cerr << "geometric calibration error" << endl;
//        return false;
//    }
    
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
    VideoCapture* video = getVideoCapture();
//    getVideoCapture(&video);
    _print_name(*video);
    GeometricCalibration gc;
    if (!gc.doCalibration(accessMapCam2Pro, video)) {
        cerr << "error of geometric calibration" << endl;
        return false;
    }
    
    // 上で得たプロカム間のルックアップテーブルをクラス変数に代入
    // set class variable
    setAccessMapCam2Pro(accessMapCam2Pro);
    free(accessMapCam2Pro);
    
    // save
    saveAccessMapCam2Pro();
    loadAccessMapCam2Pro();

    return true;
}

// 光学キャリブレーションを行う
bool ProCam::colorCalibration(void){
    cout << "color calibration start!" << endl;
    cout << "color calibration finish!" << endl;
    return true;
}

// プロジェクタの線形化を行う
// return   : 成功したかどうか
bool ProCam::linearlizeOfProjector(void){
    // 線形化配列を一旦ローカルに落とす
    int rows = m_projectorResponse.rows, cols = m_projectorResponse.cols;
    Mat_<Vec3b> prjResponse(rows, cols);
    for (int y = 0; y < rows; ++ y) {
        Vec3b* p_prjRes = prjResponse.ptr<Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            int val = x % 256;
            Vec3b color(val);
            p_prjRes[x] = color;
        }
    }
    
    // get projector response
    LinearizerOfProjector linearPrj(this);
    if ( !linearPrj.linearlize(&prjResponse) ) return false;
    
    // 落とした配列をメンバ配列に代入する
    if ( !setProjectorResponse(&prjResponse) ) {ERROR_PRINT("error is setProjectorResponse"); return false;}
    cout << "saving projector response" << endl;
    saveProjectorResponse(PROJECTOR_RESPONSE_FILE_NAME_02, 0, 0);
    cout << "saved projector response" << endl;
    return true;
}

///////////////////////////////  other method ///////////////////////////////

// output / captureImage    : 撮影した画像を代入する場所
// input / projectionImage  : 投影する画像
// return                   : 成功したかどうか
bool ProCam::captureFromLight(cv::Mat* const captureImage, const cv::Mat& projectionImage){
    // 投影
    imshow(WINDOW_NAME, projectionImage);
    cv::waitKey(SLEEP_TIME);
    
    // N回撮影する
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
