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
#include <string>
#include "ProCam.h"
#include "myOpenCV.h"

const double NOISE_RANGE = 5 / 255.0;
//#define NOISE_RANGE 0.05

const std::string OUTPUT_DATA_PATH = "data/";
const std::string C_FULL_FILE_NAME = "data/cfull.png";
const std::string C_0_FILE_NAME = "data/c0.png";
const std::string K_FILE_NAME = "data/estK.png";
const std::string F_FILE_NAME = "data/estF.png";
const std::string C_FILE_NAME = "data/C.png";
const std::string P_FILE_NAME = "data/P.png";
const std::string TARGET_FILE_NAME = "data/target.png";
const std::string ESTIMATE_K_FILE_NAME = "calibrationData/estimate/estimateK.dat";
const std::string ESTIMATE_KF_FILE_NAME = "calibrationData/estimate/estimateKF.dat";
const std::string TEST_PROJECTION_FILE_NAME = "calibrationData/projection/projection.dat";
const std::string SIM_ESTIMATE_K_FILE_NAME = "simulationData/estimate/estimateK.dat";
const std::string SIM_ESTIMATE_KF_FILE_NAME = "simulationData/estimate/estimateKF.dat";
const std::string SIM_PROJECTION_FILE_NAME = "simulationData/projection/projection.dat";
const std::string CHECK_CMAX_MIN_FILE_NAME = "calibrationData/checkCMax_CMin";

enum mode {
    e_Amano,
    e_Fujii,
    e_Shimizu
};

class ProCam;

class AppearanceEnhancement{
private:
    ProCam m_procam;
    // simulation data
    cv::Mat m_C, m_P, m_K, m_F, m_Cfull, m_C0;
    // experimental data
    cv::Mat m_KMap, m_FMap, m_CfullMap, m_C0Map;
    mode m_currentMode;
    
    AppearanceEnhancement(const AppearanceEnhancement& ae); // コピーコンストラクタ隠し（プログラムで１つしか存在しない為）
public:
    ///////////////////////////////  constructor ///////////////////////////////
    // using simulation
    AppearanceEnhancement(const double& _CMaxNum, const double& _CMinNum);
    // using experiment
    AppearanceEnhancement(const cv::Size& _prjSize);
    ///////////////////////////////  destructor ///////////////////////////////
    ///////////////////////////////  init method ///////////////////////////////
    bool init(void);
    bool initRadiometricModel(void);
    bool initK(const cv::Size& _camSize);
    bool initF(const cv::Size& _camSize);
    bool initCfull(const cv::Size& _camSize);
    bool initC0(const cv::Size& _camSize);
    bool initProCam(void);
    ///////////////////////////////  set method ///////////////////////////////
    bool setKMap(const cv::Mat& _K);
    bool setFMap(const cv::Mat& _F);
    bool setCfullMap(const bool _denoisingFlag = false);
    bool setCfullMap(const cv::Mat& _Cfull);
    bool setC0Map(const bool _denoisingFlag = false);
    bool setC0Map(const cv::Mat& _C0);
    ///////////////////////////////  get method ///////////////////////////////
    bool getCfull(cv::Mat* const Cfull);
    const cv::Mat& getCfullMap(void);
    bool getC0(cv::Mat* const C0);
    const cv::Mat& getC0Map(void);
    const cv::Mat& getKMap(void);
    const cv::Mat& getFMap(void);
    ProCam* getProCam(void);
    bool switchMode(void);
    bool isAmanoMode(void) const;
    ///////////////////////////////  print method ///////////////////////////////
    bool printStandardDeviationOfRadiometricModel(void);
    bool printSwitchIteratorError(void);
    bool printSimultaneousIteratorError(void);
    bool printAmanoMethod(void);
    bool printAppearanceEnhancement(void);
    ///////////////////////////////  save method ///////////////////////////////
    bool saveCfull(const std::string& _fileName = C_FULL_FILE_NAME);
    bool saveC0(const std::string& _fileName = C_0_FILE_NAME);
    bool saveK(const std::string& _fileName = K_FILE_NAME);
    bool saveF(const std::string& _fileName = F_FILE_NAME);
    bool saveAll(const cv::Mat& _C, const cv::Mat& _P, const cv::Mat& _targetC, const std::string& _fileNameC = C_FILE_NAME, const std::string& _fileNameP = P_FILE_NAME, const std::string& _fileNameTarget = TARGET_FILE_NAME);
    ///////////////////////////////  load method ///////////////////////////////
    bool loadCfull(const std::string& _fileName = C_FULL_FILE_NAME);
    bool loadC0(const std::string& _fileName = C_0_FILE_NAME);
    ///////////////////////////////  calc method ///////////////////////////////
    bool calcReflectAndAmbient(cv::Mat* const _K, cv::Mat* const _F, const cv::Mat& _P1, const cv::Mat& _C1, const cv::Mat& _P2, const cv::Mat& _C2);
    bool calcReflect(cv::Mat* const _K, const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _F);
    bool calcAmbient(cv::Mat* const _F, const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _K);
    bool calcIdealCamera(cv::Mat* const _C, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _P);
    bool calcCameraAddedNoise(cv::Mat* const _C, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _P, const double& noiseRange);
    bool calcCameraAddedFixNoise(cv::Mat* const _C, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _P, const double& noise);
    bool calcNextProjection(cv::Mat* const _P, const cv::Mat& _C, const cv::Mat& _K, const cv::Mat& _F);
    bool calcRangeOfDesireC(cv::Mat* const _rangeTop, cv::Mat* const _rangeDown, const cv::Mat& _K, const cv::Mat& _F);
    bool calcTargetImage(cv::Mat* const _targetImage, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _CMin, const double& _s = 1.3, const int _enhanceType = 0);
    bool calcTargetImageAtPixel(double* const _targetImage, const double& _K, const double& _F, const double& _CMin, const double& _KGray, const double& _s = 1.3, const int _enhanceType = 0);
    bool calcNextProjectionImage(cv::Mat* const _nextP, cv::Mat* const _error, cv::Mat* const _Cr, const cv::Mat& _targetImage, const cv::Mat& _targetImageBefore, const cv::Mat& _C, const cv::Mat& _P, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _FBefore, const cv::Mat& _CMax, const cv::Mat& _CMin, const double& _alpha = 0.1);
    bool test_calcNextProjectionImage(const cv::Mat& _answerK, const cv::Mat& _answerF, const cv::Mat& _CMax, const cv::Mat& _CMin, const cv::Scalar& _mask = cv::Scalar(1, 1, 1, 0));
    bool calcNextProjectionImageAtPixel(uchar* const _nextP, double* const _error, double* const _Cr, const double& _targetImage, const double& _targetImageBefore, const double& _C, const double& _P, const double& _K, const double& _F, const double& _FBefore, const double& _Cfull, const double& _C0, const double& _PMax, const double& _PMin, const double& _alpha = 0.1);
    bool test_calcNextProjectionImageAtPixel(void);
    bool calcReflectanceAtPixel(double* const _K, const double& _nC, const double& _nP, const double& _nCMax, const double& _nCMin, const double& _nPMax, const double& _nPMin);
    bool test_calcReflectanceAtPixel(void);
    bool calcReflectanceAndAmbientLightAtPixel(double* const _K, double* const _F, const double& _nC1, const double& _nP1, const double& _nC2, const double& _nP2, const double& _nCMax, const double& _nCMin, const double& _nPMax, const double& _nPMin);
    bool test_calcReflectanceAndAmbientLightAtPixel(void);
    bool calcCaptureImageAddNoise(double* const _C, const double& _P, const double& _K, const double& _F, const double& _CMax, const double& _CMin, const double& _noiseRange = NOISE_RANGE);
    bool calcVirtualC(cv::Mat* const _vrC, const cv::Mat& _P);
    ////////////////////////////// estimate method //////////////////////////////
    bool estimateK(const cv::Mat& _P);
    bool estimateK(const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _CMax, const cv::Mat& _CMin, const cv::Mat& _F);
    bool test_estimateK(const cv::Mat& _answerK, const cv::Mat& _CMax, const cv::Mat& _CMin, const cv::Scalar& _mask = cv::Scalar(1, 1, 1, 0));
    bool estimateF(const cv::Mat& _P);
    bool estimateF(const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _CMax, const cv::Mat& _CMin);
    bool estimateKFByAmanoModel(const cv::Mat& _P1, const cv::Mat& _P2);
    bool estimateKFByAmanoModel(const cv::Mat& _P1, const cv::Mat& _P2, const cv::Mat& _C1, const cv::Mat& _C2);
    bool test_estimateKFByAmanoModel(const cv::Mat& _answerK, const cv::Scalar& _mask = cv::Scalar(1, 1, 1, 0));
    bool estimateKFByFujiiModel(const cv::Mat& _P1, const cv::Mat& _P2);
    ////////////////////////////// evaluate method //////////////////////////////
    bool evaluateEstimate(const cv::Mat& _C, const cv::Mat& _P, const int num);
    bool evaluateEstimationAndProjection(const cv::Mat& _ansK, const cv::Mat& _estK, const cv::Mat& _ansF, const cv::Mat& _estF, const cv::Mat& _targetImage, const cv::Mat& _captureImage);
    ///////////////////////////////  round method ///////////////////////////////
    bool roundDesireC(cv::Mat* const _desireC, const cv::Mat& _K, const cv::Mat& _F);
    bool roundReflectance(cv::Mat* const _K);
    bool roundAmbient(cv::Mat* const _F);
    ///////////////////////////////  show method ///////////////////////////////
    bool showKMap(void);
    bool showFMap(void);
    bool showCfullMap(void);
    bool showC0Map(void);
    bool showAll(const cv::Mat& _captureImage, const cv::Mat& _projectionImage, const cv::Mat& _targetImage, const cv::Mat& _answerK, const cv::Mat& _answerF, const cv::Mat& _errorOfMPC, const cv::Mat& _CrOfMPC);
    ///////////////////////////////  other method ///////////////////////////////
    bool test_RadiometricModel(void);
    bool test_CMaxMin(const cv::Mat& _CMax, const cv::Mat& _CMin);
    bool doAppearanceCrealy(cv::Mat* const _P, const double _s);
    bool doAppearanceEnhancementByAmano(void);
};

#endif /* defined(__cameraBase03__AppearanceEnhancement__) */
