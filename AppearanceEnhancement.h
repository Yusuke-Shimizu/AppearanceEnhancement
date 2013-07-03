//
//  AppearanceEnhancement.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/09.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#ifndef __cameraBase03__AppearanceEnhancement__
#define __cameraBase03__AppearanceEnhancement__

#include <fstream>
#include "ProCam.h"
#include "myOpenCV.h"

class ProCam;

class AppearanceEnhancement{
private:
    ProCam* m_procam;
    // C=K{(C_full-C_0)P + C_0 + F}に必要な行列
    cv::Mat m_C, m_P, m_K, m_F, m_Cfull, m_C0;
public:
    // constructor
    AppearanceEnhancement(void);
    // destructor
    // set method
    bool setRadiometricModel(const cv::Mat& C, const cv::Mat& P, const cv::Mat& K, const cv::Mat& F, const cv::Mat& Cfull, const cv::Mat& C0);
    bool setC(const cv::Mat& C);
    bool setP(const cv::Mat& P);
    bool setK(const cv::Mat& K);
    bool setF(const cv::Mat& F);
    bool setCfull(const cv::Mat& Cfull);
    bool setCfull(const double& red, const double& blue, const double& green);
    bool setCfull(const double& luminance);
    bool setC0(const cv::Mat& C0);
    bool setC0(const double& red, const double& blue, const double& green);
    bool setC0(const double& luminance);
    // get method
    bool getCfull(cv::Mat* const Cfull);
    bool getC0(cv::Mat* const C0);
    // init method
    bool init(void);
    bool initRadiometricModel(void);
    bool initC(void);
    bool initP(void);
    bool initK(void);
    bool initF(void);
    bool initCfull(void);
    bool initC0(void);
    // print method
//    bool printData(std::ofstream* ofs, const cv::Mat& data);
//    bool printData(std::ofstream* ofs, const int index, const cv::Mat& ansK1, const cv::Mat& ansK2, const cv::Mat& estK, const cv::Mat& ansF1, const cv::Mat& ansF2, const cv::Mat& estF);
    bool printStandardDeviationOfRadiometricModel(void);
    bool printSwitchIteratorError(void);
    bool printSimultaneousIteratorError(void);
    bool printAmanoMethod(void);
    // other method
    bool test_RadiometricModel(void);
    bool calcReflectAndAmbient(cv::Mat* const _K, cv::Mat* const _F, const cv::Mat& _P1, const cv::Mat& _C1, const cv::Mat& _P2, const cv::Mat& _C2);
    bool calcReflect(cv::Mat* const _K, const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _F);
    bool calcAmbient(cv::Mat* const _F, const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _K);
    bool calcIdealCamera(cv::Mat* const _C, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _P);
    bool calcCameraAddedNoise(cv::Mat* const _C, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _P, const double& noiseRange);
    bool calcCameraAddedFixNoise(cv::Mat* const _C, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _P, const double& noise);
    bool calcNextProjection(cv::Mat* const _P, const cv::Mat& _C, const cv::Mat& _K, const cv::Mat& _F);
    bool calcRangeOfDesireC(cv::Mat* const _rangeTop, cv::Mat* const _rangeDown, const cv::Mat& _K, const cv::Mat& _F);
    bool roundDesireC(cv::Mat* const _desireC, const cv::Mat& _K, const cv::Mat& _F);
    bool roundReflectance(cv::Mat* const _K);
    bool roundAmbient(cv::Mat* const _F);
};

#endif /* defined(__cameraBase03__AppearanceEnhancement__) */
