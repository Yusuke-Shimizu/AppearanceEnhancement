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

#define RESPONSE_SIZE 256
class LinearizerOfProjector;

class ProCam{
private:
    cv::Size m_cameraSize;              // カメラ画像の大きさ ok
    cv::Size m_projectorSize;           // プロジェクタ画像の大きさ ok
    cv::VideoCapture m_video;           // カメラ映像のストリーム ok
    cv::Point* m_accessMapCam2Pro;      // カメラからプロジェクタへの位置マップ ポインタのポインタの方がいいのかな？
    cv::Mat m_ColorTransMatCam2Pro;     // カメラ色空間からプロジェクタ色空間への変換行列
    double* m_cameraResponse;           // カメラの応答特性[0-1]->[0-1]
    double* m_projectorResponse;        // プロジェクタの応答特性[0-1]->[0-1]
    int m_cameraResponseSize;           // カメラ応答特性の大きさ
    int m_projectorResponseSize;        // プロジェクタ応答特性の大きさ
    
    LinearizerOfProjector* linearPrj;   // プロジェクタ線形化を行うインスタンス
    
public:
    // constructor
    ProCam(const cv::Size* const projectorSize);
    // destractor
    ~ProCam(void);
    // set method
    bool setCameraSize(const cv::Size* const cameraSize);
    bool setProjectorSize(const cv::Size* const projectorSize);
    bool setAccessMapCam2Pro(const cv::Point* const accessMapCam2Pro, const cv::Size* const mapSize);
    bool setCameraResponseSize(const int camResSize);
    bool setProjectorResponseSize(const int prjResSize);
    bool setCameraResponse(const double* const camRes, const int camResSize);
    bool setProjectorResponse(const double* const prjRes, const int prjResSize);
    // get method
    bool getCameraSize(cv::Size* const cameraSize);
    bool getProjectorSize(cv::Size* const projectorSize);
    bool getCaptureImage(cv::Mat* const image);
    bool getAccessMapCam2Pro(cv::Point* const accessMapCam2Pro, const cv::Size* const mapSize);
    int getCameraResponseSize(void);
    int getProjectorResponseSize(void);
    // init method
    bool init(const cv::Size* const projectorSize);
    bool initCameraSize(void);
    bool initVideoCapture(void);
    bool initAccessMapCam2Pro(void);
    bool initCameraResponseSize(void);
    bool initProjectorResponseSize(void);
    bool initCameraResponse(const int camResSize);
    bool initProjectorResponse(const int prjResSize);
    // other method
    bool captureFromLight(cv::Mat* const captureImage, const cv::Mat* const projectionImage);
    bool linearlizeOfProjector(void);
};

#endif /* defined(__cameraBase03__ProCam__) */
