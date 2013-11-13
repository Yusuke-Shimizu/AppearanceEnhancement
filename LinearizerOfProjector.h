//
//  LinearizerOfProjector.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/07.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#ifndef __cameraBase03__LinearizerOfProjector__
#define __cameraBase03__LinearizerOfProjector__

#include <iostream>
#include "myOpenCV.h"
#include "ProCam.h"
#include <vector>

#define CMM_MAP_FILE_NAME "calibrationData/cmmMap.dat"
#define CMM_MAP_FILE_NAME_BYTE "calibrationData/cmmMap_byte.dat"
const char ALL_C_IMAGES_FILE_NAME_BYTE[256] = "calibrationData/allCImages_byte.dat";

const int PROJECTION_LUMINANCE_STEP = 1;

// 先攻宣言
class ProCam;

class LinearizerOfProjector{
private:
    ProCam* m_procam;
//    cv::Mat** m_aColorMixingMatrix;  // 全カメラ画素分のC=VPのV, プロジェクタからカメラへの色変換行列
//    std::vector<cv::Mat> m_aColorMixingMatrix;  // 全カメラ画素分のC=VPのV, プロジェクタからカメラへの色変換行列
    // こいつはRGB色空間
    cv::Mat_<Vec9d> m_colorMixingMatrixMap;     // 全カメラ画素分のC=VPのV, プロジェクタからカメラへの色変換行列（サイズ：カメラと同じ）
    cv::Mat m_allCImages;                       // ０から２５５を投影し撮影したものを全て格納している場所
    
    LinearizerOfProjector(const LinearizerOfProjector& _lop);   // コピーコンストラクタ隠し（プログラムで１つしか存在しない為）
public:
    ///////////////////////////////  constructor ///////////////////////////////
    LinearizerOfProjector(ProCam* procam);
    ///////////////////////////////  destructor ///////////////////////////////
    ~LinearizerOfProjector(void);
    ///////////////////////////////  set method ///////////////////////////////
    bool setProCam(ProCam* procam);
    bool setColorMixMatMap(const cv::Mat_<Vec9d>& _aMat);
//    bool setResponseMap(cv::Mat_<cv::Vec3b>* const _responseMap, const cv::Mat_<cv::Vec3b>& _response, const int _depth, const int _maxDepth);
    bool setResponseMap(cv::Mat_<cv::Vec3b>* const _responseMapP2I, cv::Mat_<cv::Vec3b>* const _responseMapI2P, const cv::Mat_<cv::Vec3b>& _CImage, const uchar _INum);
    bool setAllCImages(const cv::Mat& _allCImages);
    bool setCImages(const cv::Mat& _CImage, const int _luminance);
    ///////////////////////////////  get method ///////////////////////////////
    ProCam* getProCam(void);
    const cv::Mat_<Vec9d>* getColorMixMatMap(void);
    const cv::Mat* getAllCImages(void);
    ///////////////////////////////  init method ///////////////////////////////
    bool initColorMixingMatrixMap(const cv::Size& _cameraSize);
    bool initAllCImages(void);
    ///////////////////////////////  save method ///////////////////////////////
    bool saveColorMixingMatrix(const char* _fileName);
    bool saveColorMixingMatrixOfByte(const char* _fileName);
    bool saveAllC(const char* _fileName, const cv::Point& _pt);
    bool saveAllCImages(const char* _fileName = ALL_C_IMAGES_FILE_NAME_BYTE);
    ///////////////////////////////  load method ///////////////////////////////
    bool loadColorMixingMatrixOfByte(const char* fileName);
    bool loadAllCImages(const char* _fileName = ALL_C_IMAGES_FILE_NAME_BYTE);
    ///////////////////////////////  show method ///////////////////////////////
    bool showVMap(void);
    bool showAllCImages(void);
    ///////////////////////////////  other method ///////////////////////////////
    bool doLinearlize(cv::Mat_<cv::Vec3b>* const _responseMap, cv::Mat_<cv::Vec3b>* const _responseMapP2I);
    bool calcColorMixingMatrix(void);
    bool calcMoreDetailColorMixingMatrix(void);
    bool createVMap(const cv::Mat& _normalR2BL, const cv::Mat& _normalG2BL, const cv::Mat& _normalB2BL);
    bool calcResponseFunction(cv::Mat_<cv::Vec3b>* const _responseMap, cv::Mat_<cv::Vec3b>* const _responseMapP2I);
    bool convertCImage2PImage(cv::Mat_<cv::Vec3b>* const _PImageOnCameraDomain, const cv::Mat_<cv::Vec3b>& _CImage);
    bool calcP(cv::Vec3b* const _response, const cv::Vec3b& _C, const cv::Mat& _V);
    bool calcPByVec(cv::Vec3b* const _response, const cv::Vec3b& _C, const Vec9d& _VVec);
    bool doRadiometricCompensation(const cv::Mat& _desiredImage, const int _waitTimeNum = SLEEP_TIME);
    bool doRadiometricCompensation(const cv::Vec3b& _desiredColor, const int _waitTimeNum = SLEEP_TIME);
    bool doRadiometricCompensation(const uchar& _desiredColorNumber, const int _waitTimeNum = SLEEP_TIME);
    bool convertCameraImageToProjectorOne(cv::Mat* const _prjImg, const cv::Mat&  _camImg);
};

#endif /* defined(__cameraBase03__LinearizerOfProjector__) */
