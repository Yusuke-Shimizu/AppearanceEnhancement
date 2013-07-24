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

const int PROJECTION_LUMINANCE_STEP = 1;

// 先攻宣言
class ProCam;

class LinearizerOfProjector{
private:
    ProCam* m_procam;
//    cv::Mat** m_aColorMixingMatrix;  // 全カメラ画素分のC=VPのV, プロジェクタからカメラへの色変換行列
//    std::vector<cv::Mat> m_aColorMixingMatrix;  // 全カメラ画素分のC=VPのV, プロジェクタからカメラへの色変換行列
    cv::Mat_<Vec9d> m_colorMixingMatrixMap;    // 全カメラ画素分のC=VPのV, プロジェクタからカメラへの色変換行列（サイズ：カメラと同じ）
    
    LinearizerOfProjector(const LinearizerOfProjector& _lop);   // コピーコンストラクタ隠し（プログラムで１つしか存在しない為）
public:
    ///////////////////////////////  constructor ///////////////////////////////
    LinearizerOfProjector(void);
    LinearizerOfProjector(ProCam* procam);
    ///////////////////////////////  destructor ///////////////////////////////
    ~LinearizerOfProjector(void);
    ///////////////////////////////  set method ///////////////////////////////
    bool setProCam(ProCam* procam);
    bool setColorMixMatMap(const cv::Mat_<Vec9d>& _aMat);
//    bool setResponseMap(cv::Mat_<cv::Vec3b>* const _responseMap, const cv::Mat_<cv::Vec3b>& _response, const int _depth, const int _maxDepth);
    bool setResponseMap(cv::Mat_<cv::Vec3b>* const _responseMapP2I, cv::Mat_<cv::Vec3b>* const _responseMapI2P, const cv::Mat_<cv::Vec3b>& _CImage, const uchar _INum);
    ///////////////////////////////  get method ///////////////////////////////
    ProCam* getProCam(void);
    const cv::Mat_<Vec9d>* getColorMixMatMap(void);
    ///////////////////////////////  init method ///////////////////////////////
    bool initColorMixingMatrixMap(const cv::Size& _cameraSize);
    ///////////////////////////////  save method ///////////////////////////////
    bool saveColorMixingMatrix(const char* fileName);
    bool saveColorMixingMatrixOfByte(const char* fileName);
    ///////////////////////////////  load method ///////////////////////////////
    bool loadColorMixingMatrix(const char* fileName);
    bool loadColorMixingMatrixOfByte(const char* fileName);
    ///////////////////////////////  other method ///////////////////////////////
    bool linearlize(cv::Mat_<cv::Vec3b>* const _responseMap, cv::Mat_<cv::Vec3b>* const _responseMapP2I);
    bool calcColorMixingMatrix(void);
    bool createVMap(const cv::Mat& _normalR2BL, const cv::Mat& _normalG2BL, const cv::Mat& _normalB2BL);
    bool calcResponseFunction(cv::Mat_<cv::Vec3b>* const _responseMap, cv::Mat_<cv::Vec3b>* const _responseMapP2I);
    bool convertCImage2PImage(cv::Mat_<cv::Vec3b>* const _PImageOnCameraDomain, const cv::Mat_<cv::Vec3b>& _CImage);
    bool calcP(cv::Vec3b* const _response, const cv::Vec3b& _C, const cv::Mat& _V);
    bool calcPByVec(cv::Vec3b* const _response, const cv::Vec3b& _C, const Vec9d& _VVec);
    bool showVMap(void);
    bool doRadiometricCompensation(const cv::Mat& _desiredImage);
    bool doRadiometricCompensation(const cv::Vec3b& _desiredColor);
    bool doRadiometricCompensation(const uchar& _desiredColorNumber);
    bool convertCameraImageToProjectorOne(cv::Mat* const _prjImg, const cv::Mat&  _camImg);
};

#endif /* defined(__cameraBase03__LinearizerOfProjector__) */
