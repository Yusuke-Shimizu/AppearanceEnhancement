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

#define RESPONSE_SIZE 256*3

// 幾何キャリブレーションで得たルックアップテーブルのファイル名
#define LOOK_UP_TABLE_FILE_NAME "caliblationData/lookUpTableCameraToProjector.dat"
#define PROJECTOR_RESPONSE_FILE_NAME "caliblationData/projectorResponse.dat"
#define PROJECTOR_RESPONSE_FILE_NAME_02 "caliblationData/projectorResponse02.dat"
#define WINDOW_NAME "projection image"

// スリープ時間(ms)
#define SLEEP_TIME 10
#define CAPTURE_NUM 10

class LinearizerOfProjector;

class ProCam{
private:
    cv::Size m_cameraSize;              // カメラ画像の大きさ ok
    cv::Size m_projectorSize;           // プロジェクタ画像の大きさ ok
    cv::VideoCapture m_video;           // カメラ映像のストリーム ok
    cv::Point* m_accessMapCam2Pro;      // カメラからプロジェクタへの位置マップ ポインタのポインタの方がいいのかな？
    cv::Mat m_ColorTransMatCam2Pro;     // カメラ色空間からプロジェクタ色空間への変換行列
    double* m_cameraResponse;           // カメラの応答特性[0-1]->[0-1]
//    double* m_projectorResponse;        // プロジェクタの応答特性[0-1]->[0-1]
//    char* m_projectorResponse;        // プロジェクタの応答特性[0-1]->[0-1]
    cv::Mat_<cv::Vec3b> m_projectorResponse;    // プロジェクタの応答特性[0-255]
//    int m_cameraResponseSize;           // カメラ応答特性の大きさ
//    int m_projectorResponseSize;        // プロジェクタ応答特性の大きさ
    
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
    bool initCameraResponse(const int camResSize);
    bool initProjectorResponse(void);
    ///////////////////////////////  set method ///////////////////////////////
    bool setCameraSize(const cv::Size* const cameraSize);
    bool setProjectorSize(const cv::Size& projectorSize);
    bool setAccessMapCam2Pro(const cv::Point* const accessMapCam2Pro, const cv::Size& mapSize);
    bool setAccessMapCam2Pro(const cv::Point* const accessMapCam2Pro);
//    bool setCameraResponseSize(const int camResSize);
//    bool setProjectorResponseSize(const int prjResSize);
    bool setCameraResponse(const double* const camRes, const int camResSize);
    bool setProjectorResponse(const cv::Mat_<cv::Vec3b>* const _response);
    ///////////////////////////////  get method ///////////////////////////////
    bool getCameraSize(cv::Size* const cameraSize);
    cv::Size* getCameraSize(void);
    int getPixelsOfCamera(void);
    bool getProjectorSize(cv::Size* const projectorSize);
    cv::Size* getProjectorSize(void);
    bool getCaptureImage(cv::Mat* const image);
    bool getVideoCapture(cv::VideoCapture* _video);
    cv::VideoCapture* getVideoCapture(void);
    bool getAccessMapCam2Pro(cv::Point* const accessMapCam2Pro, const cv::Size& mapSize);
    bool getAccessMapCam2Pro(cv::Point* const accessMapCam2Pro);
//    int getCameraResponseSize(void);
//    int getProjectorResponseSize(void);
    ///////////////////////////////  save method ///////////////////////////////
    bool saveAccessMapCam2Pro(void);
    bool saveProjectorResponse(const char* fileName);
    bool saveProjectorResponse(const char* fileName, const uchar index, const uchar color);
    ///////////////////////////////  load method ///////////////////////////////
    bool loadAccessMapCam2Pro(void);
    ///////////////////////////////  calibration method ///////////////////////////////
    bool allCalibration(void);
    bool geometricCalibration(void);
    bool colorCalibration(void);
    bool linearlizeOfProjector(void);
    ///////////////////////////////  other method ///////////////////////////////
    bool captureFromLight(cv::Mat* const captureImage, const cv::Mat& projectionImage);
};

#endif /* defined(__cameraBase03__ProCam__) */
