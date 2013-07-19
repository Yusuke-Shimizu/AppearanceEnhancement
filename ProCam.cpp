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
    if ( !initProjectorResponseP2I() ) return false;
    
    // アクセスマップの初期化
    if ( !initAccessMapCam2Pro() ) return false;
    if ( !initAccessMapCam2Prj() ) return false;
    
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
bool ProCam::initAccessMapCam2Prj(void){
    cv::Size* camSize = getCameraSize();
    m_accessMapCam2Prj = Mat::zeros(*camSize, CV_16SC1);
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

bool ProCam::initProjectorResponseP2I(void){
    // init
    const cv::Size* prjSize = getProjectorSize();
    const cv::Size prjResSize(prjSize->width * 256, prjSize->height);
    
    // set projector response
    m_projectorResponseP2I = cv::Mat_<cv::Vec3b>(prjResSize);
    initProjectorResponseP2I(&m_projectorResponseP2I);
    return true;
}

bool ProCam::initProjectorResponseP2I(cv::Mat* const _prjResP2I){
    int rows = _prjResP2I->rows, cols = _prjResP2I->cols;
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pPrjResP2I = _prjResP2I->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols / 256; ++ x) {
            for (int i = 1; i < 255; ++ i) {
                l_pPrjResP2I[x * 256 + i] = Vec3b(INIT_RES_NUM, INIT_RES_NUM, INIT_RES_NUM);
            }
            l_pPrjResP2I[x * 256 + 0] = Vec3b(0, 0, 0);
            l_pPrjResP2I[x * 256 + 255] = Vec3b(255, 255, 255);
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
bool ProCam::setAccessMapCam2Prj(const cv::Mat_<cv::Vec2i>& _accessMapCam2Prj){
    // error
    if (!isEqualSizeAndType(_accessMapCam2Prj, m_accessMapCam2Prj)) {
        cerr << "size or type is different" << endl;
        _print_mat_propaty(_accessMapCam2Prj);
        _print_mat_propaty(m_accessMapCam2Prj);
        return false;
    }
    
    // deep copy
    m_accessMapCam2Prj = _accessMapCam2Prj.clone();
    
    return true;
}

// プロジェクタ応答特性（m_projectorResponse）の設定
// input / prjResSize   : 設定したい大きさ
// return               : 成功したかどうか
bool ProCam::setProjectorResponse(const cv::Mat_<cv::Vec3b>& _response){
    // error processing
    if ( !isEqualSizeAndType(_response, m_projectorResponse)) {
        ERROR_PRINT("_response Size or Type is different from m_projectorResponse");
        _print_mat_propaty(_response);
        _print_mat_propaty(m_projectorResponse);
        return false;
    }
    
    // deep copy
    m_projectorResponse = _response.clone();
    return true;
}

bool ProCam::setProjectorResponseP2I(const cv::Mat_<cv::Vec3b>& _response){
    // error processing
    if ( !isEqualSizeAndType(_response, m_projectorResponseP2I)) {
        ERROR_PRINT("_response Size or Type is different from m_projectorResponse");
        _print_mat_propaty(_response);
        _print_mat_propaty(m_projectorResponseP2I);
        return false;
    }
    
    // deep copy
    m_projectorResponseP2I = _response.clone();
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
const cv::Mat_<cv::Vec2i>* ProCam::getAccessMapCam2Prj(void){
    return &m_accessMapCam2Prj;
}

const cv::Mat_<cv::Vec3b>* ProCam::getProjectorResponse(void){
    return &m_projectorResponse;
}
const cv::Mat_<cv::Vec3b>* ProCam::getProjectorResponseP2I(void){
    return &m_projectorResponseP2I;
}
void ProCam::getProjectorResponseP2I(cv::Mat* const _responseImage, const int _index){
    const Mat_<Vec3b>* l_prjResP2I = getProjectorResponseP2I();
    if (_responseImage->rows != l_prjResP2I->rows || _responseImage->cols != l_prjResP2I->cols / 256) {
        cerr << "different size" << endl;
        _print_mat_propaty(*_responseImage);
        _print_mat_propaty(*l_prjResP2I);
        exit(-1);
    }
    const int rows = _responseImage->rows, cols = _responseImage->cols;
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        const Vec3b* l_pPrjResP2I = l_prjResP2I->ptr<Vec3b>(y);
        Vec3b* l_pResImg = _responseImage->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            l_pResImg[x] = l_pPrjResP2I[x * 256 + _index];
        }
    }
}

///////////////////////////////  save method ///////////////////////////////
bool ProCam::saveAccessMapCam2Prj(void){
    // init
    cout << "saving look up table..." << endl;
    const Mat_<Vec2i>* l_accessMapC2P = getAccessMapCam2Prj();
    
    // バイナリ出力モード（初期化を行う）
    ofstream ofs;
    ofs.open(LOOK_UP_TABLE_FILE_NAME, ios_base::out | ios_base::trunc | ios_base::binary);
    if (!ofs) {
        ERROR_PRINT2(LOOK_UP_TABLE_FILE_NAME, "is Not Found");
        exit(-1);
    }
    
    // write look up table size
    int rows = l_accessMapC2P->rows, cols = l_accessMapC2P->cols;
    ofs.write((char*)&rows, sizeof(int));
    ofs.write((char*)&cols, sizeof(int));
    
    // write table
    if (l_accessMapC2P->isContinuous()) {
        cols *= rows;
        rows = 1;
    }
    Point tmp(0, 0);
    for (int y = 0; y < rows; ++ y) {
        const Vec2i* l_pAccessMapC2P = l_accessMapC2P->ptr<Vec2i>(y);
        
        for (int x = 0; x < cols; ++ x) {
            tmp = Point(l_pAccessMapC2P[x]);
            ofs.write((char*)&tmp.x, sizeof(int));
            ofs.write((char*)&tmp.y, sizeof(int));
        }
    }
    
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

bool ProCam::saveProjectorResponseForByte(const char* fileName){
    // init
    const Mat_<Vec3b>* const l_proRes = &m_projectorResponse;
    ofstream ofs;
    ofs.open(fileName, ios_base::out | ios_base::trunc | ios_base::binary);
    if (!ofs) {
        ERROR_PRINT2(fileName, "is Not Found");
        exit(-1);
    }
    
    // write
    int rows = l_proRes->rows, cols = l_proRes->cols;
    ofs.write((char*)&rows, sizeof(int));
    ofs.write((char*)&cols, sizeof(int));
    for (int y = 0; y < rows; ++ y) {
        const Vec3b* p_proRes = l_proRes->ptr<Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            for (int ch = 0; ch < 3; ++ ch) {
                ofs.write((char*)&p_proRes[x][ch], sizeof(uchar));
            }
        }
    }
    
    return true;
}

bool ProCam::saveProjectorResponseP2IForByte(const char* fileName){
    // init
    const Mat_<Vec3b>* const l_proRes = getProjectorResponseP2I();
    ofstream ofs;
    ofs.open(fileName, ios_base::out | ios_base::trunc | ios_base::binary);
    if (!ofs) {
        ERROR_PRINT2(fileName, "is Not Found");
        exit(-1);
    }
    
    // write
    int rows = l_proRes->rows, cols = l_proRes->cols;
    ofs.write((char*)&rows, sizeof(int));
    ofs.write((char*)&cols, sizeof(int));
    for (int y = 0; y < rows; ++ y) {
        const Vec3b* p_proRes = l_proRes->ptr<Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            for (int ch = 0; ch < 3; ++ ch) {
                ofs.write((char*)&p_proRes[x][ch], sizeof(uchar));
            }
        }
    }
    
    return true;
}

///////////////////////////////  load method ///////////////////////////////
bool ProCam::loadAccessMapCam2Prj(void){
    cout << "loading look up table..." << endl;
    // init
    ifstream ifs(LOOK_UP_TABLE_FILE_NAME);
    if (!ifs) {
        cerr << LOOK_UP_TABLE_FILE_NAME << "is not found" << endl;
        exit(-1);
    }
    
    // load size
    Size mapSize(0, 0);
    ifs.read((char*)&mapSize.height, sizeof(int));
    ifs.read((char*)&mapSize.width, sizeof(int));
//    Size* cameraSize = getCameraSize();
//    if (mapSize != *cameraSize) {
//        cerr << "loaded size is different from current camera size" << endl;
//        ERROR_PRINT2(*cameraSize, mapSize);
//        return false;
//    }
    
    // init access map
    Mat_<Vec2i> l_accessMapC2P = Mat::zeros(mapSize, CV_16SC2);
    
    // load
    int rows = l_accessMapC2P.rows, cols = l_accessMapC2P.cols;
    Point tmp(0, 0);
    for (int y = 0; y < rows; ++ y) {
        Vec2i* l_pAccessMapC2P = l_accessMapC2P.ptr<Vec2i>(y);
        
        for (int x = 0; x < cols; ++ x) {
            ifs.read((char*)&tmp.x, sizeof(int));
            ifs.read((char*)&tmp.y, sizeof(int));
            l_pAccessMapC2P[x] = Vec2i(tmp);
        }
    }
    
    // check
//    const Mat_<Vec2i>* l_accessMapC2P_correct = getAccessMapCam2Prj();
//    if (l_accessMapC2P.size != l_accessMapC2P_correct->size) {
//        _print3("Size of loaded access map is different from correct this", l_accessMapC2P.size, l_accessMapC2P_correct->size);
//        exit(-1);
//    }
//    for (int y = 0; y < rows; ++ y) {
//        Vec2i* l_pAccessMapC2P = l_accessMapC2P.ptr<Vec2i>(y);
//        const Vec2i* l_pAccessMapC2P_correct = l_accessMapC2P_correct->ptr<Vec2i>(y);
//        
//        for (int x = 0; x < cols; ++ x) {
//            if (l_pAccessMapC2P[x] != l_pAccessMapC2P_correct[x]) {
//                _print3("loaded access map is failed", l_pAccessMapC2P[x], l_pAccessMapC2P_correct[x]);
//                exit(-1);
//            }
//        }
//    }
    
    // setting look up table
    setAccessMapCam2Prj(l_accessMapC2P);
    
    cout << "finish loading!" << endl;
    return true;
}

// load response function of projector
bool ProCam::loadProjectorResponseForByte(const char* fileName){
    // init
    const Mat_<Vec3b>* const l_proRes_answer = &m_projectorResponse;
    ifstream ifs(fileName);
    if (!ifs) {
        ERROR_PRINT2(fileName, "is Not Found");
        exit(-1);
    }
    
    // get size
    int rows = 0, cols = 0;
    ifs.read((char*)&rows, sizeof(int));
    ifs.read((char*)&cols, sizeof(int));
    if (l_proRes_answer->rows != rows || l_proRes_answer->cols != cols) {
        ERROR_PRINT3("different size", rows, cols);
        exit(-1);
    }
    Mat_<Vec3b> l_proRes(rows, cols, CV_8UC3);
    
    // get number
    for (int y = 0; y < rows; ++ y) {
        Vec3b* p_proRes = l_proRes.ptr<Vec3b>(y);
        const Vec3b* p_proRes_answer = l_proRes_answer->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int ch = 0; ch < 3; ++ ch) {
                ifs.read((char*)&p_proRes[x][ch], sizeof(uchar));
                if (p_proRes[x][ch] != p_proRes_answer[x][ch]) {
                    ERROR_PRINT3("diff number", p_proRes[x][ch], p_proRes_answer[x][ch]);
                    exit(-1);
                }
            }
        }
    }
    
    return true;
}

bool ProCam::loadProjectorResponseP2IForByte(const char* fileName){
    // init
    ifstream ifs(fileName);
    if (!ifs) {
        ERROR_PRINT2(fileName, "is Not Found");
        exit(-1);
    }
    
    // get size
    int rows = 0, cols = 0;
    ifs.read((char*)&rows, sizeof(int));
    ifs.read((char*)&cols, sizeof(int));
    Mat_<Vec3b> l_proRes(rows, cols, CV_8UC3);
    
    // get number
    for (int y = 0; y < rows; ++ y) {
        Vec3b* p_proRes = l_proRes.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int ch = 0; ch < 3; ++ ch) {
                ifs.read((char*)&p_proRes[x][ch], sizeof(uchar));
            }
        }
    }
    
    // setting
    setProjectorResponseP2I(l_proRes);
    return true;
}

///////////////////////////////  calibration method ///////////////////////////////
// 全てのキャリブレーションを行う
bool ProCam::allCalibration(void){
    Mat projectionImage = Mat::zeros(PRJ_SIZE_HEIGHT, PRJ_SIZE_WIDTH, CV_8UC3);
    imshow(WINDOW_NAME, projectionImage);
//    cvMoveWindow(WINDOW_NAME, POSITION_PROJECTION_IMAGE_X, POSITION_PROJECTION_IMAGE_Y);    // mac
    cv::waitKey(1);
    // geometri calibration
//    if ( !geometricCalibration() ) {
//        cerr << "geometric calibration error" << endl;
//        return false;
//    }
    loadAccessMapCam2Prj();

    
    // linearized projector
    if ( !linearlizeOfProjector() ) {
        cerr << "linearized error of projector" << endl;
        exit(-1);
    }

    // color caliration
    if ( !colorCalibration() ) {
        cerr << "color caliration error" << endl;
        exit(-1);
    }
    
    return true;
}

// 幾何キャリブレーションを行う
bool ProCam::geometricCalibration(void){
    // init
    Size* camSize = getCameraSize();
    Mat_<Vec2i> l_accessMapCam2Prj(*camSize);
    
    // geometric calibration
    VideoCapture* video = getVideoCapture();
    GeometricCalibration gc(this);
    if (!gc.doCalibration(&l_accessMapCam2Prj, video)) {
        cerr << "error of geometric calibration" << endl;
        return false;
    }
    
    // 上で得たプロカム間のルックアップテーブルをクラス変数に代入
    // set class variable
    _print_mat_propaty(l_accessMapCam2Prj);
    setAccessMapCam2Prj(l_accessMapCam2Prj);
    
    // save
    saveAccessMapCam2Prj();
    
    // test geometric calibration
    gc.test_accessMap();
    
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
    // init
    int rows = m_projectorResponse.rows, cols = m_projectorResponse.cols;
    Mat_<Vec3b> prjResponse(rows, cols);
    Mat_<Vec3b> l_prjResponseP2I(rows, cols);
    initProjectorResponseP2I(&prjResponse);
    initProjectorResponseP2I(&l_prjResponseP2I);
    
    // get projector response
    LinearizerOfProjector linearPrj(this);
    if ( !linearPrj.linearlize(&prjResponse, &l_prjResponseP2I) ) return false;
    
    // show
    showProjectorResponseP2I();
    
    // print
    printProjectorResponseP2I(Point(cols*0.6/256, rows*0.6));
    
    // test
    cout << "do radiometric compensation" << endl;
    linearPrj.doRadiometricCompensation(100);
    cout << "did radiometric compensation" << endl;
    return true;
}

///////////////////////////////  convert method ///////////////////////////////

// 幾何変換テーブルを用いて，カメラ座標系からプロジェクタ座標系に変換する
// output / _psImg  : プロジェクタ座標系に投影した画像
// input / _csImg   : カメラ座標系の画像
bool ProCam::convertProjectorCoordinateSystemToCameraOne(cv::Mat* const _psImg, const cv::Mat&  _csImg){
    // error processing
    const Mat_<Vec2i>* l_accessMapC2P = getAccessMapCam2Prj();              // カメラ空間からプロジェクタ空間への変換テーブル
    if ( !isEqualSize(*l_accessMapC2P, _csImg) ) {
        cerr << "size is different" << endl;
        _print_mat_propaty(_csImg);
        _print_mat_propaty(*l_accessMapC2P);
        exit(-1);
    }
    const Size* l_prjSize = getProjectorSize();
    const Size l_psImgSize(_psImg->cols, _psImg->rows);
    if (*l_prjSize != l_psImgSize) {
        _print_mat_propaty(*_psImg);
        ERROR_PRINT2(*l_prjSize, l_psImgSize);
        exit(-1);
    }
    
    // camera coordinate system -> projector coordinate system
    Mat_<Vec3b> l_psImg = Mat::zeros(_psImg->rows, _psImg->cols, CV_8UC3);  // プロジェクタ空間上の画像（ローカル）
    int rows = _csImg.rows, cols = _csImg.cols;
    if (isContinuous(*l_accessMapC2P, _csImg) ) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        const Vec2i* l_pAccessMapC2P = l_accessMapC2P->ptr<Vec2i>(y);
        const Vec3b* l_pCsImg = _csImg.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            Point prjPoint = Point(l_pAccessMapC2P[x]); // projector coordinate system
            l_psImg.at<Vec3b>(prjPoint) = l_pCsImg[x];  // set projector image
            //            Point camPoint(x, y);
            //            _print2(camPoint, prjPoint);
        }
    }
    
    // deep copy
    *_psImg = l_psImg.clone();
    
    return true;
}


// プロジェクタの強度が線形化されていない画像から線形化された画像へ変換
// output / _linearImg      : 線形化後の画像
// input / _nonLinearImg    : 線形化前の画像
bool ProCam::convertNonLinearImageToLinearOne(cv::Mat* const _linearImg, const cv::Mat&  _nonLinearImg){
    // error processing
    const Mat_<Vec3b>* l_prjRes = getProjectorResponse();               // プロジェクタ強度の線形化ルックアップテーブル
    const Mat l_compressedPrjRes(l_prjRes->rows, l_prjRes->cols / 256, CV_8UC3);
    if (!isEqualSize(*_linearImg, _nonLinearImg, l_compressedPrjRes)) {
        cerr << "different size" << endl;
        _print_name(*_linearImg);
        printMatPropaty(*_linearImg);
        _print_name(_nonLinearImg);
        printMatPropaty(_nonLinearImg);
        _print_name(l_compressedPrjRes);
        printMatPropaty(l_compressedPrjRes);
        exit(-1);
    }
    
    // init lineared image
    int rows = _nonLinearImg.rows, cols = _nonLinearImg.cols, imgCh = _nonLinearImg.channels(); // 投影サイズ
    Mat l_linearImage(rows, cols, CV_8UC3, Scalar(0, 0, 0));    // プロジェクタ強度の線形化を行った後の投影像
    
    // scan all pixel and channel
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        const Vec3b* l_pPrjRes = l_prjRes->ptr<Vec3b>(y);
        const Vec3b* l_pNonLinearImg = _nonLinearImg.ptr<Vec3b>(y);
        Vec3b* l_pLinearImg = _linearImg->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int ch = 0; ch < imgCh; ++ ch) {
                const int prjResIndex = x * 256 + l_pNonLinearImg[x][ch];   // 応答特性マップの座標
                l_pLinearImg[x][ch] = l_pPrjRes[prjResIndex][ch];
            }
        }
    }
    
    // deep copy
    *_linearImg = l_linearImage.clone();
    
    return true;
}

// PからIに変換する
bool ProCam::convertPtoI(cv::Mat* const _I, const cv::Mat&  _P){
    // error processing
    if (!isEqualSizeAndType(*_I, _P)) {
        cerr << "different size or type" << endl;
        _print_mat_propaty(*_I);
        _print_mat_propaty(_P);
        exit(-1);
    }
    
    // scanning all pixel
    const Mat* l_transP2I = getProjectorResponseP2I();
    const int rows = _I->rows, cols = _I->cols, ch = _I->channels();
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pI = _I->ptr<Vec3b>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3b* l_pTransP2I = l_transP2I->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < ch; ++ c) {
                // convert
                const int index = x * 256 + (int)l_pP[x][c];
                l_pI[x][c] = l_pTransP2I[index][c];
            }
        }
    }
    
    return true;
}

///////////////////////////////  other method ///////////////////////////////

// output / captureImage    : 撮影した画像を代入する場所
// input / projectionImage  : 投影する画像
// return                   : 成功したかどうか
bool ProCam::captureFromLight(cv::Mat* const captureImage, const cv::Mat& projectionImage){
    // 投影
    imshow(WINDOW_NAME, projectionImage);
    cvMoveWindow(WINDOW_NAME, POSITION_PROJECTION_IMAGE_X, POSITION_PROJECTION_IMAGE_Y);
    cv::waitKey(SLEEP_TIME / 2);
    
    // N回撮影する
    cv::Mat image;
//    Mat_<Vec3b>* aImage = new Mat_<Vec3b>[CAPTURE_NUM];
//    Size* camSize = getCameraSize();
//    Mat_<Vec3d> sumImage = Mat::zeros(*camSize, CV_64FC3), tmp;
    for (int i = 0; i < CAPTURE_NUM; ++ i) {
//        aImage[i] = Mat::zeros(*camSize, CV_8UC3);
        getCaptureImage(&image);
//        getCaptureImage(&aImage[i]);
        
        // uchar -> double
//        aImage[i].convertTo(tmp, CV_64FC3);

        // add
//        sumImage += tmp;
    }
//    sumImage /= CAPTURE_NUM;
    
    *captureImage = image.clone();
//    *captureImage = sumImage.clone();
    cv::waitKey(SLEEP_TIME / 2);

    return true;
}

// 線形化したプロジェクタを用いて投影・撮影を行う
bool ProCam::captureFromLinearLight(cv::Mat* const captureImage, const cv::Mat& projectionImage){
    // init lineared projection image
    Mat l_linearProjectionImage(projectionImage.rows, projectionImage.cols, CV_8UC3, Scalar(0, 0, 0));    // プロジェクタ強度の線形化を行った後の投影像
    
    // non linear image -> linear one
    convertNonLinearImageToLinearOne(&l_linearProjectionImage, projectionImage);
    
    return captureFromLight(captureImage, l_linearProjectionImage);
}

// response mapの表示
bool ProCam::showProjectorResponseP2I(void){
    const Mat_<Vec3b>* l_responseMap = getProjectorResponseP2I();
    Mat l_responseImage(l_responseMap->rows, l_responseMap->cols / 256, CV_8UC3, Scalar(0, 0, 0));
    
    for (int i = 0; i < 256; ++ i) {
        _print(i);
        getProjectorResponseP2I(&l_responseImage, i);
        Mat l_flatImage(l_responseImage.rows, l_responseImage.cols, CV_8UC3, Scalar(i, i, i));

        MY_IMSHOW(l_flatImage);
        MY_IMSHOW(l_responseImage);
        waitKey(1);
    }
    
    return true;
}

// projector responseの出力
bool ProCam::printProjectorResponseP2I(const cv::Point& _pt){
    const Mat_<Vec3b>* l_responseMap = getProjectorResponseP2I();
    Mat l_responseImage(l_responseMap->rows, l_responseMap->cols / 256, CV_8UC3, Scalar(0, 0, 0));
    
    for (int i = 0; i < 256; ++ i) {
        getProjectorResponseP2I(&l_responseImage, i);
        
        Vec3i tmp = (Vec3i)l_responseImage.at<Vec3b>(_pt);
        _print_gnuplot4(std::cout, i, tmp[2], tmp[1], tmp[0]);
    }
    
    return true;
}

// projector responseの補間を行う
bool ProCam::interpolationProjectorResponseP2I(cv::Mat* const _prjRes){
    // initialize
    int rows = _prjRes->rows, cols = _prjRes->cols / 256, channels = _prjRes->channels();
    if (_prjRes->isContinuous()) {
        cols *= rows;
        rows = 1;
    }
    
    // scan all pixel and color value
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pPrjRes = _prjRes->ptr<Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            
            // scan all luminance
            for (int p = 1; p < 255; ++ p) {    // 最初と最後は初期化の時に決定済みである為，除外
                // scan all color
                for (int c = 0; c < channels; ++ c) {
                    if (l_pPrjRes[x * 256 + p][c] == INIT_RES_NUM) {   // 抜けている箇所の特定
                        uchar p0 = p - 1, i0 = l_pPrjRes[x * 256 + p0][c];  // 下端の値

                        // 上端の検索
                        uchar p1 = p + 1, i1 = 0;
                        for (; ; ++ p1) {
                            if (l_pPrjRes[x * 256 + p1][c] != INIT_RES_NUM) {
                                i1 = l_pPrjRes[x * 256 + p1][c];
                                break;
                            }
                        }
                        
                        // set alpha
                        double alpha = (double)(p - p0)/(double)(p1 - p0);
                        
                        // 抜けている箇所の修復
                        l_pPrjRes[x * 256 + p][c] = i0 + (uchar)(alpha * (double)(i1 - i0));
                    }
                }
            }
        }
    }
    
    return true;
}

// カメラが線形であるかを確かめる
bool ProCam::checkCameraLinearity(void){
    Mat image;
    getCaptureImage(&image);
    
    Vec3b* p_image = image.ptr<Vec3b>(image.rows * 0.5);
    for (int x = 0; x < image.cols; ++ x) {
        cout << p_image[x] << endl;
    }
    return true;
}
