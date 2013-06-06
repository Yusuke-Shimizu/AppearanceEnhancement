//
//  ProCam.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/01.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#ifndef __cameraBase03__ProCam__
#define __cameraBase03__ProCam__

#include "myOpenCV.h"

class ProCam{
private:
    cv::Size m_cameraSize;            // カメラ画像の大きさ ok
    cv::Size m_projectorSize;         // プロジェクタ画像の大きさ ok
    cv::VideoCapture m_video;         // カメラ映像のストリーム ok
    cv::Point* m_accessMapCam2Pro;    // カメラからプロジェクタへの位置マップ
    cv::Mat m_ColorTransMatCam2Pro;   // カメラ色空間からプロジェクタ色空間への変換行列
    double* m_cameraResponse;         // カメラの応答特性[0-1]->[0-1]
    double* m_projectorResponse;      // プロジェクタの応答特性[0-1]->[0-1]
    
public:
    // constructor
    ProCam(const cv::Size* const cameraSize, const cv::Size* const projectorSize, const int cameraBitDepth, const int projectorBitDepth);
    // set method
    bool setCameraSize(const cv::Size* const cameraSize);
    bool setProjectorSize(const cv::Size* const projectorSize);
    bool setAccessMapCam2Pro(const cv::Point* const accessMapCam2Pro, const cv::Size* const mapSize);
    // get method
    bool getCameraSize(cv::Size* const cameraSize);
    bool getProjectorSize(cv::Size* const projectorSize);
    bool getCaptureImage(cv::Mat* const image);
    bool getAccessMapCam2Pro(cv::Point* const accessMapCam2Pro, const cv::Size* const mapSize);
    // init method
    bool init(const cv::Size* const cameraSize, const cv::Size* const projectorSize, const int cameraBitDepth, const int projectorBitDepth);
    bool initCameraSize(void);
    bool initVideoCapture(void);
    bool initAccessMapCam2Pro(cv::Point* const accessMapCam2Pro, const cv::Size* const mapSize);
    // other method
    bool captureFromLight(cv::Mat* const captureImage, const cv::Mat* const projectionImage);
};

#endif /* defined(__cameraBase03__ProCam__) */
