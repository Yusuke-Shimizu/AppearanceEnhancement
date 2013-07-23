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
#include "common.h"

// 定義
#define VGA_WIDTH 640
#define VGA_HEIGHT 480
#define MAC_PRJ_WIDTH 1680
#define MAC_PRJ_HEIGHT 1050
#define LINUX_PRJ_WIDTH 1024
#define LINUX_PRJ_HEIGHT 768

const cv::Size MAC_DISIPLAY_SIZE(1366, 768);
const cv::Size LINUX_DISPLAY_SIZE(1680, 1050);
const cv::Size MAC_OTHER_DISPLAY_SIZE(1680, 1050);
const cv::Size PROJECTOR_DISPLAY_SIZE(1024, 768);
const cv::Size USB_CAMERA_SIZE(2592, 1944);
const cv::Size IEEE_CAMERA_SIZE(640, 480);

const cv::Point MAC_OTHER_DISPLAY_POS(0, -1 * MAC_OTHER_DISPLAY_SIZE.height);
const cv::Point MAC_PROJECTOR_DISPLAY_POS(0, -1 * PROJECTOR_DISPLAY_SIZE.height);
const cv::Point LINUX_PROJECTOR_DISPLAY_POS(LINUX_DISPLAY_SIZE.width, 0);

#define MAC_POS_X 0
#define MAC_POS_Y -1050
#define LINUX_POS_X 1680
#define LINUX_POS_Y 0

//#define PRJ_SIZE_WIDTH VGA_WIDTH
//#define PRJ_SIZE_HEIGHT VGA_HEIGHT

#ifdef MAC
#define PRJ_SIZE_WIDTH MAC_PRJ_WIDTH
#define PRJ_SIZE_HEIGHT MAC_PRJ_HEIGHT
#define POSITION_PROJECTION_IMAGE_X MAC_POS_X
#define POSITION_PROJECTION_IMAGE_Y MAC_POS_Y
#endif

#ifdef MAC_PROJECTOR
#define PRJ_SIZE_WIDTH PROJECTOR_DISPLAY_SIZE.width
#define PRJ_SIZE_HEIGHT PROJECTOR_DISPLAY_SIZE.height
#define POSITION_PROJECTION_IMAGE_X MAC_PROJECTOR_DISPLAY_POS.x
#define POSITION_PROJECTION_IMAGE_Y MAC_PROJECTOR_DISPLAY_POS.y
#endif

#ifdef LINUX   // Linux
#define PRJ_SIZE_WIDTH LINUX_PRJ_WIDTH
#define PRJ_SIZE_HEIGHT LINUX_PRJ_HEIGHT
#define POSITION_PROJECTION_IMAGE_X LINUX_POS_X
#define POSITION_PROJECTION_IMAGE_Y LINUX_POS_Y
#endif

// response size
//#define RESPONSE_SIZE 256*3

// 幾何キャリブレーションで得たルックアップテーブルのファイル名
#define LOOK_UP_TABLE_FILE_NAME "calibrationData/lookUpTableCameraToProjector.dat"
#define PROJECTOR_RESPONSE_FILE_NAME "calibrationData/projectorResponse.dat"
#define PROJECTOR_RESPONSE_FILE_NAME_02 "calibrationData/projectorResponse02.dat"
#define PROJECTOR_RESPONSE_I2P_FILE_NAME_BYTE "calibrationData/projectorResponseI2PForByte.dat"
#define PROJECTOR_RESPONSE_P2I_FILE_NAME_BYTE "calibrationData/projectorResponseP2IForByte.dat"
#define WINDOW_NAME "projection image"

// スリープ時間(ms)
#define SLEEP_TIME 20
#define CAPTURE_NUM 10

const uchar INIT_RES_NUM = 0;   // 応答特性の初期値

// 先攻宣言
class GeometricCalibration;
class LinearizerOfProjector;

class ProCam{
private:
    cv::Size m_cameraSize;              // カメラ画像の大きさ ok
    cv::Size m_projectorSize;           // プロジェクタ画像の大きさ ok
    cv::VideoCapture m_video;           // カメラ映像のストリーム ok
    cv::Point* m_accessMapCam2Pro;      // カメラからプロジェクタへの位置マップ (Mat_<Vec2i>に変えたい。。。)
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
    bool initCameraSize(void);
    bool initVideoCapture(void);
    bool initAccessMapCam2Pro(void);
    bool initAccessMapCam2Prj(void);
    bool initCameraResponse(const int camResSize);
    bool initProjectorResponse(void);
    bool initProjectorResponseP2I(void);
    bool initProjectorResponseP2I(cv::Mat* const _prjResP2I);
    ///////////////////////////////  set method ///////////////////////////////
    bool setCameraSize(const cv::Size* const cameraSize);
    bool setProjectorSize(const cv::Size& projectorSize);
    bool setAccessMapCam2Pro(const cv::Point* const accessMapCam2Pro, const cv::Size& mapSize);
    bool setAccessMapCam2Pro(const cv::Point* const accessMapCam2Pro);
    bool setAccessMapCam2Prj(const cv::Mat_<cv::Vec2i>& _accessMapCam2Prj);
    bool setCameraResponse(const double* const camRes, const int camResSize);
    bool setProjectorResponse(const cv::Mat_<cv::Vec3b>& _response);
    bool setProjectorResponseP2I(const cv::Mat_<cv::Vec3b>& _response);
    ///////////////////////////////  get method ///////////////////////////////
    cv::Size* getCameraSize(void);
    int getPixelsOfCamera(void);
    cv::Size* getProjectorSize(void);
    bool getCaptureImage(cv::Mat* const image);
    cv::VideoCapture* getVideoCapture(void);
    bool getAccessMapCam2Pro(cv::Point* const accessMapCam2Pro, const cv::Size& mapSize);
    bool getAccessMapCam2Pro(cv::Point* const accessMapCam2Pro);
    const cv::Mat_<cv::Vec2i>* getAccessMapCam2Prj(void);
    const cv::Mat_<cv::Vec3b>* getProjectorResponse(void);
    const cv::Mat_<cv::Vec3b>* getProjectorResponseP2I(void);
    void getImageProjectorResponseP2I(cv::Mat* const _responseImage, const cv::Mat& _responseMap, const int _index);
    void getImageProjectorResponseP2I(cv::Mat* const _responseImage, const int _index);
    ///////////////////////////////  save method ///////////////////////////////
//    bool saveAccessMapCam2Pro(void);
    bool saveAccessMapCam2Prj(void);
    bool saveProjectorResponseForByte(const char* fileName);
    bool saveProjectorResponseP2IForByte(const char* fileName);
    bool savePrintProjectorResponse(const char* fileName, const cv::Point& _pt, const cv::Mat& _prjRes);
    ///////////////////////////////  load method ///////////////////////////////
//    bool loadAccessMapCam2Pro(void);
    bool loadAccessMapCam2Prj(void);
    bool loadProjectorResponseForByte(const char* fileName);
    bool loadProjectorResponseP2IForByte(const char* fileName);
    ///////////////////////////////  calibration method ///////////////////////////////
    bool allCalibration(void);
    bool geometricCalibration(void);
    bool colorCalibration(void);
    bool linearlizeOfProjector(void);
    ///////////////////////////////  convert method ///////////////////////////////
    bool convertProjectorCoordinateSystemToCameraOne(cv::Mat* const _psImg, const cv::Mat&  _csImg);
    bool convertNonLinearImageToLinearOne(cv::Mat* const _linearImg, const cv::Mat&  _nonLinearImg);
    bool convertPtoI(cv::Mat* const _I, const cv::Mat&  _P);
//    bool convertCameraImageToProjectorOne(cv::Mat* const _prjImg, const cv::Mat&  _camImg);
    ///////////////////////////////  show method ///////////////////////////////
    bool showProjectorResponseP2I(void);
    bool showProjectorResponseP2I(const cv::Mat& _prjRes);
    ///////////////////////////////  print method ///////////////////////////////
    bool printProjectorResponseP2I(const cv::Point& _pt);
    bool printProjectorResponseP2I(const cv::Point& _pt, const cv::Mat& _prjRes);
    ///////////////////////////////  capture from light method ///////////////////////////////
    bool captureFromLight(cv::Mat* const captureImage, const cv::Mat& projectionImage, const int _waitTimeNum = SLEEP_TIME);
//    bool captureFromLight(cv::Mat* const captureImage, const cv::Mat& projectionImage);
    bool captureFromNonGeometricTranslatedLight(cv::Mat* const captureImage, const cv::Mat& projectionImage);
    bool captureFromLinearLight(cv::Mat* const captureImage, const cv::Mat& projectionImage);
    ///////////////////////////////  other method ///////////////////////////////
    bool interpolationProjectorResponseP2I(cv::Mat* const _prjRes);
    bool test_interpolationProjectorResponseP2I(void);
    bool checkCameraLinearity(void);
};

#endif /* defined(__cameraBase03__ProCam__) */
