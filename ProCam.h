//
//  ProCam.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/01.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#ifndef __cameraBase03__ProCam__
#define __cameraBase03__ProCam__

#include "LinearizerOfProjector.h"
#include "myOpenCV.h"
#include "myDc1394.h"
#include "common.h"

#define GEO_CAL_CALC_FLAG       // 幾何変換を計算するフラグ
//#define PRJ_LINEAR_CALC_FLAG    // 線形化変換を計算するフラグ

//#define BAYER_FLAG
#define LIB_DC1394_FLAG

// 定義
const cv::Size VGA_SIZE(640, 480);
const cv::Size XGA_SIZE(1024, 768);
const cv::Size HDTV_720P_SIZE(1280, 720);
const cv::Size WSXGA_PLUS_SIZE(1680, 1050);

const cv::Size MAC_DISIPLAY_SIZE(1366, 768);
const cv::Size LINUX_DISPLAY_SIZE(WSXGA_PLUS_SIZE);
const cv::Size MAC_OTHER_DISPLAY_SIZE(WSXGA_PLUS_SIZE);
const cv::Size PROJECTOR_DISPLAY_SIZE(XGA_SIZE);
const cv::Size IEEE_CAMERA_SIZE(VGA_SIZE);
const cv::Size IEEE_CAMERA_SIZE2(XGA_SIZE);
const cv::Size USB_CAMERA_SIZE(2592, 1944);
const cv::Size MAC_INSIDE_CAMERA_SIZE(HDTV_720P_SIZE);

const cv::Point MAC_OTHER_DISPLAY_POS(0, -1 * MAC_OTHER_DISPLAY_SIZE.height);
const cv::Point MAC_PROJECTOR_DISPLAY_POS(0, -1 * PROJECTOR_DISPLAY_SIZE.height);
const cv::Point LINUX_PROJECTOR_DISPLAY_POS(LINUX_DISPLAY_SIZE.width, 0);

#ifdef MAC
const cv::Size PRJ_SIZE(MAC_OTHER_DISPLAY_SIZE);
const cv::Point POSITION_OF_PROJECTION_IMAGE(MAC_OTHER_DISPLAY_POS);
#endif

#ifdef MAC_PROJECTOR
const cv::Size PRJ_SIZE(PROJECTOR_DISPLAY_SIZE);
const cv::Point POSITION_OF_PROJECTION_IMAGE(MAC_PROJECTOR_DISPLAY_POS);
#endif

#ifdef LINUX   // Linux
const cv::Size PRJ_SIZE(PROJECTOR_DISPLAY_SIZE);
const cv::Point POSITION_OF_PROJECTION_IMAGE(LINUX_PROJECTOR_DISPLAY_POS);
#endif

// 幾何キャリブレーションで得たルックアップテーブルのファイル名
#define LOOK_UP_TABLE_FILE_NAME "calibrationData/lookUpTableCameraToProjector.dat"  // 幾何変換表（カメラ=>プロジェクタ）
#define PROJECTOR_RESPONSE_I2P_FILE_NAME_BYTE "calibrationData/projectorResponseI2PForByte.dat"
#define PROJECTOR_RESPONSE_P2I_FILE_NAME_BYTE "calibrationData/projectorResponseP2IForByte.dat"
#define PROJECTOR_RESPONSE_AT_SOME_POINT_I2P_FILE_NAME "calibrationData/projectorResponseAtSomePointI2P.dat"
#define PROJECTOR_RESPONSE_AT_SOME_POINT_P2I_FILE_NAME "calibrationData/projectorResponseAtSomePointP2I.dat"
#define PROJECTOR_ALL_C_IMAGES_FILE_NAME "calibrationData/allCImages.dat"
#define WINDOW_NAME "projection image"

// スリープ時間(ms)
const int SLEEP_TIME = 50;
const int CAPTURE_NUM = 10;

const uchar INIT_RES_NUM = 0;   // 応答特性の初期値

// 先攻宣言
class GeometricCalibration;
class LinearizerOfProjector;

class ProCam{
private:
    DCam m_dcam;                        // カメラ情報
    cv::Size m_cameraSize;              // カメラ画像の大きさ ok
    cv::Size m_projectorSize;           // プロジェクタ画像の大きさ ok
    cv::VideoCapture m_video;           // カメラ映像のストリーム ok
    cv::Mat_<cv::Vec2i> m_accessMapCam2Prj; // カメラからプロジェクタへの位置マップ
    cv::Mat m_ColorTransMatCam2Pro;     // カメラ色空間からプロジェクタ色空間への変換行列
    double* m_cameraResponse;           // カメラの応答特性[0-1]->[0-1]
    cv::Mat_<cv::Vec3b> m_projectorResponseI2P;    // プロジェクタの応答特性[0-255]
    cv::Mat_<cv::Vec3b> m_projectorResponseP2I;    // プロジェクタの応答特性のインバース[0-255]
    
    ProCam(const ProCam& _procam);      // コピーコンストラクタ隠し（プログラムで１つしか存在しない為）
public:
    ///////////////////////////////  constructor ///////////////////////////////
    ProCam(void);
    ProCam(const cv::Size& projectorSize);
    ProCam(const int _width, const int _height);
    ProCam(const int _size);
    ///////////////////////////////  destructor ///////////////////////////////
    ~ProCam(void);
    ///////////////////////////////  init method ///////////////////////////////
    bool init(const cv::Size& projectorSize);
    bool init(const int _width, const int _height);
    bool init(const int _size);
    bool init(void);
    bool initCamera(void);
    bool initDCam(void);
    bool initCameraSize(void);
    bool initCameraSizeOfDCam1394(void);
    bool initProjectorSize(const cv::Size& projectorSize);
    bool initVideoCapture(void);
    bool initAccessMapCam2Prj(void);
    bool initCameraResponse(const int camResSize);
    bool initProjectorResponseI2P(void);
    bool initProjectorResponseP2I(void);
    bool initProjectorResponse(cv::Mat* const _prjResP2I);
    ///////////////////////////////  set method ///////////////////////////////
    bool setCameraSize(const cv::Size& cameraSize);
    bool setProjectorSize(const cv::Size& projectorSize);
    bool setAccessMapCam2Prj(const cv::Mat_<cv::Vec2i>& _accessMapCam2Prj);
    bool setCameraResponse(const double* const camRes, const int camResSize);
    bool setProjectorResponse(const cv::Mat_<cv::Vec3b>& _response);
    bool setProjectorResponseP2I(const cv::Mat_<cv::Vec3b>& _response);
    ///////////////////////////////  get method ///////////////////////////////
    DCam getDCam(void);
    cv::Size* getCameraSize(void);
    int getPixelsOfCamera(void);
    cv::Size* getProjectorSize(void);
    bool getCaptureImage(cv::Mat* const _image);
    cv::VideoCapture* getVideoCapture(void);
    const cv::Mat_<cv::Vec2i>* getAccessMapCam2Prj(void);
    const cv::Mat_<cv::Vec3b>* getProjectorResponseI2P(void);
    const cv::Mat_<cv::Vec3b>* getProjectorResponseP2I(void);
    void getImageProjectorResponseP2I(cv::Mat* const _responseImage, const cv::Mat& _responseMap, const int _index);
    void getImageProjectorResponseP2I(cv::Mat* const _responseImage, const int _index);
    ///////////////////////////////  save method ///////////////////////////////
//    bool saveAccessMapCam2Pro(void);
    bool saveAccessMapCam2Prj(void);
    bool saveProjectorResponseForByte(const char* fileName);
    bool saveProjectorResponseP2IForByte(const char* fileName);
    bool savePrintProjectorResponse(const char* fileName, const cv::Point& _pt, const cv::Mat& _prjRes);
    bool savePrintProjectorResponseI2P(const char* fileName, const cv::Point& _pt);
    bool savePrintProjectorResponseP2I(const char* fileName, const cv::Point& _pt);
    ///////////////////////////////  load method ///////////////////////////////
//    bool loadAccessMapCam2Pro(void);
    bool loadAccessMapCam2Prj(void);
    bool loadProjectorResponseForByte(const char* fileName);
    bool loadProjectorResponseP2IForByte(const char* fileName);
    ///////////////////////////////  calibration method ///////////////////////////////
    bool allCalibration(void);
    bool geometricCalibration(void);
    bool linearizeOfProjector(void);
    bool colorCalibration(void);
    ///////////////////////////////  convert method ///////////////////////////////
    bool convertProjectorDomainToCameraOne(cv::Mat* const _psImg, const cv::Mat&  _csImg);
    bool convertNonLinearImageToLinearOne(cv::Mat* const _linearImg, const cv::Mat&  _nonLinearImg);
    bool convertPtoI(cv::Mat* const _I, const cv::Mat&  _P);
//    bool convertCameraImageToProjectorOne(cv::Mat* const _prjImg, const cv::Mat&  _camImg);
    ///////////////////////////////  show method ///////////////////////////////
    bool showAccessMapCam2Prj(void);
    bool showProjectorResponseI2P(void);
    bool showProjectorResponseP2I(void);
    bool showProjectorResponse(const cv::Mat& _prjRes);
    ///////////////////////////////  print method ///////////////////////////////
    bool printProjectorResponseI2P(const cv::Point& _pt);
    bool printProjectorResponseP2I(const cv::Point& _pt);
    bool printProjectorResponse(const cv::Point& _pt, const cv::Mat& _prjRes);
    void printVideoPropaty(void);
    ///////////////////////////////  capture from light method ///////////////////////////////
    bool captureFromLight(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromFlatLight(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromFlatGrayLight(cv::Mat* const _captureImage, const uchar& _projectionNum, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromFlatLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromFlatGrayLightOnProjectorDomain(cv::Mat* const _captureImage, const uchar _projectionNumber, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLinearLight(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, const int _waitTimeNum = SLEEP_TIME);
    ///////////////////////////////  other method ///////////////////////////////
    bool interpolationProjectorResponseP2I(cv::Mat* const _prjRes);
    bool test_interpolationProjectorResponseP2I(void);
    bool checkCameraLinearity(void);
};

#endif /* defined(__cameraBase03__ProCam__) */
