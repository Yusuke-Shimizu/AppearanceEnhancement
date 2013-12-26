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
{
    init();
}

ProCam::ProCam(const cv::Size& projectorSize)
{
    init(projectorSize);
}

ProCam::ProCam(const int _width, const int _height)
{
    init(_width, _height);
}

ProCam::ProCam(const int _size)
{
    init(_size);
}

///////////////////////////////  destructor ///////////////////////////////
// デストラクタ
ProCam::~ProCam(void){
    cout << "deleting ProCam (" << this <<")" << endl;
    destroyAllWindows();    // ウィンドウの全削除]
#ifdef LIB_DC1394_FLAG
    DCam_stop_capture(m_dcam);
    DCam_delete(&m_dcam);
#endif
    cout << "ProCam is deleted (" << this <<")" << endl;
}

///////////////////////////////  init method ///////////////////////////////

// ProCamクラスの初期化
bool ProCam::init(const cv::Size& projectorSize){
    // カメラの初期化
    if ( !initCamera() ) return false;
    
    // プロジェクタの初期化
    if ( !initProjectorSize(projectorSize) ) return false;
    if ( !initProjectorResponseI2P() ) return false;
    if ( !initProjectorResponseP2I() ) return false;
    
    // アクセスマップの初期化
    if ( !initAccessMapCam2Prj() ) return false;
    
    // Vの初期化
    initV();
    initF();
    
    return true;
}
bool ProCam::init(const int _width, const int _height){
    Size projectorSize(_width, _height);
    return init(projectorSize);
}
bool ProCam::init(const int _size) { return init(_size, _size); }
bool ProCam::init(void) { return init(100); }

// カメラの初期化
bool ProCam::initCamera(void){
#ifdef LIB_DC1394_FLAG
    if ( !initDCam() ) return false;
    if ( !initCameraSizeOfDCam1394() ) return false;
#else
    if ( !initVideoCapture() ) return false;
    if ( !initCameraSize() ) return false;
#endif
    return true;
}

// m_dcamの初期化
bool ProCam::initDCam(void){
    const CameraConfig config = config_dragonfly_vga_color;
    const int cameraNumber = 0;
    
    // DCamの設定
    m_dcam = DCam_new(config, cameraNumber);
    DCam_start_capture(m_dcam);

    return true;
}

// m_cameraSizeの初期化
// return   : 成功したかどうか
bool ProCam::initCameraSize(void){
    // videoCaptureからサイズを取得する
//    VideoCapture* l_video = getVideoCapture();
//    cv::Size videoSize(l_video->get(CV_CAP_PROP_FRAME_WIDTH), l_video->get(CV_CAP_PROP_FRAME_HEIGHT));
    Mat image;
    getCaptureImage(&image);
    const Size camSize(image.cols, image.rows);

    // setting
    if( !setCameraSize(camSize) ) return false;
    cout << "camera size is " << camSize << endl;
    
    return true;
}
bool ProCam::initCameraSizeOfDCam1394(void){
    // カメラサイズの取得
    DCam l_dcam = getDCam();
    unsigned int l_width = 0, l_height = 0;
    dc1394_get_image_size_from_video_mode(l_dcam->camera, DC1394_VIDEO_MODE_640x480_RGB8, &l_width, &l_height);
    const Size camSize(l_width, l_height);
    
    // setting
    if( !setCameraSize(camSize) ) return false;
    cout << "camera size is " << camSize << endl;
    return true;
}

//
bool ProCam::initProjectorSize(const cv::Size& projectorSize){
    cout << "projector size is " << projectorSize << endl;
    return setProjectorSize(projectorSize);
}

// ビデオキャプチャーの初期化
// return   : 成功したかどうか
bool ProCam::initVideoCapture(void){
//    m_video = cv::VideoCapture(0);
    m_video.open(0);
    if( !m_video.isOpened() ){
        std::cerr << "ERROR : camera is not opened !!" << std::endl;
        exit(-1);
    }
    return true;
}

// アクセスマップの初期化
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
bool ProCam::initProjectorResponseI2P(void){
    // init
    const cv::Size* prjSize = getProjectorSize();
    const cv::Size prjResSize(prjSize->width * 256, prjSize->height);
    
    // set projector response
    m_projectorResponseI2P = cv::Mat_<cv::Vec3b>(prjResSize);
    initProjectorResponse(&m_projectorResponseI2P);
    return true;
}

bool ProCam::initProjectorResponseP2I(void){
    // init
    const cv::Size* prjSize = getProjectorSize();
    const cv::Size prjResSize(prjSize->width * 256, prjSize->height);
    
    // set projector response
    m_projectorResponseP2I = cv::Mat_<cv::Vec3b>(prjResSize);
    initProjectorResponse(&m_projectorResponseP2I);
    return true;
}

bool ProCam::initProjectorResponse(cv::Mat* const _prjRes){
    int rows = _prjRes->rows, cols = _prjRes->cols;
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pPrjResP2I = _prjRes->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols / 256; ++ x) {
            for (int i = 0; i < 256; ++ i) {
                l_pPrjResP2I[x * 256 + i] = Vec3b(INIT_RES_NUM, INIT_RES_NUM, INIT_RES_NUM);
            }
            l_pPrjResP2I[x * 256 + 0] = CV_VEC3B_BLACK;
            l_pPrjResP2I[x * 256 + 255] = CV_VEC3B_WHITE;
        }
    }
    return true;
}

// Vの初期化
bool ProCam::initV(void){
    // init
    const Vec9d l_initV(0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0);
    const Size *l_camSize = getCameraSize();
    
    // initialize
    m_V = Mat_<Vec9d>(*l_camSize);
    const int rows = l_camSize->height, cols = l_camSize->width;
    for (int y = 0; y < rows; ++ y) {
        Vec9d* p_cmmm = m_V.ptr<Vec9d>(y);
        for (int x = 0; x < cols; ++ x) {
            p_cmmm[x] = l_initV;
        }
    }
    return true;
}

void ProCam::initF(void){
    const Size* camSize = getCameraSize();
    m_F = Mat(*camSize, CV_8UC3, Scalar(0, 0, 0));
}

///////////////////////////////  set method ///////////////////////////////
// m_cameraSizeの設定
bool ProCam::setCameraSize(const cv::Size& cameraSize){
    m_cameraSize = cameraSize;
    return true;
}

// m_projectorSizeの設定
bool ProCam::setProjectorSize(const cv::Size& projectorSize){
    m_projectorSize = projectorSize;
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
    const Mat* l_prjResI2P = getProjectorResponseI2P();
    if ( !isEqualSizeAndType(_response, *l_prjResI2P)) {
        ERROR_PRINT("_response Size or Type is different from m_projectorResponse");
        _print_mat_propaty(_response);
        _print_mat_propaty(*l_prjResI2P);
        return false;
    }
    
    // deep copy
    m_projectorResponseI2P = _response.clone();
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
bool ProCam::setProjectorResponseP2IAtOutOfCameraArea(void){
    return interpolateProjectorResponseP2IAtOutOfCameraArea(&m_projectorResponseP2I);
}

// 画像をプロジェクタ応答マップに挿入
bool ProCam::setImageProjectorResponseP2I(cv::Mat* const _responseMap, const cv::Mat& _responseImage, const int _index){
    const int rows = _responseImage.rows, cols = _responseImage.cols;
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3b* l_pPrjResP2I = _responseMap->ptr<Vec3b>(y);
        const Vec3b* l_pResImg = _responseImage.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            l_pPrjResP2I[x * 256 + _index] = l_pResImg[x];
        }
    }

    return true;
}
bool ProCam::setImageProjectorResponseP2I(const cv::Mat& _responseImage, const int _index){
    setImageProjectorResponseP2I(&m_projectorResponseP2I, _responseImage, _index);
    return true;
}

// Vのセッティング
bool ProCam::setV(const cv::Mat& _diffBB, const cv::Mat& _diffGB, const cv::Mat& _diffRB){
    // error handle
    if (!isEqualSize(_diffBB, _diffGB, _diffRB, m_V)) {
        cerr << "size is different" << endl;
        _print_mat_propaty(_diffBB);
        _print_mat_propaty(_diffGB);
        _print_mat_propaty(_diffRB);
        _print_mat_propaty(m_V);
        exit(-1);
    }
    
    // setting
    int rows = _diffBB.rows, cols = _diffBB.cols;
    if (isContinuous(_diffBB, _diffGB, _diffRB, m_V)) {
        cols *= rows;
        rows = 1;
    }
    
    // scanning all pixel
    for (int y = 0; y < rows; ++ y) {
        const Vec3d* l_pDiffBB = _diffBB.ptr<Vec3d>(y);
        const Vec3d* l_pDiffGB = _diffGB.ptr<Vec3d>(y);
        const Vec3d* l_pDiffRB = _diffRB.ptr<Vec3d>(y);
        Vec9d* l_pV = m_V.ptr<Vec9d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int i = 0; i < 3; ++ i) {
                l_pV[x][i * 3 + 0] = l_pDiffBB[x][i];
                l_pV[x][i * 3 + 1] = l_pDiffGB[x][i];
                l_pV[x][i * 3 + 2] = l_pDiffRB[x][i];
            }
        }
    }
    
    return true;
}
bool ProCam::setV(const std::vector<cv::Mat>& _vecC, const std::vector<cv::Vec3d>& _vecP){
    
    // create P mat
    std::vector<cv::Vec3d> l_vecPMat;
    for (vector<Vec3d>::const_iterator l_vecPItr = _vecP.begin();
         l_vecPItr != _vecP.end();
         ++ l_vecPItr) {
        l_vecPMat.push_back(*l_vecPItr);
        _print(*l_vecPItr);
    }
    Mat l_matP(3, (int)l_vecPMat.size(), CV_64FC1, 0.0);
    convertVector2Mat(&l_matP, l_vecPMat);
//    cv::merge(l_vecPMat, l_matP);
    _print(l_matP);

    //
    int rows = _vecC[0].rows, cols = _vecC[0].cols;
//    Mat_<Vec9d> l_V(rows, cols, Vec9d(0,0,0,0,0,0,0,0,0));
    for (int y = 0; y < rows; ++ y) {
        Vec9d* l_pV = m_V.ptr<Vec9d>(y);
//        vector<const Vec3d* const> l_pVecC;
//        for (vector<Mat>::const_iterator l_itrC = _vecC.begin();
//             l_itrC != _vecC.end();
//             ++ l_itrC) {
//            l_pVecC.push_back(l_itrC->ptr<Vec3b>(y));
//        }
        
        for (int x = 0; x < cols; ++ x) {
            // create C mat
            vector<Vec3d> l_vecMatC;
            for (vector<Mat>::const_iterator l_itrVecC = _vecC.begin();
                 l_itrVecC != _vecC.end();
                 ++ l_itrVecC) {
                l_vecMatC.push_back((l_itrVecC->at<Vec3d>(y, x)));
            }
            Mat l_matC(3, (int)l_vecMatC.size(), CV_64FC1, 0.0);
//            cv::merge(l_vecMatC, l_matC);
            convertVector2Mat(&l_matC, l_vecMatC);
            
            // calc
            const Mat l_matV = l_matC * l_matP.inv();
            
            // Mat -> Vec9d
            convertMatToVec(&l_pV[x], l_matV);
        }
    }
    return true;
}

// m_Fの設定
// F = C - VP
void ProCam::setF(const cv::Mat& _P){
    const Mat_<Vec9d>* l_V = getV();
    const Size* l_camSize = getCameraSize();
    Mat l_C(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Mat l_VP(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    
    // error handle
    if (!isEqualSize(_P, l_C, *l_V) ) {
        cerr << "different size" << endl;
        _print_mat_propaty(_P);
        _print_mat_propaty(l_C);
        _print_mat_propaty(*l_V);
        exit(-1);
    }
    
    // get C from P
    captureFromLinearLightOnProjectorDomain(&l_C, _P);
    
    // get F
    int rows = _P.rows, cols = _P.cols;
    if (isContinuous(_P, l_C, *l_V, m_F, l_VP)) {
        cols *= rows;
        rows = 1;
    }
    Mat l_matP(3, 1, CV_64FC1), l_matC(3, 1, CV_64FC1), l_matV(3, 3, CV_64FC1), l_matF(3, 1, CV_64FC1);
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3b* l_pC = l_C.ptr<Vec3b>(y);
        const Vec9d* l_pV = l_V->ptr<Vec9d>(y);
        Vec3b* l_pF = m_F.ptr<Vec3b>(y);
        Vec3b* l_pVP = l_VP.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // Vec3b(d) -> Mat_<double>
            l_matP = Mat(Vec3d(l_pP[x]));
            l_matC = Mat(Vec3d(l_pC[x]));
            convertVecToMat(&l_matV, l_pV[x]);
            
            // calculation (F = C - VP)
            Mat l_VP = l_matV * l_matP;
            l_matF = l_matC - l_VP;
            
            // Mat -> Vec3b
            l_pF[x] = Vec3b(Vec3d(l_matF));
            l_pVP[x] = Vec3b(l_VP);
        }
    }
    MY_IMSHOW(l_VP);
    waitKey(30);
}

///////////////////////////////  get method ///////////////////////////////
// m_dcamの取得
DCam ProCam::getDCam(void){
    return m_dcam;
}

// m_cameraSizeの取得
//bool ProCam::getCameraSize(cv::Size* const cameraSize){
//    *cameraSize = m_cameraSize;
//    return true;
//}
cv::Size* ProCam::getCameraSize(void){
    return &m_cameraSize;
}
const cv::Size& ProCam::getCameraSize_(void){
    return m_cameraSize;
}

// カメラのピクセル数を返す
// return   : カメラの全ピクセル数
int ProCam::getPixelsOfCamera(void){
    const Size* cameraSize = getCameraSize();
    return cameraSize->area();
}

// m_projectorSizeの取得
cv::Size* ProCam::getProjectorSize(void){
    return &m_projectorSize;
}
const cv::Size& ProCam::getProjectorSize_(void){
    return m_projectorSize;
}

// m_videoから撮影画像を取得
// output/image : 取得した画像を入れる行列
// return       : 成功したかどうか
bool ProCam::getCaptureImage(cv::Mat* const _image){
#ifdef LIB_DC1394_FLAG  // libdc1394を用いる場合
    DCam l_dcam = getDCam();
    captureCVImage(_image, l_dcam);
#else
    VideoCapture* l_video = getVideoCapture();
#ifdef BAYER_FLAG       // カメラ入力がbayer配列の場合
    for (int i = 0; i < BUFFER_NUM; ++ i) {
        Mat l_bayerImage;
        *l_video >> l_bayerImage;
        cv::cvtColor(l_bayerImage, *_image, CV_BayerBG2RGB);
    }
#else                   // OpenCVで処理出来る場合
    for (int i = 0; i < BUFFER_NUM; ++ i) {
        *l_video >> *_image;
    }
#endif
#endif
    return true;
}

// m_videoの取得
// output / _video
cv::VideoCapture* ProCam::getVideoCapture(void){
    return &m_video;
}

// m_accessMapCam2Proの取得
const cv::Mat_<cv::Vec2i>* ProCam::getAccessMapCam2Prj(void){
    return &m_accessMapCam2Prj;
}

// カメラ座標値からプロジェクタ座標値を取得
void ProCam::getPointOnPrjDomainFromPointOnCamDomain(cv::Point* const _prjPoint, const cv::Point& _camPoint){
    const Mat_<Vec2i>* l_accessMapCam2Prj = getAccessMapCam2Prj();
    *_prjPoint = Point(l_accessMapCam2Prj->at<Vec2i>(_camPoint));
}

const cv::Mat* ProCam::getProjectorResponseI2P(void){
    return &m_projectorResponseI2P;
}
const cv::Mat* ProCam::getProjectorResponseP2I(void){
    return &m_projectorResponseP2I;
}

// 引数で与えられた応答特性マップのある輝度の画像を取得
void ProCam::getImageProjectorResponseP2I(cv::Mat* const _responseImage, const cv::Mat& _responseMap, const int _index){
    // error processing
    if (_responseImage->rows != _responseMap.rows || _responseImage->cols != _responseMap.cols / 256) {
        cerr << "different size" << endl;
        _print_mat_propaty(*_responseImage);
        _print_mat_propaty(_responseMap);
        exit(-1);
    }
    const int rows = _responseImage->rows, cols = _responseImage->cols;
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        const Vec3b* l_pPrjResP2I = _responseMap.ptr<Vec3b>(y);
        Vec3b* l_pResImg = _responseImage->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            l_pResImg[x] = l_pPrjResP2I[x * 256 + _index];
        }
    }
}

// 応答特性マップのある輝度の時の画像を取得する
void ProCam::getImageProjectorResponseP2I(cv::Mat* const _responseImage, const int _index){
    const Mat* l_prjResP2I = getProjectorResponseP2I();
    getImageProjectorResponseP2I(_responseImage, *l_prjResP2I, _index);
}

// Vの取得
const cv::Mat_<Vec9d>* ProCam::getV(void){
    return &m_V;
}

const cv::Mat* ProCam::getF(void){
    return &m_F;
}

// P = V^{-1} * (A^{-1} * C - F)
void ProCam::getNextProjectionImage(cv::Mat* const _P, const cv::Mat& _C){
    const Mat *l_V = getV(), *l_F = getF();
    // error handle
    if (!isEqualSizeAndType(*_P, _C, *l_F)) {
        cerr << "different size or type" << endl;
        _print_mat_propaty(*_P);
        _print_mat_propaty(_C);
        _print_mat_propaty(*l_F);
        exit(-1);
    } else if (!isEqualSize(*l_F, *l_V)) {
        cerr << "different size" << endl;
        _print_mat_propaty(*l_F);
        _print_mat_propaty(*l_V);
        exit(-1);
    }
    
    //
    int rows = _P->rows, cols = _P->cols;
    if (isContinuous(*_P, _C, *l_F, *l_V)) {
        cols *= rows;
        rows = 1;
    }
    Mat l_matP(3, 1, CV_64FC1), l_matC(3, 1, CV_64FC1), l_matF(3, 1, CV_64FC1), l_matV(3, 3, CV_64FC1);
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3b* l_pP = _P->ptr<Vec3b>(y);
        const Vec3b* l_pC = _C.ptr<Vec3b>(y);
        const Vec3b* l_pF = l_F->ptr<Vec3b>(y);
        const Vec9d* l_pV = l_V->ptr<Vec9d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // Vec -> Mat
            Vec3d tmp = Vec3d(l_pC[x]);
            l_matC = Mat(tmp);
            tmp = Vec3d(l_pF[x]);
            l_matF = Mat(tmp);
            convertVecToMat(&l_matV, l_pV[x]);
            
            // calculation (P = V^{-1} * (A^{-1} * C - F))
            l_matP = l_matV.inv() * (l_matC - l_matF);
            
            // Mat -> Vec
            tmp = Vec3d(l_matP);
            l_pP[x] = Vec3b(tmp);
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

// 
bool ProCam::saveProjectorResponseForByte(const char* fileName){
    // init
    const Mat* const l_proRes = getProjectorResponseI2P();
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
    const Mat* const l_proRes = getProjectorResponseP2I();
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

// printProjectorResponseの内容を保存
bool ProCam::savePrintProjectorResponse(const char* fileName, const cv::Point& _pt, const cv::Mat& _prjRes){
    Mat l_responseImage(_prjRes.rows, _prjRes.cols / 256, CV_8UC3, Scalar(0, 0, 0));
    ofstream ofs(fileName);
    if (!ofs) {
        cout << fileName << "is not found" << endl;
        exit(-1);
    }
    
    for (int i = 0; i < 256; ++ i) {
        getImageProjectorResponseP2I(&l_responseImage, _prjRes, i);
        
        Vec3i tmp = (Vec3i)l_responseImage.at<Vec3b>(_pt);
        _print_gnuplot4(ofs, i, tmp[2], tmp[1], tmp[0]);
    }
    
    return true;
}
bool ProCam::savePrintProjectorResponseI2P(const char* fileName, const cv::Point& _pt){
    const Mat* l_responseMap = getProjectorResponseI2P();

    return savePrintProjectorResponse(fileName, _pt, *l_responseMap);
}
bool ProCam::savePrintProjectorResponseP2I(const char* fileName, const cv::Point& _pt){
    const Mat* l_responseMap = getProjectorResponseP2I();
    
    return savePrintProjectorResponse(fileName, _pt, *l_responseMap);
}

// PとIの関係を出力
bool ProCam::saveRelationP2I(void){
    const Size l_cameraSize = getCameraSize_();
    Mat l_P(l_cameraSize, CV_8UC3, CV_SCALAR_BLACK), l_I(l_cameraSize, CV_8UC3, CV_SCALAR_BLACK);
    Scalar l_mean(0,0,0,0), l_stddev(0,0,0,0);
    ofstream ofs(P2I_DATA_PATH.c_str());
    
    for (int i = 0; i < 256; ++ i) {
        l_P = cv::Scalar(i, i, i, 0);
        convertPtoI(&l_I, l_P);
        meanStdDev(l_I, l_mean, l_stddev);
        
        // output
        std::cout << i << "\t";
        _print_gnuplot2(std::cout, l_mean, l_stddev);
        ofs << i << "\t";
        _print_gnuplot_color2_l(ofs, l_mean, l_stddev);
    }
    return true;
}

bool ProCam::saveRelationI2C(void){
    const Size* l_camSize = getCameraSize();
    Mat l_captureImage(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    
    for (int i = 1; i < 8; ++ i) {
        if (i == 3 || i >= 5) {
            continue;
        }
        const int l_col1 = i % 2;
        const int l_col2 = (i / 2) % 2;
        const int l_col3 = (i / 4) % 2;
        Vec3b l_mask(l_col3, l_col2, l_col1);
        ostringstream oss;
        oss <<I2C_DATA_PATH << l_col3 << l_col2 << l_col1 << ".dat";
        cout << oss.str() << endl;
        ofstream ofs(oss.str().c_str());
        
        for (int prj = 0; prj < 256; ++ prj) {
            // capture
//            Vec3b l_color(prj * l_mask[0], prj * l_mask[1], prj * l_mask[2]);
            Vec3b l_color = prj * l_mask;
            captureFromLightOnProjectorDomain(&l_captureImage, l_color);
            MY_IMSHOW(l_captureImage);
            
            // get mean and standard deviation
            Vec3d l_mean(0, 0, 0), l_stddev(0, 0, 0);
            meanStdDev(l_captureImage, l_mean, l_stddev);
            
            // print
            std::cout << prj << "\t";
            _print_gnuplot2(std::cout, l_mean, l_stddev);
            ofs << prj << "\t";
            _print_gnuplot_color2_l(ofs, l_mean, l_stddev);
        }
        ofs.close();
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
    
    // setting look up table
    setAccessMapCam2Prj(l_accessMapC2P);
    
    cout << "finish loading!" << endl;
    return true;
}

// load response function of projector
bool ProCam::loadProjectorResponseForByte(const char* fileName){
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
    setProjectorResponse(l_proRes);
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
    // geometri calibration
#ifdef GEO_CAL_CALC_FLAG
    while (true) {
        if ( !geometricCalibration() ) {
            cerr << "geometric calibration error" << endl;
            return false;
        }
        
        cout << "Calibration is ok?" << endl;
        if (yes_no()) {
            break;
        }
    }
#else
    // load
    loadAccessMapCam2Prj();
#endif
    
    // linearized projector
    if ( !linearizeOfProjector(CALC_LINEAR_FLAG_AT_INIT, SHOW_LINEAR_FLAG_AT_INIT) ) {
        cerr << "linearized error of projector" << endl;
        exit(-1);
    }

    // color caliration
//    if ( !colorCalibration() ) {
    if ( !colorCalibration3() ) {
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
    
    // calculation
    GeometricCalibration gc(this);
    if (!gc.doCalibration(&l_accessMapCam2Prj)) return false;
    setAccessMapCam2Prj(l_accessMapCam2Prj);
    saveAccessMapCam2Prj();

//#ifdef SHOW_GEOMETRIC_CALIBRATION_MAP_FLAG
    // show geometric map
    showAccessMapCam2Prj();
//#endif
    
    return true;
}

// プロジェクタの線形化を行う
// return   : 成功したかどうか
bool ProCam::linearizeOfProjector(const bool _calcLinearFlag, const bool _showLinearFlag){
    // init
    const Mat* l_prjRes = getProjectorResponseI2P();
    int rows = l_prjRes->rows, cols = l_prjRes->cols;
    Mat_<Vec3b> l_prjResponseI2P(rows, cols);
    Mat_<Vec3b> l_prjResponseP2I(rows, cols);
    initProjectorResponse(&l_prjResponseI2P);
    initProjectorResponse(&l_prjResponseP2I);
    
    // get projector response
    LinearizerOfProjector linearPrj(this);
    if (_calcLinearFlag) {
        // 引数消してもいいかも
        if ( !linearPrj.doLinearlize(&l_prjResponseI2P, &l_prjResponseP2I) ) {
            return false;
        }
    } else {
        linearPrj.loadColorMixingMatrixOfByte(CMM_MAP_FILE_NAME_BYTE);
        loadProjectorResponseP2IForByte(PROJECTOR_RESPONSE_P2I_FILE_NAME_BYTE);
        loadProjectorResponseForByte(PROJECTOR_RESPONSE_I2P_FILE_NAME_BYTE);
        linearPrj.loadAllCImages();
//        linearPrj.test_responseFunction();
//        linearPrj.saveEstimatedC();
    }

    // show and print
    if (_showLinearFlag) {
        // show
        showProjectorResponseP2I();
        showProjectorResponseI2P();
        
        // print
        Point pt(cols*0.6/256, rows*0.6);
        printProjectorResponseP2I(pt);
        printProjectorResponseI2P(pt);
        savePrintProjectorResponseP2I(PROJECTOR_RESPONSE_AT_SOME_POINT_P2I_FILE_NAME, pt);
        savePrintProjectorResponseI2P(PROJECTOR_RESPONSE_AT_SOME_POINT_I2P_FILE_NAME, pt);
        linearPrj.saveAllC(PROJECTOR_ALL_C_IMAGES_FILE_NAME, pt);
    }
    
    return true;
}
bool ProCam::test_linearizeOfProjector(void){
    const Size* l_camSize = getCameraSize();
    Mat l_captureImage(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Mat l_captureImageNotLinear(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    ofstream ofs(CHECK_LINEARIZE_FILE_NAME);
    
    for (int i = 0; i < 256; ++ i) {
        // projection and capturing
        captureFromLinearLightOnProjectorDomain(&l_captureImage, i);
        captureFromLightOnProjectorDomain(&l_captureImageNotLinear, i);
        
        // get mean and standard deviation
        Vec3d l_meanColor(0, 0, 0), l_stddevColor(0, 0, 0), l_meanColorNL(0, 0, 0), l_stddevColorNL(0, 0, 0);
        meanStdDev(l_captureImage, l_meanColor, l_stddevColor);
        meanStdDev(l_captureImageNotLinear, l_meanColorNL, l_stddevColorNL);

        // print
//        std::cout << i << "\t";
//        _print_gnuplot_color4_l(std::cout, l_meanColor, l_stddevColor, l_meanColorNL, l_stddevColorNL);
        ofs << i << "\t";
        _print_gnuplot_color4_l(ofs, l_meanColor, l_stddevColor, l_meanColorNL, l_stddevColorNL);
        
        // if you are mistake, you should push delete button then go back
        if (waitKey(30) == CV_BUTTON_DELETE) {
            return false;
        }
    }
    
    return true;
}

// 光学キャリブレーションを行う
Mat g_F0;
bool ProCam::colorCalibration(const bool _denoisingFlag){
    cout << "color calibration start!" << endl;
    
    // init
    Size *cameraSize = getCameraSize();
    
    // init capture image
    uchar depth8x3 = CV_8UC3;
    cv::Mat black_cap   (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat red_cap     (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat green_cap   (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat blue_cap    (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    
    // capture from some color light
    captureFromLinearLightOnProjectorDomain(&black_cap, CV_VEC3B_BLACK, _denoisingFlag, SLEEP_TIME * 5);
    captureFromLinearLightOnProjectorDomain(&red_cap, CV_VEC3B_RED, _denoisingFlag);
    captureFromLinearLightOnProjectorDomain(&green_cap, CV_VEC3B_GREEN, _denoisingFlag);
    captureFromLinearLightOnProjectorDomain(&blue_cap, CV_VEC3B_BLUE, _denoisingFlag);
    g_F0 = black_cap.clone();
    
    // show image
    imshow("black_cap", black_cap);
    imshow("red_cap", red_cap);
    imshow("green_cap", green_cap);
    imshow("blue_cap", blue_cap);
    waitKey(30);
    
    // translate bit depth (uchar[0-255] -> double[0-1])
    uchar depth64x3 = CV_64FC3;
    double rate = 1.0 / 255.0;
    black_cap.convertTo(black_cap, depth64x3, rate);
    red_cap.convertTo(red_cap, depth64x3, rate);
    green_cap.convertTo(green_cap, depth64x3, rate);
    blue_cap.convertTo(blue_cap, depth64x3, rate);
    
    // calc difference[-1-1]
    Mat diffRedAndBlack = red_cap;
    Mat diffGreenAndBlack = green_cap;
    Mat diffBlueAndBlack = blue_cap;
    if (COLOR_CALIB_2004) {
        diffRedAndBlack -= black_cap;
        diffGreenAndBlack -= black_cap;
        diffBlueAndBlack -= black_cap;
    }
    
    // set V
    setV(diffBlueAndBlack, diffGreenAndBlack, diffRedAndBlack);
    
    cout << "color calibration finish!" << endl;
    return true;
}
bool ProCam::colorCalibration2(cv::Mat_<Vec12d>* const _V){
    cout << "color calibration start!" << endl;
    
    // init
    Size *cameraSize = getCameraSize();
    
    // init capture image
    uchar depth8x3 = CV_8UC3;
    cv::Mat black_cap   (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat blue_cap    (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat green_cap   (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat red_cap     (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    
    // capture from some color light
    captureFromLinearLightOnProjectorDomain(&black_cap, CV_VEC3B_BLACK, false, SLEEP_TIME * 5);
    captureFromLinearLightOnProjectorDomain(&blue_cap, CV_VEC3B_BLUE);
    captureFromLinearLightOnProjectorDomain(&green_cap, CV_VEC3B_GREEN);
    captureFromLinearLightOnProjectorDomain(&red_cap, CV_VEC3B_RED);
    
    // show image
    imshow("black_cap", black_cap);
    imshow("blue_cap", blue_cap);
    imshow("green_cap", green_cap);
    imshow("red_cap", red_cap);
    waitKey(30);
    
    //
    const Mat l_PV1 = (Mat_<double>(4, 1) << CV_VEC3B_BLACK[0],CV_VEC3B_BLUE[0],CV_VEC3B_GREEN[0],CV_VEC3B_RED[0]);
    const Mat l_PV2 = (Mat_<double>(4, 1) << CV_VEC3B_BLACK[1],CV_VEC3B_BLUE[1],CV_VEC3B_GREEN[1],CV_VEC3B_RED[1]);
    const Mat l_PV3 = (Mat_<double>(4, 1) << CV_VEC3B_BLACK[2],CV_VEC3B_BLUE[2],CV_VEC3B_GREEN[2],CV_VEC3B_RED[2]);
    Mat l_K1(4, 1, CV_64FC1), l_K2(4, 1, CV_64FC1), l_K3(4, 1, CV_64FC1);
    Mat l_CK(4, 4, CV_64FC1), l_PK(4, 1, CV_64FC1);
    
    // set V
    int rows = black_cap.rows, cols = black_cap.cols;
    if (isContinuous(black_cap, red_cap, green_cap, blue_cap, *_V)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec12d* l_pV = _V->ptr<Vec12d>(y);
        const Vec3b* l_pBlack = black_cap.ptr<Vec3b>(y);
        const Vec3b* l_pBlue = blue_cap.ptr<Vec3b>(y);
        const Vec3b* l_pGreen = green_cap.ptr<Vec3b>(y);
        const Vec3b* l_pRed = red_cap.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // set l_CK
            for (int i = 0; i < 4; ++ i) {
                l_CK.at<double>(i, 3) = 1;
            }
            for (int i = 0; i < 3; ++ i) {
                l_CK.at<double>(0, i) = l_pBlack[x][i];
                l_CK.at<double>(1, i) = l_pBlue[x][i];
                l_CK.at<double>(2, i) = l_pGreen[x][i];
                l_CK.at<double>(3, i) = l_pRed[x][i];
            }
            Mat l_invCK = l_CK.inv();
            
            // calc
            l_K1 = l_invCK * l_PV1;
            l_K2 = l_invCK * l_PV2;
            l_K3 = l_invCK * l_PV3;
            
            // set
            for (int i = 0; i < 4; ++ i) {
                l_pV[x][i + 0] = l_K1.at<double>(i, 0);
                l_pV[x][i + 4] = l_K2.at<double>(i, 0);
                l_pV[x][i + 8] = l_K3.at<double>(i, 0);
            }
        }
    }
    
    cout << "color calibration finish!" << endl;
    return true;
}
bool ProCam::colorCalibration3(const bool _denoisingFlag){
    cout << "color calibration start!" << endl;
    
    // init
    Size cameraSize = getCameraSize_();
    
    // init project color
    std::vector<Vec3b> l_vecPrjColor;
    l_vecPrjColor.push_back(Vec3b(0, 0, g_maxLuminance[2]));
    l_vecPrjColor.push_back(Vec3b(0, g_maxLuminance[1], 0));
    l_vecPrjColor.push_back(Vec3b(g_maxLuminance[0], 0, 0));
    
    // capture from some color light
    cv::Mat black_cap(cameraSize, CV_8UC3, CV_SCALAR_BLACK);
    captureFromLinearLightOnProjectorDomain(&black_cap, CV_VEC3B_BLACK, _denoisingFlag, SLEEP_TIME * 5);
    g_F0 = black_cap.clone();
    
    int i = 0;
    vector<Mat> l_vecCapImg;
    for (vector<Vec3b>::const_iterator l_itr = l_vecPrjColor.begin();
         l_itr != l_vecPrjColor.end();
         ++ l_itr, ++ i) {
        cv::Mat l_capImg(cameraSize, CV_8UC3, CV_SCALAR_BLACK);
        captureFromLinearLightOnProjectorDomain(&l_capImg, *l_itr, _denoisingFlag);
        l_vecCapImg.push_back(l_capImg);
        
        // show
        ostringstream oss;
        oss << "projection image" << i;
        imshow(oss.str().c_str(), l_capImg);
    }
    
    // get vec C
    const double rate = 1.0 / 255.0;
    black_cap.convertTo(black_cap, CV_64FC3, rate);
    vector<Mat> l_vecCapImg_d;
    for (vector<Mat>::const_iterator l_capImgItr = l_vecCapImg.begin();
         l_capImgItr != l_vecCapImg.end();
         ++ l_capImgItr) {
        // Vec3b -> Vec3d
        Mat l_capImg_d = l_capImgItr->clone();
        l_capImgItr->convertTo(l_capImg_d, CV_64FC3, rate);
        
        // calc diff
        if (COLOR_CALIB_2004) {
            l_capImg_d -= black_cap;
        }
        
        // push
        l_vecCapImg_d.push_back(l_capImg_d);
    }
    
    // get vec P
    vector<Vec3d> l_vecPrjColor_d;
    for (vector<Vec3b>::const_iterator l_prjItr = l_vecPrjColor.begin();
         l_prjItr != l_vecPrjColor.end();
         ++ l_prjItr) {
        const Vec3d l_prjColor_d = (Vec3d)(*l_prjItr) * rate;
        l_vecPrjColor_d.push_back(l_prjColor_d);
    }
    
    // set V
    setV(l_vecCapImg_d, l_vecPrjColor_d);
    
    cout << "color calibration finish!" << endl;
    return true;
}

bool ProCam::test_colorCalibration(void){
    const Size* l_camSize = getCameraSize();
    Mat l_captureImage(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Mat l_captureImageNC(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Mat l_captureImageNCNL(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    
    for (int i = 1; i < 8; ++ i) {
        if (i == 3 || i >= 5) {
            continue;
        }
        const int l_col1 = i % 2;
        const int l_col2 = (i / 2) % 2;
        const int l_col3 = (i / 4) % 2;
        Vec3b l_mask(l_col3, l_col2, l_col1);
        ostringstream oss;
        oss <<CHECK_COLOR_CALIBRATION_FILE_NAME << l_col3 << l_col2 << l_col1 << ".dat";
        cout << oss.str() << endl;
        ofstream ofs(oss.str().c_str());

        for (int prj = 0; prj < 256; ++ prj) {
            // capture
            Vec3b l_color(prj * l_mask[0], prj * l_mask[1], prj * l_mask[2]);
            captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_color);
            captureFromLinearLightOnProjectorDomain(&l_captureImageNC, l_color);
            captureFromLightOnProjectorDomain(&l_captureImageNCNL, l_color);
            const Mat l_captureImage_ = l_captureImage / 255.0;
            MY_IMSHOW3(l_captureImage_, l_captureImageNC, l_captureImageNCNL);
            
            // get mean and standard deviation
            Vec3d l_meanColor(0, 0, 0), l_stddevColor(0, 0, 0), l_meanColorNC(0, 0, 0), l_stddevColorNC(0, 0, 0), l_meanColorNCNL(0, 0, 0), l_stddevColorNCNL(0, 0, 0);
            meanStdDev(l_captureImage, l_meanColor, l_stddevColor);
            meanStdDev(l_captureImageNC, l_meanColorNC, l_stddevColorNC);
            meanStdDev(l_captureImageNCNL, l_meanColorNCNL, l_stddevColorNCNL);
            
            // print
            std::cout << prj << "\t";
            _print_gnuplot6(std::cout, l_meanColor, l_stddevColor, l_meanColorNC, l_stddevColorNC, l_meanColorNCNL, l_stddevColorNCNL);
            ofs << prj << "\t";
            _print_gnuplot_color6_l(ofs, l_meanColor, l_stddevColor, l_meanColorNC, l_stddevColorNC, l_meanColorNCNL, l_stddevColorNCNL);
            
            // if you are mistake, you should push delete button then go back
            if (waitKey(30) == CV_BUTTON_DELETE) {
                return false;
            }
        }
        ofs.close();
    }
    return true;
}

///////////////////////////////  convert method ///////////////////////////////

// 幾何変換テーブルを用いて，カメラ座標系からプロジェクタ座標系に変換する
// output / _psImg  : プロジェクタ座標系に投影した画像
// input / _csImg   : カメラ座標系の画像
bool ProCam::convertProjectorDomainToCameraOne(cv::Mat* const _psImg, const cv::Mat&  _csImg){
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
    Mat l_psImg(_psImg->rows, _psImg->cols, CV_8UC3, Scalar(0, 0, 0));  // プロジェクタ空間上の画像（ローカル）
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
    const Mat* l_prjRes = getProjectorResponseI2P();               // プロジェクタ強度の線形化ルックアップテーブル
    const Mat l_compressedPrjRes(l_prjRes->rows, l_prjRes->cols / 256, CV_8UC3);
    if (!isEqualSize(*_linearImg, _nonLinearImg, l_compressedPrjRes)) {
        cerr << "different size" << endl;
        _print_mat_propaty(*_linearImg);
        _print_mat_propaty(_nonLinearImg);
        _print_mat_propaty(l_compressedPrjRes);
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


// PからIに変換する
bool ProCam::convertPtoIBySomePoint(cv::Mat* const _I, const cv::Mat&  _P, const cv::Point& _point){
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
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < ch; ++ c) {
                // convert
                const Point l_P2IPoint(_point.x * 256 + (int)l_pP[x][c], _point.y);
                l_pI[x][c] = (l_transP2I->at<Vec3b>(l_P2IPoint))[c];
            }
        }
    }
    
    return true;
}

void ProCam::convertColorSpace(cv::Mat* const _dst, const cv::Mat& _src, const bool P2CFlag){
    const Mat_<Vec9d>* l_V = getV();
    
    // error handle
    if (!isEqualSize(*_dst, _src, *l_V)) {
        cerr << "size is different" << endl;
        _print_mat_propaty(*_dst);
        _print_mat_propaty(_src);
        _print_mat_propaty(*l_V);
        exit(-1);
    }
    
    //
    int rows = _dst->rows, cols = _dst->cols;
    if (isContinuous(*_dst, _src, *l_V, g_F0)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        // init pointer
//        Vec3b* l_pDst = _dst->ptr<Vec3b>(y);
        Vec3d* l_pDst = _dst->ptr<Vec3d>(y);
        const Vec3b* l_pSrc = _src.ptr<Vec3b>(y);
        const Vec9d* l_pV = l_V->ptr<Vec9d>(y);
        const Vec3b* l_pF0 = g_F0.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // vec -> mat
            const Mat l_srcMat = Mat(Vec3d(l_pSrc[x]));
            const Mat l_F0Mat = Mat(Vec3d(l_pF0[x]));
            Mat l_VMat(3, 3, CV_64FC1);
            convertVecToMat(&l_VMat, l_pV[x]);
            
            // calc
            Mat colorMix, l_dstMat;
            if (P2CFlag) {
                colorMix = l_VMat;
                l_dstMat = colorMix * l_srcMat;
            } else {
                colorMix = l_VMat.inv();
                l_dstMat = colorMix * (l_srcMat - l_F0Mat);
                if (COLOR_CALIB_2003) {
                    l_dstMat = colorMix * l_srcMat;
                }
            }
            
            // mat -> vec
//            l_pDst[x] = Vec3b(l_dstMat);
            l_pDst[x] = l_dstMat;
        }
    }
}

// カメラの色空間からプロジェクタの色空間へ変換する
void ProCam::convertColorSpaceOfCameraToProjector(cv::Mat* const _imageOnPrj, const cv::Mat& _imageOnCam){
    convertColorSpace(_imageOnPrj, _imageOnCam, false);
}

// プロジェクタの色空間からカメラの色空間へ変換する
void ProCam::convertColorSpaceOfProjectorToCamera(cv::Mat* const _imageOnCam, const cv::Mat& _imageOnPrj){
    convertColorSpace(_imageOnCam, _imageOnPrj, true);
}

///////////////////////////////  show method ///////////////////////////////

// アクセスマップの表示
bool ProCam::showAccessMapCam2Prj(void){
    const Size* prjSize = getProjectorSize();
    const Size* camSize = getCameraSize();
    bool flag = true;
    Point pt(camSize->height / 2, camSize->width / 2);
    Mat l_capImage(*camSize, CV_8UC3);
    int l_lineLength = 50;

    cout << "show access map" << endl;
    while (flag) {
        // create projection image
        Mat whiteImg(*camSize, CV_8UC3, Scalar(255, 255, 255)), prjImg(*prjSize, CV_8UC3, Scalar(0, 0, 0));
//        Point l_startPt(pt.x - 1, pt.y - 1), l_endPt(pt.x + 1, pt.y + 1);
//        cv::rectangle(whiteImg, l_startPt, l_endPt, CV_SCALAR_BLUE, -1, CV_AA);
        cv::line(whiteImg, Point(pt.x - l_lineLength, pt.y), Point(pt.x + l_lineLength, pt.y), CV_SCALAR_BLUE);
        cv::line(whiteImg, Point(pt.x, pt.y - l_lineLength), Point(pt.x, pt.y + l_lineLength), CV_SCALAR_BLUE);
        
        // capture
        captureFromLightOnProjectorDomain(&l_capImage, whiteImg);
        Mat l_capImage_ = l_capImage.clone();
//        cv::rectangle(l_capImage, cv::Point(pt.x - 1,pt.y - 1), cv::Point(pt.x + 1, pt.y + 1), CV_SCALAR_RED, -1, CV_AA);
        cv::line(l_capImage, Point(pt.x - l_lineLength, pt.y), Point(pt.x + l_lineLength, pt.y), CV_SCALAR_RED);
        cv::line(l_capImage, Point(pt.x, pt.y - l_lineLength), Point(pt.x, pt.y + l_lineLength), CV_SCALAR_RED);

        MY_IMSHOW2(l_capImage, l_capImage_);

        int pushKey = waitKey(-1);
        switch (pushKey) {
            case (CV_BUTTON_ESC):
                flag = false;
                break;
            case (CV_BUTTON_UP):
                pt.y -= 10;
                break;
            case (CV_BUTTON_LEFT):
                pt.x -= 10;
                break;
            case (CV_BUTTON_DOWN):
                pt.y += 10;
                break;
            case (CV_BUTTON_RIGHT):
                pt.x += 10;
                break;
                
            case (CV_BUTTON_s):
                l_lineLength += 10;
                break;
            case (CV_BUTTON_S):
                l_lineLength += 10;
                break;
                
            default:
                break;
        }
    }
    cout << "finish show access map" << endl;

    return true;
}

// response mapの表示
bool ProCam::showProjectorResponseI2P(void){
    const Mat* l_responseMap = getProjectorResponseI2P();
    
    return showProjectorResponse(*l_responseMap);
}

bool ProCam::showProjectorResponseP2I(void){
    const Mat* l_responseMap = getProjectorResponseP2I();
        
    return showProjectorResponse(*l_responseMap);
}

bool ProCam::showProjectorResponse(const cv::Mat& _prjRes){
    Mat l_responseImage(_prjRes.rows, _prjRes.cols / 256, CV_8UC3, Scalar(0, 0, 0));
    
    for (int i = 0;;) {
        _print(i);
        getImageProjectorResponseP2I(&l_responseImage, _prjRes, i);
        Mat l_flatImage(l_responseImage.rows, l_responseImage.cols, CV_8UC3, Scalar(i, i, i));
        
        MY_IMSHOW(l_flatImage);
        MY_IMSHOW(l_responseImage);
        if (i > 255) i = 0;
        if (i < 0) i = 255;
        
        // handle image
        int pushKey = waitKey(-1);
        if (pushKey == CV_BUTTON_LEFT || pushKey == CV_BUTTON_DOWN){
            ++ i;
        }
        if (pushKey == CV_BUTTON_RIGHT || pushKey == CV_BUTTON_UP){
            -- i;
        }
        if (pushKey == CV_BUTTON_ESC) {
            cout<<"finish show image"<<endl;break;
        }
    }
    destroyAllWindows();
    
    return true;
}

void ProCam::showV(void){
    // init
    const Mat_<Vec9d>* l_cmmm = getV();
    int rows = l_cmmm->rows, cols = l_cmmm->cols;
    Mat_<Vec3d> l_VRed(rows, cols), l_VGreen(rows, cols), l_VBlue(rows, cols);
    if (isContinuous(*l_cmmm, l_VRed, l_VGreen, l_VBlue)) {
        cols *= rows;
        rows = 1;
    }
    
    // create V images
    for (int y = 0; y < rows; ++ y) {
        const Vec9d* p_cmmm = l_cmmm->ptr<Vec9d>(y);
        Vec3d* p_VRed = l_VRed.ptr<Vec3d>(y);
        Vec3d* p_VGreen = l_VGreen.ptr<Vec3d>(y);
        Vec3d* p_VBlue = l_VBlue.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            p_VBlue[x] = Vec3d(p_cmmm[x][0], p_cmmm[x][1], p_cmmm[x][2]);
            p_VGreen[x] = Vec3d(p_cmmm[x][3], p_cmmm[x][4], p_cmmm[x][5]);
            p_VRed[x] = Vec3d(p_cmmm[x][6], p_cmmm[x][7], p_cmmm[x][8]);
        }
    }
    
    // show
    MY_IMSHOW(l_VRed);
    MY_IMSHOW(l_VGreen);
    MY_IMSHOW(l_VBlue);
    waitKey(30);
//    MY_WAIT_KEY(CV_BUTTON_ESC);
}

///////////////////////////////  print method ///////////////////////////////

// projector responseの出力
bool ProCam::printProjectorResponseI2P(const cv::Point& _pt){
    const Mat* l_responseMap = getProjectorResponseI2P();
    
    return printProjectorResponse(_pt, *l_responseMap);
}
//
bool ProCam::printProjectorResponseP2I(const cv::Point& _pt){
    const Mat* l_responseMap = getProjectorResponseP2I();
    
    return printProjectorResponse(_pt, *l_responseMap);
}
bool ProCam::printProjectorResponse(const cv::Point& _pt, const cv::Mat& _prjRes){
    Mat l_responseImage(_prjRes.rows, _prjRes.cols / 256, CV_8UC3, Scalar(0, 0, 0));
    
    for (int i = 0; i < 256; ++ i) {
        getImageProjectorResponseP2I(&l_responseImage, _prjRes, i);
        
        Vec3i tmp = (Vec3i)l_responseImage.at<Vec3b>(_pt);
        _print_gnuplot4(std::cout, i, tmp[2], tmp[1], tmp[0]);
    }
    
    return true;
}

///////////////////////////////  capture from light method ///////////////////////////////

// output / captureImage    : 撮影した画像を代入する場所
// input / projectionImage  : 投影する画像
// return                   : 成功したかどうか
bool ProCam::captureFromLight(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, const bool _denoiseFlag, const int _waitTimeNum){
    // 投影
    imshow(WINDOW_NAME, _projectionImage);
    cvMoveWindow(WINDOW_NAME, POSITION_OF_PROJECTION_IMAGE.x, POSITION_OF_PROJECTION_IMAGE.y);
    cv::waitKey(_waitTimeNum);
    
    // get image
    cv::Mat l_image(_captureImage->rows, _captureImage->cols, CV_8UC3);
    if (_denoiseFlag) {
        // 複数撮影し，ベクタに格納
        std::vector<Mat> l_vCaptureImages;
        for (int i = 0; i < 10; ++ i) {
            cv::Mat l_tmp(_captureImage->rows, _captureImage->cols, CV_8UC3);
            getCaptureImage(&l_tmp);
            l_vCaptureImages.push_back(l_tmp);
        }
        
        // get average
        meanMat(&l_image, l_vCaptureImages);
    } else {
        // normal capture
        getCaptureImage(&l_image);
    }
    
    *_captureImage = l_image.clone();
    cv::waitKey(_waitTimeNum);
    
    return true;
}
bool ProCam::captureFromLight(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, const bool _denoiseFlag, const int _waitTimeNum){
    const Size* l_prjSize = getProjectorSize();
    const Mat l_projectionImage(*l_prjSize, CV_8UC3, Scalar(_projectionColor));
    return captureFromLight(_captureImage, l_projectionImage, _denoiseFlag, _waitTimeNum);
}
bool ProCam::captureFromLight(cv::Mat* const _captureImage, const uchar& _projectionNum, const bool _denoiseFlag, const int _waitTimeNum){
    const Vec3b l_projectionColor(_projectionNum, _projectionNum, _projectionNum);
    return captureFromLight(_captureImage, l_projectionColor, _denoiseFlag, _waitTimeNum);
}

// 幾何変換を行ったものを投影・撮影
bool ProCam::captureFromLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, const bool _denoiseFlag, const int _waitTimeNum){
    // error processing
    const Size* l_camSize = getCameraSize();
    if (_projectionImage.rows != l_camSize->height || _projectionImage.cols != l_camSize->width) {
        cout << "size is different" << endl;
        _print_mat_propaty(_projectionImage);
        _print(*l_camSize);
        exit(-1);
    }
    
    const Size* l_prjSize = getProjectorSize();
    Mat l_projectionImageOnProjectorSpace(*l_prjSize, CV_8UC3, Scalar(0, 0, 0));
    convertProjectorDomainToCameraOne(&l_projectionImageOnProjectorSpace, _projectionImage);
    return captureFromLight(_captureImage, l_projectionImageOnProjectorSpace, _denoiseFlag, _waitTimeNum);
}
bool ProCam::captureFromLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, const bool _denoiseFlag, const int _waitTimeNum){
    const Size* l_camSize = getCameraSize();    // カメラサイズの取得
    const Mat l_projectionImage(*l_camSize, CV_8UC3, Scalar(_projectionColor)); // 投影画像
    return captureFromLightOnProjectorDomain(_captureImage, l_projectionImage, _denoiseFlag, _waitTimeNum);
}
bool ProCam::captureFromLightOnProjectorDomain(cv::Mat* const _captureImage, const uchar _projectionNumber, const bool _denoiseFlag, const int _waitTimeNum){
    const Vec3b l_projectionColor(_projectionNumber, _projectionNumber, _projectionNumber);
    return captureFromLightOnProjectorDomain(_captureImage, l_projectionColor, _denoiseFlag, _waitTimeNum);
}

// 線形化したプロジェクタを用いて投影・撮影を行う
bool ProCam::captureFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, const bool _denoiseFlag, const int _waitTimeNum){
    // geometric translate
    const Size* l_prjSize = getProjectorSize();
    Mat l_projectionImageOnProjectorSpace(*l_prjSize, CV_8UC3, Scalar(0, 0, 0));
    convertProjectorDomainToCameraOne(&l_projectionImageOnProjectorSpace, _projectionImage);
    
    // linearize
    Mat l_linearProjectionImage(l_projectionImageOnProjectorSpace.rows, l_projectionImageOnProjectorSpace.cols, CV_8UC3, Scalar(0, 0, 0));    // プロジェクタ強度の線形化を行った後の投影像
    
    // non linear image -> linear one
    // Pointの値を変えて，線形化LUTの参照ポイントを変更可能
    convertPtoI(&l_linearProjectionImage, l_projectionImageOnProjectorSpace);
    
    // 幾何変換後に投影
    return captureFromLight(_captureImage, l_linearProjectionImage, _denoiseFlag, _waitTimeNum);
}
bool ProCam::captureFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, const bool _denoiseFlag, const int _waitTimeNum){
    const Size* l_camSize = getCameraSize();    // カメラサイズの取得
    const Mat l_projectionImage(*l_camSize, CV_8UC3, Scalar(_projectionColor)); // 投影画像
    return captureFromLinearLightOnProjectorDomain(_captureImage, l_projectionImage, _denoiseFlag, _waitTimeNum);
}
bool ProCam::captureFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const uchar _projectionNumber, const bool _denoiseFlag, const int _waitTimeNum){
    const Vec3b l_projectionColor(_projectionNumber, _projectionNumber, _projectionNumber);
    return captureFromLinearLightOnProjectorDomain(_captureImage, l_projectionColor, _denoiseFlag, _waitTimeNum);
}

// 線形化した投影光をプロジェクタの色空間の画像で取得する
bool ProCam::captureOfProjecctorColorFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, const bool _denoiseFlag, const int _waitTimeNum){
    // 投影し撮影
    Mat tmp = _captureImage->clone();
    captureFromLinearLightOnProjectorDomain(&tmp, _projectionImage, _denoiseFlag, _waitTimeNum);
    
    // convert color space camera to projector
    convertColorSpaceOfCameraToProjector(_captureImage, tmp);
    
    return true;
}
bool ProCam::captureOfProjecctorColorFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, const bool _denoiseFlag, const int _waitTimeNum){
    const Size* l_camSize = getCameraSize();    // カメラサイズの取得
    const Mat l_projectionImage(*l_camSize, CV_8UC3, Scalar(_projectionColor)); // 投影画像
    return captureOfProjecctorColorFromLinearLightOnProjectorDomain(_captureImage, l_projectionImage, _denoiseFlag, _waitTimeNum);
}
bool ProCam::captureOfProjecctorColorFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const uchar _projectionNumber, const bool _denoiseFlag, const int _waitTimeNum){
    const Vec3b l_projectionColor(_projectionNumber, _projectionNumber, _projectionNumber);
    return captureOfProjecctorColorFromLinearLightOnProjectorDomain(_captureImage, l_projectionColor, _denoiseFlag, _waitTimeNum);
}

///////////////////////////////  other method ///////////////////////////////

// projector responseの補間を行う
bool ProCam::interpolationProjectorResponseP2I(cv::Mat* const _prjRes){
    cout << "interpolation now ..." << endl;
    // initialize
    int rows = _prjRes->rows, cols = _prjRes->cols / 256, channels = _prjRes->channels();
    if (_prjRes->isContinuous()) {
        cols *= rows;
        rows = 1;
    }
    
    // scan all pixel
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pPrjRes = _prjRes->ptr<Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            
            // scan all color
            for (int c = 0; c < channels; ++ c) {
                // scan all luminance
                for (int p = 1; p < 255; ++ p) {    // 最初と最後は初期化の時に決定済みである為，除外
                    if (l_pPrjRes[x * 256 + p][c] == INIT_RES_NUM) {   // 抜けている箇所の特定
                        uchar p0 = p - 1, i0 = l_pPrjRes[x * 256 + p0][c];  // 下端の値

                        // 上端の検索
                        uchar p1 = p + 1, i1 = 0;
                        for (; ; ++ p1) {
//                            cout << (int)p1 << endl;
                            if (l_pPrjRes[x * 256 + p1][c] != INIT_RES_NUM || p1 == 255) {
                                i1 = l_pPrjRes[x * 256 + p1][c];
//                                cout << "top is " << (int)p1 << endl;
//                                cout << "bottom is " << (int)p << endl;
                                break;
                            }
                        }
//                        if (p == 1 && p1 == 255) {
//                            l_pPrjRes[x * 256 + p1][c] = INIT_RES_NUM;
//                            break;
//                        }
                        
                        // 抜けていた箇所全てを修復
                        for (int p_current = p; p_current < p1; ++ p_current) {
                            // set alpha
                            double alpha = (double)(p_current - p0)/(double)(p1 - p0);
                            
                            // 抜けている箇所の修復
                            l_pPrjRes[x * 256 + p_current][c] = i0 + (uchar)(alpha * (double)(i1 - i0));
                        }
                    }
                }
            }
        }
    }
    
    cout << "finished interpolation" << endl;

    return true;
}
// test of front method
bool ProCam::test_interpolationProjectorResponseP2I(void){
    Mat m1(2, 256, CV_8UC3, Scalar(INIT_RES_NUM, INIT_RES_NUM, INIT_RES_NUM));
//    m1.at<Vec3b>(0, 255) = Vec3b(127, 127, 127);
//    m1.at<Vec3b>(1, 255) = Vec3b(0, 0, 0);ls
    for (int i = 0; i < 256; i += 5) {
        m1.at<Vec3b>(0, i) = Vec3b(i, i, i);
        m1.at<Vec3b>(1, i) = Vec3b(0, 0, 0);
        if (i > 128) {
            m1.at<Vec3b>(1, i) = CV_VEC3B_WHITE;
        }
    }
//    m1.at<Vec3b>(1, 255) = Vec3b(255, 255, 255);
    _print(m1);
    interpolationProjectorResponseP2I(&m1);
    _print(m1);
    
    return true;
}

// カメラ撮影場所以外の応答特性を設定
bool ProCam::interpolateProjectorResponseP2IAtOutOfCameraArea(cv::Mat* const _prjResP2I){
    // error handle
    const Size* l_prjSize = getProjectorSize();
    if (_prjResP2I->rows != l_prjSize->height || _prjResP2I->cols != l_prjSize->width * 256) {
        cerr << "different size from ["<<l_prjSize->height<<", "<<l_prjSize->height * 256<<"]" << endl;
        _print_mat_propaty(*_prjResP2I);
        exit(-1);
    }
    
    // create non camera area flag
    const Size* l_camSize = getCameraSize();
    const Mat whiteImage(*l_camSize, CV_8UC3, CV_SCALAR_WHITE);
    Mat l_checkNonCameraAreaFlag(*l_prjSize, CV_8UC3, CV_SCALAR_BLACK);
    convertProjectorDomainToCameraOne(&l_checkNonCameraAreaFlag, whiteImage);
    
    // set non camera area
    for (int P = 0; P < 256; ++ P) {
        Vec3b l_aveColor(0, 0, 0);
        Vec3d l_sum(0, 0, 0);
        int cnt = 0;
        
        // calc sum color
        for (int y = 0; y < l_prjSize->height; ++ y) {
            const Vec3b* l_pCheckNonCameraAreaFlag = l_checkNonCameraAreaFlag.ptr<Vec3b>(y);
            const Vec3b* l_pPrjResP2I = _prjResP2I->ptr<Vec3b>(y);
            
            for (int x = 0; x < l_prjSize->width; ++ x) {
                if (l_pCheckNonCameraAreaFlag[x] == CV_VEC3B_WHITE) {
                    l_sum += l_pPrjResP2I[x * 256 + P];
                    ++ cnt;
                }
            }
        }
        
        // get average
//        l_aveColor = (Vec3b)(l_sum / (l_camSize->area()));
        l_aveColor = (Vec3b)(l_sum / (double)cnt);
        
        // set response at out of camera area
        for (int y = 0; y < l_prjSize->height; ++ y) {
            const Vec3b* l_pCheckNonCameraAreaFlag = l_checkNonCameraAreaFlag.ptr<Vec3b>(y);
            Vec3b* l_pPrjResP2I = _prjResP2I->ptr<Vec3b>(y);
            
            for (int x = 0; x < l_prjSize->width; ++ x) {
                if (l_pCheckNonCameraAreaFlag[x] == CV_VEC3B_BLACK) {
                    l_pPrjResP2I[x * 256 + P] = l_aveColor;
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

// 藤井らの手法の光学補正
bool ProCam::doRadiometricCompensation(const cv::Mat& _desiredImage, const int _waitTimeNum){
    cout << "対象となる紙をセットして下さい．" << endl;
    cout << "セット出来ましたらESCボタンを押して下さい．" << endl;
    MY_WAIT_KEY(CV_BUTTON_ESC);

    // 色変換行列(V)を計算
    colorCalibration();
    showV();
    
    // 環境光(F)を計算
    const Size* l_camSize = getCameraSize();
    const Mat whiteImage(*l_camSize, CV_8UC3, CV_SCALAR_WHITE);
    setF(whiteImage);
    
    // 投影画像を計算する
    Mat l_projectionImageOnCameraSpace(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    getNextProjectionImage(&l_projectionImageOnCameraSpace, _desiredImage);
    
    // 上で決定した投影光からCを復元出来るか試す
    Mat l_reconstructC(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    reconstructC(&l_reconstructC, l_projectionImageOnCameraSpace);

    // project desired image
    Mat l_cameraImageFromDesiredImageProjection(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    captureFromLightOnProjectorDomain(&l_cameraImageFromDesiredImageProjection, _desiredImage, _waitTimeNum);
    
    // project compensated image
    Mat l_cameraImage(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    captureFromLinearLightOnProjectorDomain(&l_cameraImage, l_projectionImageOnCameraSpace, _waitTimeNum);
    
    // calc difference
    Vec3d l_diffC(0.0, 0.0, 0.0), l_diffPDC(0.0, 0.0, 0.0);
    getAvgOfDiffMat2(&l_diffC, _desiredImage, l_cameraImage);
    getAvgOfDiffMat2(&l_diffPDC, _desiredImage, l_cameraImageFromDesiredImageProjection);
    int index = (int)_desiredImage.at<Vec3b>(0,0)[0];
    _print_gnuplot7(cout, index, l_diffC[2], l_diffC[1], l_diffC[0], l_diffPDC[2], l_diffPDC[1], l_diffPDC[0]);
    Vec3b l_aveC, l_avePDC;
    calcAverageOfImage(&l_aveC, l_cameraImage);
    calcAverageOfImage(&l_avePDC, l_cameraImageFromDesiredImageProjection);
    _print3(index, l_avePDC, l_aveC);
    
    // show images
    imshow("desired C", _desiredImage);
    imshow("P on Camera Domain", l_projectionImageOnCameraSpace);
    imshow("C", l_cameraImage);
    imshow("C when projection desired C", l_cameraImageFromDesiredImageProjection);
    waitKey(30);
    
    return true;
}

bool ProCam::doRadiometricCompensation(const cv::Vec3b& _desiredColor, const int _waitTimeNum){
    const Size* l_camSize = getCameraSize();
    const Scalar l_color(_desiredColor);
    Mat l_image(*l_camSize, CV_8UC3, l_color);
//    return doRadiometricCompensation(l_image, _waitTimeNum);
    return doRadiometricCompensation(l_image, _waitTimeNum);
}
bool ProCam::doRadiometricCompensation(const uchar& _desiredColorNumber, const int _waitTimeNum){
    return doRadiometricCompensation(Vec3b(_desiredColorNumber, _desiredColorNumber, _desiredColorNumber), _waitTimeNum);
}
bool ProCam::doRadiometricCompensation(const char* _fileName, const int _waitTimeNum){
    Mat l_image = imread(_fileName, 1);
//    return doRadiometricCompensation(l_image, _waitTimeNum);
    return doRadiometricCompensation(l_image, _waitTimeNum);
}
// C = F + VP
void ProCam::reconstructC(cv::Mat* const _C, const cv::Mat& _P){
    const Mat* l_F = getF();
    const Mat_<Vec9d>* l_V = getV();
    
    int rows = _C->rows, cols = _C->cols;
    if (isContinuous(*_C, _P, *l_F, *l_V)) {
        cols *= rows;
        rows = 1;
    }
    Mat l_matP(3, 1, CV_64FC1), l_matF(3, 1, CV_64FC1), l_matC(3, 1, CV_64FC1), l_matV(3, 3, CV_64FC1);
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3b* l_pC = _C->ptr<Vec3b>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3b* l_pF = l_F->ptr<Vec3b>(y);
        const Vec9d* l_pV = l_V->ptr<Vec9d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // vec -> mat
            l_matF = Mat(Vec3d(l_pF[x]));
            l_matP = Mat(Vec3d(l_pP[x]));
            convertVecToMat(&l_matV, l_pV[x]);
            
            // calc
            l_matP = l_matF + l_matV * l_matP;
            
            // mat -> vec
            l_pC[x] = Vec3b(l_matP);
        }
    }
}

// yoshidaらの手法
bool ProCam::doRadiometricCompensationByYoshidaMethod(const cv::Mat& _desiredImage, const int _waitTimeNum){
    cout << "対象となる紙をセットして下さい．" << endl;
    cout << "セット出来ましたらESCボタンを押して下さい．" << endl;
    MY_WAIT_KEY(CV_BUTTON_ESC);
    
    // 色変換行列(V)を計算
    const Size* l_camSize = getCameraSize();
    Mat_<Vec12d> l_K(*l_camSize);
    colorCalibration2(&l_K);
    
    // 投影画像を計算する
    Mat l_projectionImageOnCameraSpace(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    //    getNextProjectionImage(&l_projectionImageOnCameraSpace, _desiredImage);
    getProjectionImage(&l_projectionImageOnCameraSpace, _desiredImage, l_K);
    
    // project desired image
    Mat l_cameraImageFromDesiredImageProjection(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    captureFromLightOnProjectorDomain(&l_cameraImageFromDesiredImageProjection, _desiredImage, _waitTimeNum);
    
    // project compensated image
    Mat l_cameraImage(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    captureFromLinearLightOnProjectorDomain(&l_cameraImage, l_projectionImageOnCameraSpace, _waitTimeNum);
    
    // calc difference
    Vec3d l_diffC(0.0, 0.0, 0.0), l_diffPDC(0.0, 0.0, 0.0);
    getAvgOfDiffMat2(&l_diffC, _desiredImage, l_cameraImage);
    getAvgOfDiffMat2(&l_diffPDC, _desiredImage, l_cameraImageFromDesiredImageProjection);
    int index = (int)_desiredImage.at<Vec3b>(0,0)[0];
    _print_gnuplot7(cout, index, l_diffC[2], l_diffC[1], l_diffC[0], l_diffPDC[2], l_diffPDC[1], l_diffPDC[0]);
    Vec3b l_aveC, l_avePDC;
    calcAverageOfImage(&l_aveC, l_cameraImage);
    calcAverageOfImage(&l_avePDC, l_cameraImageFromDesiredImageProjection);
    _print3(index, l_avePDC, l_aveC);
    
    // show images
    imshow("desired C", _desiredImage);
    imshow("P on Camera Domain", l_projectionImageOnCameraSpace);
    imshow("C", l_cameraImage);
    imshow("C when projection desired C", l_cameraImageFromDesiredImageProjection);
    waitKey(30);
    
    return true;
}

// 天野らの手法の光学補正
bool ProCam::doRadiometricCompensationByAmanoMethod(const cv::Mat& _desiredImage, const int _waitTimeNum){
    cout << "対象となる紙をセットして下さい．" << endl;
    cout << "セット出来ましたらESCボタンを押して下さい．" << endl;
    MY_WAIT_KEY(CV_BUTTON_ESC);
    
    // 色変換行列(V)を計算
    colorCalibration();
    showV();
    
    // 環境光(F)を計算
    const Size* l_camSize = getCameraSize();
    const Mat whiteImage(*l_camSize, CV_8UC3, CV_SCALAR_WHITE);
    setF(whiteImage);
    
    // 投影画像を計算する
    Mat l_projectionImageOnCameraSpace(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    getNextProjectionImage(&l_projectionImageOnCameraSpace, _desiredImage);
    
    // 上で決定した投影光からCを復元出来るか試す
    Mat l_reconstructC(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    reconstructC(&l_reconstructC, l_projectionImageOnCameraSpace);
    
    // project desired image
    Mat l_cameraImageFromDesiredImageProjection(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    captureFromLightOnProjectorDomain(&l_cameraImageFromDesiredImageProjection, _desiredImage, _waitTimeNum);
    
    // project compensated image
    Mat l_cameraImage(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    captureFromLinearLightOnProjectorDomain(&l_cameraImage, l_projectionImageOnCameraSpace, _waitTimeNum);
    
    // calc difference
    Vec3d l_diffC(0.0, 0.0, 0.0), l_diffPDC(0.0, 0.0, 0.0);
    getAvgOfDiffMat2(&l_diffC, _desiredImage, l_cameraImage);
    getAvgOfDiffMat2(&l_diffPDC, _desiredImage, l_cameraImageFromDesiredImageProjection);
    int index = (int)_desiredImage.at<Vec3b>(0,0)[0];
    _print_gnuplot7(cout, index, l_diffC[2], l_diffC[1], l_diffC[0], l_diffPDC[2], l_diffPDC[1], l_diffPDC[0]);
    Vec3b l_aveC, l_avePDC;
    calcAverageOfImage(&l_aveC, l_cameraImage);
    calcAverageOfImage(&l_avePDC, l_cameraImageFromDesiredImageProjection);
    _print3(index, l_avePDC, l_aveC);
    
    // show images
    imshow("desired C", _desiredImage);
    imshow("P on Camera Domain", l_projectionImageOnCameraSpace);
    imshow("C", l_cameraImage);
    imshow("C when projection desired C", l_cameraImageFromDesiredImageProjection);
    waitKey(30);
    
    return true;
}

// P = KC
void ProCam::getProjectionImage(cv::Mat* const _P, const cv::Mat& _desireC, const cv::Mat_<Vec12d>& _K){
    // error handle
    if (!isEqualSize(*_P, _desireC, _K)) {
        cerr << "different size" << endl;
        _print_mat_propaty(*_P);
        _print_mat_propaty(_desireC);
        _print_mat_propaty(_K);
        exit(-1);
    }
    
    //
    int rows = _P->rows, cols = _P->cols;
    if (isContinuous(*_P, _desireC, _K)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3b* l_pP = _P->ptr<Vec3b>(y);
        const Vec3b* l_pDesireC = _desireC.ptr<Vec3b>(y);
        const Vec12d* l_pK = _K.ptr<Vec12d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // vec -> mat
            Mat l_matP(3, 1, CV_64FC1), l_matC(4, 1, CV_64FC1), l_matK(3, 4, CV_64FC1);
            // set C
            for (int i = 0; i < 3; ++ i) {
                l_matC.at<double>(i, 0) = (double)(l_pDesireC[x][i]);
            }
            l_matC.at<double>(3, 0) = 1.0;
            // set K
            for (int ly = 0; ly < 3; ++ ly) {
                for (int lx = 0; lx < 4; ++ lx) {
                    l_matK.at<double>(ly, lx) = l_pK[x][lx + ly * 4];
                }
            }
            
            // calc
            l_matP = l_matK * l_matC;
            
            // mat -> vec
            l_pP[x] = Vec3b(l_matP);
        }
    }
}

// 応答特性にメディアンフィルタをかける
bool ProCam::medianBlurForProjectorResponseP2I(cv::Mat* const _dst, const cv::Mat& _src){
    cout << "start median" << endl;
    Mat l_image(_src.rows, _src.cols / 256, CV_8UC3, CV_SCALAR_BLACK);
    for (int i = 0; i < 256; ++ i) {
        getImageProjectorResponseP2I(&l_image, _src, i);
        medianBlur(l_image, l_image, 7);
        ostringstream oss;
        oss << PROJECTOR_RESPONSE_P2I_IMAGE_PATH << i << ".png";
        imwrite(oss.str().c_str(), l_image);
        setImageProjectorResponseP2I(_dst, l_image, i);
    }
    cout << "finished median" << endl;
    return true;
}
bool ProCam::test_medianBlurForProjectorResponseP2I(void){
    Mat l_imageMap(10, 2560, CV_8UC3, CV_SCALAR_BLACK);
    for (int y = 0; y < 10; ++ y) {
        for (int x = 0; x < 10; ++ x) {
            l_imageMap.at<Vec3b>(x, y * 256 + 5) = CV_VEC3B_BLUE;
        }
    }
    for (int i = 0; i < 2560; ++ i) {
        l_imageMap.at<Vec3b>(i % 10, i) = CV_VEC3B_WHITE;
    }
    imshow("before", l_imageMap);
    medianBlurForProjectorResponseP2I(&l_imageMap, l_imageMap);
    imshow("after", l_imageMap);
    return true;
}

// プロカムのパラメータ調整の為に，投影・撮影を繰り返す
bool ProCam::settingProjectorAndCamera(void){
    const Size l_camSize = getCameraSize_();
    Mat l_projectionImage(l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Mat l_captureImage(l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    bool l_loopFlag = true;
    int l_caprjType = 0, l_luminance = 255;
    Vec3b l_mask(1, 1, 1), l_projectionColor = l_mask * l_luminance;
    
    while (l_loopFlag) {
        // project and capture
        l_projectionColor = l_mask * l_luminance;
        switch (l_caprjType) {
            case 0:
                cout << "captureFromLight" << endl;
                captureFromLight(&l_captureImage, l_projectionColor);
                break;
            case 1:
                cout << "captureFromLightOnProjectorDomain" << endl;
                captureFromLightOnProjectorDomain(&l_captureImage, l_projectionColor);
                break;
            case 2:
                cout << "captureFromLinearLightOnProjectorDomain" << endl;
                captureFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionColor);
                break;
            case 3:
                cout << "captureOfProjecctorColorFromLinearLightOnProjectorDomain" << endl;
                l_captureImage.convertTo(l_captureImage, CV_64FC3);
                captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionColor);
                l_captureImage.convertTo(l_captureImage, CV_8UC3);
                break;
                
            default:
                break;
        }
        
        // calc
        Scalar l_mean(0, 0, 0, 0), l_stddev(0, 0, 0, 0);
        meanStdDev(l_captureImage, l_mean, l_stddev);
        
        // show
        _print_gnuplot2(std::cout, l_mean, l_stddev);
        MY_IMSHOW(l_captureImage);
        
        int l_key = waitKey(-1);
        switch (l_key) {
            case CV_BUTTON_r:
                l_mask = (Vec3b)CV_VEC3D_RED;
                break;
            case CV_BUTTON_g:
                l_mask = (Vec3b)CV_VEC3D_GREEN;
                break;
            case CV_BUTTON_b:
                l_mask = (Vec3b)CV_VEC3D_BLUE;
                break;
            case CV_BUTTON_k:
                l_mask = (Vec3b)CV_VEC3D_BLACK;
                break;
            case CV_BUTTON_w:
                l_mask = (Vec3b)CV_VEC3D_WHITE;
                break;
            case CV_BUTTON_y:
                l_mask = (Vec3b)CV_VEC3D_YELLOW;
                break;
            case CV_BUTTON_p:
                l_mask = (Vec3b)CV_VEC3D_PURPLE;
                break;
            case CV_BUTTON_c:
                l_mask = (Vec3b)CV_VEC3D_CYAN;
                break;
                
            case CV_BUTTON_UP:
                l_luminance ++;
                _print(l_luminance);
                break;
            case CV_BUTTON_DOWN:
                l_luminance --;
                _print(l_luminance);
                break;
            case CV_BUTTON_RIGHT:
                l_luminance += 10;
                _print(l_luminance);
                break;
            case CV_BUTTON_LEFT:
                l_luminance -= 10;
                _print(l_luminance);
                break;
                
            case CV_BUTTON_0:
                l_caprjType = 0;
                break;
            case CV_BUTTON_1:
                l_caprjType = 1;
                break;
            case CV_BUTTON_2:
                l_caprjType = 2;
                break;
            case CV_BUTTON_3:
                l_caprjType = 3;
                break;
                
            case CV_BUTTON_DELETE:
                l_loopFlag = false;
                break;
                
            default:
                break;
        }
    }
    return true;
}

