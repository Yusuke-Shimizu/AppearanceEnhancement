//
//  ProCam.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/01.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#ifndef __cameraBase03__ProCam__
#define __cameraBase03__ProCam__

// スリープ時間(ms)
const int SLEEP_TIME = 50;
const int BUFFER_NUM = 10;

#include "LinearizerOfProjector.h"
#include "myOpenCV.h"
#include "myDc1394.h"
#include "common.h"
//#include <vector>

//#define GEO_CAL_CALC_FLAG       // 幾何変換を計算するフラグ

//#define SHOW_GEOMETRIC_CALIBRATION_MAP_FLAG // 幾何変換のマップを表示するフラグ
//#define SHOW_LINEAR_FLAG                    // プロジェクタ線形化を表示するフラグ

//#define BAYER_FLAG
//#define LIB_DC1394_FLAG

const bool COLOR_CALIB_2003 = false;
const bool COLOR_CALIB_2004 = !COLOR_CALIB_2003;

// 定義
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
#ifdef LINUX   // Linux
const cv::Size PRJ_SIZE(MAC_OTHER_DISPLAY_SIZE);
const cv::Point POSITION_OF_PROJECTION_IMAGE(MAC_OTHER_DISPLAY_POS);
//const cv::Size PRJ_SIZE(PROJECTOR_DISPLAY_SIZE);
//const cv::Point POSITION_OF_PROJECTION_IMAGE(LINUX_PROJECTOR_DISPLAY_POS);
#endif

// 幾何キャリブレーションで得たルックアップテーブルのファイル名
#define LOOK_UP_TABLE_FILE_NAME "calibrationData/lookUpTableCameraToProjector.dat"  // 幾何変換表（カメラ=>プロジェクタ）
#define PROJECTOR_RESPONSE_I2P_FILE_NAME_BYTE "calibrationData/projectorResponseI2PForByte.dat"
#define PROJECTOR_RESPONSE_P2I_FILE_NAME_BYTE "calibrationData/projectorResponseP2IForByte.dat"
#define PROJECTOR_RESPONSE_AT_SOME_POINT_I2P_FILE_NAME "calibrationData/projectorResponseAtSomePointI2P.dat"
#define PROJECTOR_RESPONSE_AT_SOME_POINT_P2I_FILE_NAME "calibrationData/projectorResponseAtSomePointP2I.dat"
#define PROJECTOR_ALL_C_IMAGES_FILE_NAME "calibrationData/allCImages.dat"
#define CHECK_LINEARIZE_FILE_NAME "calibrationData/checkLinearize.dat"
//#define CHECK_COLOR_CALIBRATION_FILE_NAME "calibrationData/checkColorCalibration"
#define WINDOW_NAME "projection image"

const std::string CALIB_DATA_PATH = "calibrationData/";
const std::string COLOR_CALIB_PATH = CALIB_DATA_PATH + "colorCalibration/";
const std::string CHECK_COLOR_CALIBRATION_FILE_NAME = COLOR_CALIB_PATH + "checkColorCalibration";

const std::string I2C_DATA_PATH = CALIB_DATA_PATH + "I2C";

// 後で調整する（linear classと重複宣言してる）
const std::string PRJ_LINEAR_PATH_ = CALIB_DATA_PATH + "linearize/";
const std::string LIN_IMG_PATH_ = PRJ_LINEAR_PATH_ + "img/";
const std::string PROJECTOR_RESPONSE_C_IMAGE_PATH_ = LIN_IMG_PATH_ + "C/";
const std::string P2I_IMAGE_PATH_ = LIN_IMG_PATH_ + "P2I/";
const std::string LIN_DATA_PATH_ = PRJ_LINEAR_PATH_ + "data/";
const std::string LIN_TEST_DATA_PATH_ = LIN_DATA_PATH_ + "linearCheck.dat";

const std::string PROJECTOR_RESPONSE_P2I_IMAGE_PATH = LIN_IMG_PATH_ + "P2I/";
const std::string P2I_DATA_PATH = LIN_DATA_PATH_ + "P2I.dat";

const uchar INIT_RES_NUM = 0;   // 応答特性の初期値

const cv::Vec3b g_maxPrjLuminance(220, 230, 230);
const cv::Vec3b g_minPrjLuminance(0, 0, 0);

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
    cv::Mat m_projectorResponseI2P;     // プロジェクタの応答特性[0-255]
    cv::Mat m_projectorResponseP2I;     // プロジェクタの応答特性のインバース[0-255]
    cv::Vec3b* m_simpleProjectorResponseI2P;    // 簡易的なプロジェクタ応答特性[0-255]
    cv::Vec3b* m_simpleProjectorResponseP2I;    // 簡易的なプロジェクタ応答特性[0-255]
    
    // こっちはBGR色空間にしてる
    cv::Mat_<Vec9d> m_V;    // 全カメラ画素分のV（プロジェクタからカメラへの色変換行列）
    cv::Mat_<Vec9d> m_invV; // 全カメラ画素分のV（カメラからプロジェクタへの色変換行列）
    cv::Mat m_F;             // 環境光の全画素分
    
    // カラーキャリブレーションに必要な色のリスト
    std::vector<cv::Vec3b> m_smallColorList;
    std::vector<cv::Vec3b> m_normalColorList;
    std::vector<cv::Vec3b> m_largeColorList;
    
    // カメラキャプチャのデノイジングを行うかどうかのフラグ
    bool m_bDenoise;
//    LinearizerOfProjector m_linearPrj;
    
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
    bool initProjectorResponse(cv::Mat* const _src);
    bool initSimpleProjectorResponseI2P(void);
    bool initSimpleProjectorResponseP2I(void);
    bool initSimpleProjectorResponse(cv::Vec3b* _src);
    bool initV(void);
    void initF(void);
    bool initColorList(void);
    ///////////////////////////////  set method ///////////////////////////////
    bool setCameraSize(const cv::Size& cameraSize);
    bool setProjectorSize(const cv::Size& projectorSize);
    bool setAccessMapCam2Prj(const cv::Mat_<cv::Vec2i>& _accessMapCam2Prj);
    bool setProjectorResponse(const cv::Mat_<cv::Vec3b>& _response);
    bool setProjectorResponseP2I(const cv::Mat_<cv::Vec3b>& _response);
    bool setSimpleProjectorResponseP2I(const cv::Vec3b* const _src, const int _length = 256);
    bool setSimpleProjectorResponseI2P(const cv::Vec3b* const _src, const int _length = 256);
    bool setSimpleProjectorResponseP2I(const cv::Vec3b _num, const int _index);
    bool setSimpleProjectorResponseI2P(const cv::Vec3b _num, const int _index);
    bool setSimpleProjectorResponseP2I(const uchar _num, const int _index, const int _channel);
    bool setSimpleProjectorResponseI2P(const uchar _num, const int _index, const int _channel);
    bool setImageProjectorResponseP2I(cv::Mat* const _responseMap, const cv::Mat& _responseImage, const int _index);
    bool setImageProjectorResponseP2I(const cv::Mat& _responseImage, const int _index);
    bool setV(const cv::Mat& _diffBB, const cv::Mat& _diffGB, const cv::Mat& _diffRB);
    bool setV(const std::vector<cv::Mat>& _vecC, const std::vector<cv::Vec3d>& _vecP);
    void setF(const cv::Mat& _P);
    bool switchDenoiseFlag(void);
    bool switchOnDenoiseFlag(void);
    bool switchOffDenoiseFlag(void);
    ///////////////////////////////  get method ///////////////////////////////
    DCam getDCam(void);
    cv::Size* getCameraSize(void);
    const cv::Size& getCameraSize_(void);
    int getPixelsOfCamera(void);
    cv::Size* getProjectorSize(void);
    const cv::Size& getProjectorSize_(void);
    bool getCaptureImage(cv::Mat* const _image);
    cv::VideoCapture* getVideoCapture(void);
    const cv::Mat_<cv::Vec2i>* getAccessMapCam2Prj(void);
    void getPointOnPrjDomainFromPointOnCamDomain(cv::Point* const _prjPoint, const cv::Point& _camPoint);
    const cv::Mat* getProjectorResponseI2P(void);
    const cv::Mat* getProjectorResponseP2I(void);
    const cv::Vec3b* getSimpleProjectorResponseI2P(void);
    const cv::Vec3b* getSimpleProjectorResponseP2I(void);
    const cv::Vec3b getSimpleProjectorResponseI2P(const int _index);
    const cv::Vec3b getSimpleProjectorResponseP2I(const int _index);
    const uchar getSimpleProjectorResponseI2P(const int _index, const int _channel);
    const uchar getSimpleProjectorResponseP2I(const int _index, const int _channel);
    void getImageProjectorResponseP2I(cv::Mat* const _responseImage, const cv::Mat& _responseMap, const int _index);
    void getImageProjectorResponseP2I(cv::Mat* const _responseImage, const int _index);
    const cv::Mat_<Vec9d>* getV(void);
    const cv::Mat_<Vec9d>* getInvV(void);
    const cv::Mat* getF(void);
    void getNextProjectionImage(cv::Mat* const _P, const cv::Mat& _C);
    ///////////////////////////////  save method ///////////////////////////////
//    bool saveAccessMapCam2Pro(void);
    bool saveAccessMapCam2Prj(void);
    bool saveProjectorResponseForByte(const char* fileName);
    bool saveProjectorResponseP2IForByte(const char* fileName);
    bool savePrintProjectorResponse(const char* fileName, const cv::Point& _pt, const cv::Mat& _prjRes);
    bool savePrintProjectorResponseI2P(const char* fileName, const cv::Point& _pt);
    bool savePrintProjectorResponseP2I(const char* fileName, const cv::Point& _pt);
    bool saveRelationP2I(void);
    bool saveRelationI2C(void);
    ///////////////////////////////  load method ///////////////////////////////
//    bool loadAccessMapCam2Pro(void);
    bool loadAccessMapCam2Prj(void);
    bool loadProjectorResponseForByte(const char* fileName);
    bool loadProjectorResponseP2IForByte(const char* fileName);
    ///////////////////////////////  calibration method ///////////////////////////////
    bool allCalibration(void);
    bool geometricCalibration(void);
    bool linearizeOfProjector(const bool _calcLinearFlag, const bool _showLinearFlag = false);
    bool test_linearizeOfProjector(void);
    bool colorCalibration(const bool _denoisingFlag = false);
    bool colorCalibration2(cv::Mat_<Vec12d>* const _V);
    bool colorCalibration3(const bool _denoisingFlag = false);
    bool test_colorCalibration(void);
    ///////////////////////////////  convert method ///////////////////////////////
    bool convertProjectorDomainToCameraOne(cv::Mat* const _psImg, const cv::Mat&  _csImg);
    bool convertPtoI(cv::Mat* const _I, const cv::Mat&  _P);
    bool convertToSimpleLinearizedImage(cv::Mat* const _linearizedImage, const cv::Mat&  _nonLinearizedImage, const cv::Vec3b* const _projectorResponse);
    bool test_convertToSimpleLinearizedImage(void);
    void convertColorSpace(cv::Mat* const _imageOnPrj, const cv::Mat& _imageOnCam, const bool P2CFlag);
    void convertColorSpaceOfCameraToProjector(cv::Mat* const _imageOnPrj, const cv::Mat& _imageOnCam);
    void convertColorSpaceOfProjectorToCamera(cv::Mat* const _imageOnCam, const cv::Mat& _imageOnPrj);
    ///////////////////////////////  show method ///////////////////////////////
    bool showAccessMapCam2Prj(void);
    bool showProjectorResponseI2P(void);
    bool showProjectorResponseP2I(void);
    bool showProjectorResponse(const cv::Mat& _prjRes);
    void showV(void);
    ///////////////////////////////  print method ///////////////////////////////
    bool printProjectorResponseI2P(const cv::Point& _pt);
    bool printProjectorResponseP2I(const cv::Point& _pt);
    bool printProjectorResponse(const cv::Point& _pt, const cv::Mat& _prjRes);
    ///////////////////////////////  capture from light method ///////////////////////////////

    // normal method
    bool captureFromLight(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLight(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLight(cv::Mat* const _captureImage, const uchar& _projectionNum, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLight(cv::Vec3b* const _captureColor, const cv::Mat& _projectionImage, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLight(cv::Vec3b* const _captureColor, const cv::Vec3b& _projectionColor, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLight(cv::Vec3b* const _captureColor, const uchar _projectionNum, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    void captureFromLightTest(void);
    bool captureFromLight(std::vector<cv::Mat>* const _captureImages, const std::vector<cv::Mat>& _projectionImage, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    
    
    bool captureFromLightOnProjectorDomain(cv::Mat* const _captureImage,    const cv::Mat& _projectionImage,   const bool _denoiseFlag = false,    const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLightOnProjectorDomain(cv::Mat* const _captureImage,    const cv::Vec3b& _projectionColor, const bool _denoiseFlag = false,    const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLightOnProjectorDomain(cv::Mat* const _captureImage,    const uchar _projectionNumber,     const bool _denoiseFlag = false,    const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLightOnProjectorDomain(cv::Vec3b* const _captureColor,    const cv::Mat& _projectionImage,     const bool _denoiseFlag = false,    const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLightOnProjectorDomain(cv::Vec3b* const _captureColor,    const cv::Vec3b& _projectionColor,     const bool _denoiseFlag = false,    const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLightOnProjectorDomain(cv::Vec3b* const _captureColor,    const uchar _projectionNumber,     const bool _denoiseFlag = false,    const int _waitTimeNum = SLEEP_TIME);
    
    bool captureFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const uchar _projectionNumber, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    
    bool captureOfProjecctorColorFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    bool captureOfProjecctorColorFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    bool captureOfProjecctorColorFromLinearLightOnProjectorDomain(cv::Mat* const _captureImage, const uchar _projectionNumber, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    
    bool captureFromLinearLightOnProjectorDomainAndColor(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLinearLightOnProjectorDomainAndColor(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    bool captureFromLinearLightOnProjectorDomainAndColor(cv::Mat* const _captureImage, const uchar _projectionNumber, const bool _denoiseFlag = false, const int _waitTimeNum = SLEEP_TIME);
    ///////////////////////////////  other method ///////////////////////////////
    bool checkCameraLinearity(void);
    bool doRadiometricCompensation(const cv::Mat& _desiredImage, const int _waitTimeNum = SLEEP_TIME);
    bool doRadiometricCompensation(const cv::Vec3b& _desiredColor, const int _waitTimeNum = SLEEP_TIME);
    bool doRadiometricCompensation(const uchar& _desiredColorNumber, const int _waitTimeNum = SLEEP_TIME);
    bool doRadiometricCompensation(const char* _fileName, const int _waitTimeNum = SLEEP_TIME);
    void reconstructC(cv::Mat* const _C, const cv::Mat& _P);
    bool doRadiometricCompensationByYoshidaMethod(const cv::Mat& _desiredImage, const int _waitTimeNum = SLEEP_TIME);
    bool doRadiometricCompensationByAmanoMethod(const cv::Mat& _desiredImage, const int _waitTimeNum = SLEEP_TIME);
    void getProjectionImage(cv::Mat* const _P, const cv::Mat& _desireC, const cv::Mat_<Vec12d>& _K);
    bool medianBlurForProjectorResponseP2I(cv::Mat* const _dst, const cv::Mat& _src);
    bool test_medianBlurForProjectorResponseP2I(void);
    bool settingProjectorAndCamera(void);
};

#endif /* defined(__cameraBase03__ProCam__) */
