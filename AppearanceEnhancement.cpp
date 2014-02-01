//
//  AppearanceEnhancement.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/09.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include <ctime>
#include <cstdlib>
#include <fstream>
#include "AppearanceEnhancement.h"
#include "myOpenCV.h"
#include "common.h"
#include <vector>
#include "myMath.hpp"
//#include "gnuplot.h"

using namespace std;
using namespace cv;

///////////////////////////////  constructor ///////////////////////////////
AppearanceEnhancement::AppearanceEnhancement(const double& _CMaxNum, const double& _CMinNum)
:m_K(3, 1, CV_8UC1), m_F(3, 1, CV_8UC1), m_Cfull(3, 1, CV_8UC1), m_C0(3, 1, CV_8UC1), m_currentMode(e_Amano), m_printPoint(100, 100)
{
//    m_procam.test_medianBlurForProjectorResponseP2I();
//    m_procam.test_interpolationProjectorResponseP2I();
    srand((unsigned) time(NULL));
}
AppearanceEnhancement::AppearanceEnhancement(const cv::Size& _prjSize)
:m_procam(_prjSize), m_currentMode(e_Amano), m_printPoint(100, 100)
{
    init();
}
///////////////////////////////  denstructor ///////////////////////////////
///////////////////////////////  init method ///////////////////////////////
// 全体の初期化
bool AppearanceEnhancement::init(void){
    return initRadiometricModel();
}

// 光学モデルに必要なモデルの初期化
// return   : 成功したかどうか
bool AppearanceEnhancement::initRadiometricModel(void){
    initProCam();
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    if ( !initK(*l_camSize) ) return false;
    if ( !initF(*l_camSize) ) return false;
    if ( !initCfull(*l_camSize) ) return false;
    if ( !initC0(*l_camSize) ) return false;
    return true;
}

// 反射率(K)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initK(const cv::Size& _camSize){
    // init K map
    m_KMap = Mat(_camSize, CV_64FC3, CV_SCALAR_D_WHITE);
    
    return true;
}

// 環境光(F)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initF(const cv::Size& _camSize){
    m_FMap = Mat(_camSize, CV_64FC3, CV_SCALAR_D_BLACK);

    return true;
}

// 最大輝度撮影色(Cfull)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initCfull(const cv::Size& _camSize){
    // init Cfull map
    m_CfullMap = Mat(_camSize, CV_64FC3, CV_SCALAR_D_WHITE);
    m_CMax = Mat(_camSize, CV_64FC3, CV_SCALAR_D_WHITE);
    return true;
}

// 最小輝度撮影色(C0)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initC0(const cv::Size& _camSize){
    // init C0 map
    m_C0Map = Mat(_camSize, CV_64FC3, CV_SCALAR_D_WHITE);
    m_CMin = Mat(_camSize, CV_64FC3, CV_SCALAR_D_WHITE);
    return true;
}

bool AppearanceEnhancement::initProCam(void){
    m_procam.allCalibration();
    return true;
}

///////////////////////////////  set method ///////////////////////////////
// 反射率の設定
// input / K    : 設定する反射率
// return       : 成功したかどうか
bool AppearanceEnhancement::setKMap(const cv::Mat& _K){
    m_KMap = _K;
    return true;
}

// 環境光の設定
// input / F    : 設定する環境光
// return       : 成功したかどうか
bool AppearanceEnhancement::setFMap(const cv::Mat& _F){
    // error handle
    if (!isEqualSizeAndType(m_FMap, _F)) {
        cerr << "different size or type" << endl;
        _print_mat_propaty(_F);
        _print_mat_propaty(m_FMap);
        exit(-1);
    }
    
    m_FMap = _F;
    return true;
}


// m_CfullMapの設定
// input / Cfull    : 設定する色
// return           : 成功したかどうか
bool AppearanceEnhancement::setCfullMap(void){
    ProCam* l_procam = getProCam();
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&m_CfullMap, g_maxPrjLuminance);
    showCfullMap();
    
    // Cmaxの設定
    m_CMax = m_CfullMap.clone() / 255.0;
    
//    _print_mat_propaty(m_CfullMap);
//    _print_mat_propaty(m_CMax);
    _print_mat_content_propaty2(0.1, m_CfullMap, m_CMax);
    return true;
}

// m_C0Mapの設定
// input / C0   : 設定する色
// return       : 成功したかどうか
bool AppearanceEnhancement::setC0Map(void){
    ProCam* l_procam = getProCam();
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&m_C0Map, g_minPrjLuminance);
    showC0Map();
    
    // CMinの設定
    m_CMin = m_C0Map.clone() / 255.0;

//    _print_mat_propaty(m_C0Map);
//    _print_mat_propaty(m_CMin);
    _print_mat_content_propaty2(0.1, m_C0Map, m_CMin);
    return true;
}

///////////////////////////////  get method ///////////////////////////////
// m_Cfullの取得
bool AppearanceEnhancement::getCfull(cv::Mat* const Cfull){
    *Cfull = m_Cfull;
    return true;
}

// m_CfullMapの取得
const cv::Mat& AppearanceEnhancement::getCfullMap(void){
    return m_CfullMap;
}
const cv::Mat& AppearanceEnhancement::getCMax(void){
    return m_CMax;
}

// m_C0の取得
bool AppearanceEnhancement::getC0(cv::Mat* const C0){
    *C0 = m_C0;
    return true;
}

// m_C0Mapの取得
const cv::Mat& AppearanceEnhancement::getC0Map(void)const{
    return m_C0Map;
}

//
const cv::Mat& AppearanceEnhancement::getCMin(void){
    return m_CMin;
}

// m_KMapの取得
//const cv::Mat_<Vec9d>& AppearanceEnhancement::getKMap(void){
const cv::Mat& AppearanceEnhancement::getKMap(void){
    return m_KMap;
}

// m_FMapの取得
const cv::Mat& AppearanceEnhancement::getFMap(void){
    return m_FMap;
}

// m_procamの取得
ProCam* AppearanceEnhancement::getProCam(void){
    return &m_procam;
}

bool AppearanceEnhancement::switchMode(void){
    switch (m_currentMode) {
        case e_Amano:
//            m_currentMode = e_Fujii;
            m_currentMode = e_Shimizu;
            cout << "Shimizu Mode" << endl;
            break;
//        case e_Fujii:
//            m_currentMode = e_Shimizu;
//            cout << "Shimizu Mode" << endl;
//            break;
        case e_Shimizu:
            m_currentMode = e_Amano;
            cout << "Amano Mode" << endl;
            break;
            
        default:
            break;
    }
    
    return true;
}

bool AppearanceEnhancement::isAmanoMode(void) const {
    if (m_currentMode == e_Amano) {
        return true;
    } else {
        return false;
    }
}

// 出力したい座標値であればtrue, それ以外はfalse
bool AppearanceEnhancement::isPrintFlag(const int _x, const int _y, const int _rows, const bool _contFlag) const {
    if (_contFlag && _x == m_printPoint.x + m_printPoint.y * _rows && _y == 0) {
        return true;
    } else if (!_contFlag && _x == m_printPoint.x && _y == m_printPoint.y) {
        return true;
    } else {
        return false;
    }
}

// printPointに円を描画
bool AppearanceEnhancement::getImageDrawingPrintPoint(cv::Mat* const _img) const{
//bool AppearanceEnhancement::getImageDrawingPrintPoint(cv::Mat* _img) const{
    cv::circle(*_img, m_printPoint, 3, CV_SCALAR_RED);
    return true;
}

///////////////////////////////  save method ///////////////////////////////
bool AppearanceEnhancement::saveAll(const int _num){
    // save
    ostringstream oss;
    const Mat l_K_ = getKMap();
    Mat l_K = l_K_.clone();
    const Mat l_F_ = getFMap();
    Mat l_F = l_F_.clone() / 255.0;
    const Mat l_CMax_ = getCfullMap();
    Mat l_CMax = l_CMax_.clone() / 255.0;
    const Mat l_mCMin_ = getC0Map();
    Mat l_CMin = l_mCMin_.clone() / 255.0;
    MY_IMWRITE_D4(OUTPUT_DATA_PATH, _num, oss, l_K, l_F, l_CMax, l_CMin);
    return true;
}

///////////////////////////////  calc method ///////////////////////////////
// 光学モデルを用いて反射率と環境光の推定を行う
// output / K   : 推定する反射率
// output / F   : 推定する環境光
// input / C1   : １つ目の撮影色
// input / P1   : １つ目の投影色
// input / C2   : ２つ目の撮影色
// input / P2   : ２つ目の投影色
// return       : 成功したかどうか
bool AppearanceEnhancement::calcReflectAndAmbient(cv::Mat* const _K, cv::Mat* const _F, const cv::Mat& _P1, const cv::Mat& _C1, const cv::Mat& _P2, const cv::Mat& _C2){
    // error processing
    
    // initialize
    Mat C0 = Mat::zeros(3, 1, CV_64FC1);
    Mat Cfull = Mat::zeros(3, 1, CV_64FC1);
    getC0(&C0);
    getCfull(&Cfull);
    Mat Cfull_0 = Cfull - C0;
    Cfull.release();
    Mat calcK = Mat::zeros(3, 3, CV_64FC1);
    Mat calcF = Mat::zeros(3, 1, CV_64FC1);
    Mat C2_1 = _C2 - _C1;
    Mat P2_1 = _P2 - _P1;
    
    // calculation
    for (int color = 0; color < 3; ++ color) {
        // init
        Mat C12 = (Mat_<double>(2, 2) << _C1.at<double>(0, color), -1, _C2.at<double>(0, color), -1);
        Mat C12inv = C12.inv();
        Mat P12 = (Mat_<double>(2, 2) << _P1.at<double>(0, color), 1, _P2.at<double>(0, color), 1);
        Mat C0full = (Mat_<double>(2, 1) << Cfull_0.at<double>(0, color), C0.at<double>(0, color));
        
        // calc K and F
        Mat Ans = C12inv * P12 * C0full;
//        cout << Ans<<" = "<<C12<<".inv() * "<<P12<<" * "<<C0full<<";" << endl;
        calcK.at<double>(color, color) = 1.0 / Ans.at<double>(0, 0);
        calcF.at<double>(0, color) = Ans.at<double>(0, 1);
        //        calcK.at<double>(color, color) = C2_1.at<double>(0, color) / (Cfull_0.at<double>(0, color) * P2_1.at<double>(0, color));
        //        calcF.at<double>(0, color) = ((Cfull_0.at<double>(0, color) * (_C1.at<double>(0, color) * _P2.at<double>(0, color)))) / C2_1.at<double>(0, color);
    }
    
    // 出力
    *_K = calcK;
    *_F = calcF;
    
    return true;
}

// 光学モデルを用いて反射率の推定を行う
// output / K   : 求める反射率
// input / C    : １つ目の撮影色
// input / P    : １つ目の投影色
// input / F   : 環境光
// return       : 成功したかどうか
bool AppearanceEnhancement::calcReflect(cv::Mat* const _K, const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _F){
    // initialize
    Mat C0 = Mat::zeros(3, 1, CV_64FC1);
    Mat Cfull = Mat::zeros(3, 1, CV_64FC1);
    getC0(&C0);
    getCfull(&Cfull);
    Mat Cfull_0 = Cfull - C0;
    Cfull.release();
    Mat calcK = Mat::zeros(3, 3, CV_64FC1);
    
    // calculation
    for (int c = 0; c < 3; ++ c) {
        calcK.at<double>(c, c) = _C.at<double>(0, c) / (Cfull_0.at<double>(0, c) * _P.at<double>(0,c) + C0.at<double>(0, c) + _F.at<double>(0, c));
    }
    
    // 丸め込み
//    roundReflectance(&calcK);
    
    // get estimated K
    *_K = calcK;
    
    return true;
}

// 光学モデルを用いて環境光の推定を行う
// output / K   : 求める反射率
// input / C    : １つ目の撮影色
// input / P    : １つ目の投影色
// input / F   : 環境光
// return       : 成功したかどうか
bool AppearanceEnhancement::calcAmbient(cv::Mat* const _F, const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _K){
    // initialize
    Mat C0 = Mat::zeros(3, 1, CV_64FC1);
    Mat Cfull = Mat::zeros(3, 1, CV_64FC1);
    getC0(&C0);
    getCfull(&Cfull);
    Mat Cfull_0 = Cfull - C0;
    Cfull.release();
    Mat calcF = Mat::zeros(3, 1, CV_64FC1);
    
    // calculation
    for (int c = 0; c < 3; ++ c) {
        calcF.at<double>(0, c) = _C.at<double>(0, c) / _K.at<double>(c, c) - (Cfull_0.at<double>(0, c) * _P.at<double>(0,c) + C0.at<double>(0, c));
    }
    
    // get estimated F
    *_F = calcF;
    
    return true;
}


// 光学モデルを用いて投影色から撮影色を計算(理想的なカメラ：ノイズレス)
// output / _C  : 取得した撮影色を入れる変数
// input / _K   : 反射率
// input / _F   : 環境光
// input / _P   : 投影色
// return       : 成功したかどうか
bool AppearanceEnhancement::calcIdealCamera(cv::Mat* const _C, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _P){
    return calcCameraAddedNoise(_C, _K, _F, _P, 0);
}

// 上のノイズ加えたバージョン
bool AppearanceEnhancement::calcCameraAddedNoise(cv::Mat* const _C, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _P, const double& noiseRange){
    return calcCameraAddedFixNoise(_C, _K, _F, _P, (rand() / (double)RAND_MAX) * 2 * noiseRange - noiseRange);
}

// 上のノイズを固定にした場合
bool AppearanceEnhancement::calcCameraAddedFixNoise(cv::Mat* const _C, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _P, const double& noise){
    // error processing
    Mat mat3x1 = Mat::zeros(3, 1, CV_8SC1);
    Mat mat3x3 = Mat::zeros(3, 3, CV_8SC1);
    if ( !isEqualSize(mat3x1, *_C, _P, _F) ) {
        std::cerr << "mat size is different" << endl;
        ERROR_PRINT3(*_C, _P, _F);
        return false;
    } else if ( !isEqualSize(mat3x3, _K) ) {
        std::cerr << "mat size is different" << endl;
        ERROR_PRINT(_K);
        return false;
    }
    mat3x1.release();
    mat3x3.release();
    
    // init
    Mat C = Mat::zeros(3, 1, CV_64FC1);
    Mat Cfull, C0;
    getCfull(&Cfull);
    getC0(&C0);
    
    // calculation
    for (int c = 0; c < 3; ++ c) {
        C.at<double>(0, c) = _K.at<double>(c, c) * ((Cfull.at<double>(0, c) - C0.at<double>(0, c)) * _P.at<double>(0, c) + C0.at<double>(0, c) + _F.at<double>(0, c)) + noise;
    }
    *_C = C;
    
    return true;
}

// 次に投影するPを計算する
// output / _P  : 計算した投影色を入れる変数
// input / _C   : 撮影色
// input / _K   : 反射率
// input / _F   : 環境光
// return       : 成功したかどうか
bool AppearanceEnhancement::calcNextProjection(cv::Mat* const _P, const cv::Mat& _C, const cv::Mat& _K, const cv::Mat& _F){
    // error processing
    Mat color = Mat::zeros(3, 1, CV_8SC1);
    Mat ref = Mat::zeros(3, 3, CV_8SC1);
    if ( !isEqualSize(color, *_P, _C, _F) ) {
        std::cerr << "size is different" << std::endl;
        ERROR_PRINT3(*_P, _C, _F);
        return false;
    } else if ( !isEqualSize(ref, _K) ) {
        std::cerr << "size is different" << std::endl;
        ERROR_PRINT(_K);
        ERROR_PRINT(ref);
        return false;
    }
    color.release();
    ref.release();
    
    // init
    Mat Cfull, C0, Cfull_0;
    getC0(&C0);
    getCfull(&Cfull);
    Cfull_0 = Cfull - C0;
    //    Cfull.release();
    
    // calc _P
    Mat P = Mat::zeros(3, 1, CV_64FC1);
    divElmByElm(&P, _C, _K.diag()); // P = C ./ K
    P -= C0 + _F;                   // P = C ./ K - (C0 + F)
    divElmByElm(&P, P, Cfull_0);    // P = { C ./ K - (C0 + F) } / (Cfull - C0)
    
    // round 0 to 1
    round0to1ForMat(&P);
    
    // Pに出力
    *_P = P;
    return true;
}

// range = K * ( (Cfull - C0) * [0-1] + C0 + F)
// output / _rangeTop   : 計算する範囲の上端
// outpute / _rangeDown : 計算する範囲の下端
// input / _K           : 反射率
// input / _F           : 環境光
bool AppearanceEnhancement::calcRangeOfDesireC(cv::Mat* const _rangeTop, cv::Mat* const _rangeDown, const cv::Mat& _K, const cv::Mat& _F){
    // init
    const Mat PBottom = Mat::zeros(3, 1, CV_64FC1);
    const Mat PTop = Mat::ones(3, 1, CV_64FC1);
    Mat Cfull, C0;
    getCfull(&Cfull);
    getC0(&C0);
    Mat Cfull_0 = Cfull - C0;
    Cfull.release();
    Mat rangeTop = Mat::zeros(3, 1, CV_64FC1), rangeBottom = Mat::zeros(3, 1, CV_64FC1);
    
    // calculation
    mulElmByElm(&rangeTop, Cfull_0, PTop);
    mulElmByElm(&rangeBottom, Cfull_0, PBottom);
    
    rangeTop += C0 + _F;
    rangeBottom += C0 + _F;
    
    rangeTop = _K * rangeTop;
    rangeBottom = _K * rangeBottom;
    
    // 代入
    *_rangeTop = rangeTop;
    *_rangeDown = rangeBottom;
    
    return true;
}

// 見た目をどのような画像にするか決定する
bool AppearanceEnhancement::calcTargetImage(cv::Mat* const _targetImage, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _CMin, const double& _s, const int _enhanceType){
    // init
    int rows = _K.rows, cols = _K.cols;
    Mat l_targetImage(rows, cols, CV_64FC3, CV_SCALAR_BLACK);
    
    // create gray scale
    Mat l_grayCest(rows, cols, CV_32FC1, 0);
    Mat l_K(rows, cols, CV_32FC3);
    _K.convertTo(l_K, CV_32FC3);
    cvtColor(l_K, l_grayCest, CV_BGR2GRAY);
    l_grayCest.convertTo(l_grayCest, CV_64FC1);
    
    // scan
    if (isContinuous(l_targetImage, l_K, l_grayCest)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        Vec3d* l_pTargetImage = l_targetImage.ptr<Vec3d>(y);
        const Vec3d* l_pK = _K.ptr<Vec3d>(y);
        const Vec3d* l_pF = _F.ptr<Vec3d>(y);
        const Vec3d* l_pCMin = _CMin.ptr<Vec3d>(y);
        const double* l_pGrayCest = l_grayCest.ptr<double>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < 3; ++ c) {
                // calc desire C
                calcTargetImageAtPixel(&(l_pTargetImage[x][c]), l_pK[x][c], l_pF[x][c], l_pCMin[x][c], l_pGrayCest[x], _s, _enhanceType);
            }
        }
    }
    
    // copy
    *_targetImage = l_targetImage.clone();
//    MY_IMSHOW(l_targetImage);
    
    return true;
}
bool AppearanceEnhancement::calcTargetImageAtPixel(double* const _targetImage, const double& _K, const double& _F, const double& _CMin, const double& _KGray, const double& _s, const int _enhanceType){
    double l_targetImageNum = 0;
    switch (_enhanceType) {
        case 0:
            l_targetImageNum = ((1 + _s) * _K - _s * _KGray) * 255;
            break;
        case 1:
            l_targetImageNum = 256 / 2;
            break;
        default:
            break;
    }
//    double l_targetImageNum = ((1 + _s) * _K - _s * _KGray) * (128 + _F + _CMin);
//    roundXtoY(&l_targetImageNum, 0, 255);
//    l_pTargetImage[x][c] = (uchar)l_targetImageNum;
    *_targetImage = l_targetImageNum;

    return true;
}

// 次に投影する画像を決定
bool AppearanceEnhancement::calcNextProjectionImage(cv::Mat* const _nextP, cv::Mat* const _error, cv::Mat* const _Cr, cv::Mat* const _vrC, cv::Mat* const _desireC, const cv::Mat& _targetImage, const cv::Mat& _targetImageBefore, const cv::Mat& _C, const cv::Mat& _P, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _FBefore, const cv::Mat& _CMax, const cv::Mat& _CMin, const double& _alpha){
    
    // init
    int rows = _targetImage.rows, cols = _targetImage.cols, channel = _targetImage.channels();
    if (isContinuous(*_nextP, _targetImage, _targetImageBefore, _C, _P, _K, _F, _FBefore, _CMax, _CMin) &&
        isContinuous(*_error, *_Cr, *_vrC, *_desireC)) {
        cols *= rows;
        rows = 1;
    }
    
    // scaning
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pNextP = _nextP->ptr<Vec3b>(y);
        Vec3d* l_pError = _error->ptr<Vec3d>(y);
        Vec3d* l_pCr = _Cr->ptr<Vec3d>(y);
        Vec3d* l_pVrC = _vrC->ptr<Vec3d>(y);
        Vec3d* l_pDesireC = _desireC->ptr<Vec3d>(y);
        const Vec3d* l_pTargetImage = _targetImage.ptr<Vec3d>(y);
        const Vec3d* l_pTargetImageBefore = _targetImageBefore.ptr<Vec3d>(y);
        const Vec3d* l_pC = _C.ptr<Vec3d>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3d* l_pK = _K.ptr<Vec3d>(y);
        const Vec3d* l_pF = _F.ptr<Vec3d>(y);
        const Vec3d* l_pFBefore = _FBefore.ptr<Vec3d>(y);
        const Vec3d* l_pCMax = _CMax.ptr<Vec3d>(y);
        const Vec3d* l_pCMin = _CMin.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < channel; ++ c) {
                // normalize
                const double l_nTargetImage = l_pTargetImage[x][c] / 255.0;
                const double l_nTargetImageBefore = l_pTargetImageBefore[x][c] / 255.0;
                const double l_nC = l_pC[x][c] / 255.0;
                const double l_nP = l_pP[x][c] / 255.0;
                const double l_nK = l_pK[x][c];
                const double l_nF = l_pF[x][c] / 255.0;
                const double l_nFBefore = l_pFBefore[x][c] / 255.0;
                const double l_nCMax = l_pCMax[x][c] / 255.0;
                const double l_nCMin = l_pCMin[x][c] / 255.0;
                const double l_nPMax = (double)g_maxPrjLuminance[c] / 255.0;
                const double l_nPMin = (double)g_minPrjLuminance[c] / 255.0;

                // calculation
                calcNextProjectionImageAtPixel(&(l_pNextP[x][c]), &(l_pError[x][c]), &(l_pCr[x][c]), &(l_pVrC[x][c]), &(l_pDesireC[x][c]), l_nTargetImage, l_nTargetImageBefore, l_nC, l_nP, l_nK, l_nF, l_nFBefore, l_nCMax, l_nCMin, l_nPMax, l_nPMin, _alpha);
            }
        }
    }
//    MY_IMSHOW(*_nextP);
    return true;
}
bool AppearanceEnhancement::test_calcNextProjectionImage(const cv::Mat& _answerK, const cv::Mat& _answerF, const cv::Mat& _CMax, const cv::Mat& _CMin, const cv::Scalar& _mask){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_projectionImage(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Mat l_projectionImageBefore(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Mat l_captureImage(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Mat l_targetImage(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Mat l_targetImageBefore(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Mat l_error(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Mat l_Cr(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Mat l_vrC(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Mat l_desireC(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Scalar l_mean(0,0,0,0), l_stddev(0,0,0,0);
    Scalar l_meanCap(0,0,0,0), l_stddevCap(0,0,0,0);
    Scalar l_meanTarget(0,0,0,0), l_stddeTarget(0,0,0,0);
    double l_alphaMPC = 0.1;
    ofstream ofs(TEST_PROJECTION_FILE_NAME.c_str());
    
    for (int i = 0; i < 256; ++ i) {
        // set target
        l_targetImageBefore = l_targetImage;
        l_targetImage = Scalar(i, i, i);
        
        // To shift time
        l_projectionImageBefore = l_projectionImage.clone();
        
        // calc next projection image
        calcNextProjectionImage(&l_projectionImage, &l_error, &l_Cr, &l_vrC, &l_desireC, l_targetImage, l_targetImageBefore, l_captureImage, l_projectionImageBefore, _answerK, _answerF, _answerF, _CMax, _CMin, l_alphaMPC);
        
        // projection
        l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionImage);
        
        // calc
        calcMeanStddevOfDiffImage(&l_mean, &l_stddev, l_captureImage, l_targetImage);
        meanStdDev(l_captureImage, l_meanCap, l_stddevCap);
        meanStdDev(l_targetImage, l_meanTarget, l_stddeTarget);
        
        // print
        std::cout << i << "\t";
        _print_gnuplot_color6_l(std::cout, l_mean, l_stddev, l_meanCap, l_stddevCap, l_meanTarget, l_stddeTarget);
        ofs << i << "\t";
        _print_gnuplot_color6_l(ofs, l_mean, l_stddev, l_meanCap, l_stddevCap, l_meanTarget, l_stddeTarget);
        
        // if you are mistake, you should push delete button then go back
        if (waitKey(30) == CV_BUTTON_DELETE) {
            return false;
        }
    }
    return true;
}

bool AppearanceEnhancement::calcNextProjectionImageAtPixel(uchar* const _nextP, double* const _error, double* const _Cr, double* const _vrC, double* const _desireC, const double& _targetImage, const double& _targetImageBefore, const double& _C, const double& _P, const double& _K, const double& _F, const double& _FBefore, const double& _CMax, const double& _CMin, const double& _PMax, const double& _PMin, const double& _alpha){
    // calculation
    const double l_interP = (_P - _PMin) / (_PMax - _PMin);
    const double l_interC = (_CMax - _CMin) * l_interP + _CMin + _F;
    const double l_baseP = 0.5;
    const double l_interBaseP = (l_baseP - _PMin) / (_PMax - _PMin);
    const double l_idealBaseC = (_CMax - _CMin) * l_interBaseP + _CMin + _F;
    *_desireC = _targetImage * l_idealBaseC;
    
    double l_nNextP = _PMin + _targetImage * (l_baseP - _PMin) / _K + (_PMax - _PMin) * (_CMin + _F) * (1 + _targetImage / _K) / (_CMax - _CMin);
    *_vrC = _K * l_interC;
    *_error = _C - *_vrC;
    *_Cr = _alpha * _C + (1 - _alpha) * _targetImage;
    
    round0to1(&l_nNextP);
    
    // unnormalize
//    *_nextP = std::max((uchar)(l_nNextP * 255), (uchar)50);
    *_nextP = std::max((uchar)(l_nNextP * 255), (uchar)0);
//    *_nextP = std::min(*_nextP, _maxLuminance);
    return true;
}
bool AppearanceEnhancement::test_calcNextProjectionImageAtPixel(void){
    double l_CMax = 0.99, l_CMin = 0.1;
    double l_ansK = 0.5, l_ansF = 0.0;
    double l_P = 1, l_PBefore = 1, l_C = 0;
    double l_noise = NOISE_RANGE * 5;
    double l_target = 0.25, l_alpha = 0.5;
    ofstream ofs(SIM_PROJECTION_FILE_NAME.c_str());
    double l_error = 0, l_Cr = 0, l_vrC = 0, l_desireC;
    
    for (int i = 0; i < 256; ++ i) {
//        if (i % 10 == 0) {
//            l_target += 10.0 / 256.0;
//        }
        //
        l_PBefore = l_P;
        uchar l_ucP = 0;
        calcNextProjectionImageAtPixel(&l_ucP, &l_error, &l_Cr, &l_vrC, &l_desireC, l_target, l_target, l_C, l_PBefore, l_ansK, l_ansF, l_ansF, l_CMax, l_CMin, g_maxPrjLuminance[0]/255.0, g_minPrjLuminance[0]/255.0, l_alpha);
        l_P = (double)l_ucP / 255.0;
        
        // capture
        calcCaptureImageAddNoise(&l_C, l_P, l_ansK, l_ansF, l_CMax, l_CMin, l_noise);
        
        // print
        _print_gnuplot4(std::cout, i, l_C, l_target, l_P);
        _print_gnuplot4(ofs, i, l_C, l_target, l_P);
    }
    
    return true;
}

// 反射率の計算
bool AppearanceEnhancement::calcReflectanceAtPixel(double* const _K, const double& _nC, const double& _nP, const double& _nCMax, const double& _nCMin, const double& _nPMax, const double& _nPMin, const double& _nF, double* const _idealC){
    // calc
    // normalized linear interpolation P
    const double l_interP = (_nP - _nPMin)/(_nPMax - _nPMin);
    double l_idealC = (_nCMax - _nCMin) * l_interP + _nCMin + _nF;
    *_idealC = l_idealC;
    // avoid 0
    l_idealC = std::max(l_idealC, 1.0/255.0);
    const double l_nC = std::max(_nC, 1.0/255.0);
    double l_K = l_nC / l_idealC;
//    round0to1(&l_K);
    
//    if (l_K > 2.0) {
//        _print_bar;
//        cout<<"l_interP "<<l_interP<<"= ("<<_nP<<" - "<<_nPMin<<")/("<<_nPMax<<" - "<<_nPMin<<")"<<endl;
//        cout<<"l_idealC "<<l_idealC<<"= ("<<_nCMax<<" - "<<_nCMin<<") * "<<l_interP<<" + "<<_nCMin<<endl;
//        cout<<"l_K "<<l_K<<" = "<<l_nC<<" / "<<l_idealC<<endl;
//    }
    
    // copy
    *_K = l_K;
    return true;
}
bool AppearanceEnhancement::calcReflectanceAndAmbientLightAtPixel(double* const _K, double* const _F, const double& _nC1, const double& _nP1, const double& _nC2, const double& _nP2, const double& _nCMax, const double& _nCMin, const double& _nPMax, const double& _nPMin, const bool _printFlag){
    // set matrix
    const double l_CPmm = (_nCMax - _nCMin) / (_nPMax - _nPMin);
    const Mat l_C12 = (Mat_<double>(2, 2) << _nC1, -1, _nC2, -1);
    const Mat l_P12 = (Mat_<double>(2, 2) << _nP1, 1, _nP2, 1);
    const Mat l_mCPmm = (Mat_<double>(2, 1) << l_CPmm, (_nCMin - l_CPmm * _nPMin));
    
    // calculation
    const Mat l_nKF = l_C12.inv() * l_P12 * l_mCPmm;
    
    // set
    // K
    *_K = 1 / l_nKF.at<double>(0, 0);
    // F
    double l_nRoundF = l_nKF.at<double>(1, 0);
//    round0to1(&l_nRoundF);
    *_F = l_nRoundF * 255.0;
    
    if (_printFlag) {
        cout << l_nKF<<" = "<<l_C12<<".inv() * "<<l_P12<<" * "<<l_mCPmm << endl;
        _print2(*_K, *_F);
        _print_bar;
    }
    return true;
}

// calc capture image from C, P, K, F, CMax, and CMin
bool AppearanceEnhancement::calcCaptureImageAddNoise(double* const _C, const double& _P, const double& _K, const double& _F, const double& _CMax, const double& _CMin, const double& _noiseRange){
    *_C = _K * ((_CMax - _CMin) * _P + _CMin + _F) + (rand() / (double)RAND_MAX) * 2 * _noiseRange - _noiseRange;
    return true;
}

// 推定したKFからCを生成する
bool AppearanceEnhancement::calcVirtualC(cv::Mat* const _vrC, const cv::Mat& _P){
    const Mat l_K = getKMap();
    const Mat l_F = getFMap();
    const Mat l_CMax = getCfullMap();
    const Mat l_CMin = getC0Map();
    const Vec3b l_PMax = g_maxPrjLuminance;
    const Vec3b l_PMin = g_minPrjLuminance;
    
    int rows = _P.rows, cols = _P.cols, channels = _P.channels();
    if (isContinuous(*_vrC, _P, l_K, l_F, l_CMax, l_CMin)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        Vec3d* l_pVrC = _vrC->ptr<Vec3d>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3d* l_pK = l_K.ptr<Vec3d>(y);
        const Vec3d* l_pF = l_F.ptr<Vec3d>(y);
        const Vec3d* l_pCMax = l_CMax.ptr<Vec3d>(y);
        const Vec3d* l_pCMin = l_CMin.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < channels; ++ c) {
                // normalize
                const double l_nP = (double)l_pP[x][c] / 255.0;
                const double l_nCMax = l_pCMax[x][c] / 255.0;
                const double l_nCMin = l_pCMin[x][c] / 255.0;
                const double l_nK = l_pK[x][c];
                const double l_nF = l_pF[x][c] / 255.0;
                const double l_nPMax = (double)l_PMax[c] / 255.0;
                const double l_nPMin = (double)l_PMin[c] / 255.0;
                
                // calc
                const double l_nInterP = (l_nP - l_nPMin) / (l_nPMax - l_nPMin);
                double l_nInterStandardC = 0;
                if (isAmanoMode()) {
                    l_nInterStandardC = (l_nCMax - l_nCMin) * l_nInterP + l_nCMin;
                } else {
                    l_nInterStandardC = (l_nCMax - l_nCMin) * l_nInterP + l_nCMin + l_nF;
                }
                l_pVrC[x][c] = l_nK * l_nInterStandardC;
            }
        }
    }
    return true;
}

////////////////////////////// estimate method //////////////////////////////
bool AppearanceEnhancement::estimateK(const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _CMax, const cv::Mat& _CMin, const cv::Mat& _F, cv::Mat* const _idealC){
    // init
    int rows = _P.rows, cols = _P.cols;
    Mat l_K(rows, cols, CV_64FC3, CV_SCALAR_BLACK);
    if (isContinuous(_P, _C, _CMax, _CMin, l_K, _F, *_idealC)) {
        cols *= rows;
        rows = 1;
    }
    
    // scan
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3d* l_pK = l_K.ptr<Vec3d>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3d* l_pC = _C.ptr<Vec3d>(y);
        const Vec3d* l_pCMax = _CMax.ptr<Vec3d>(y);
        const Vec3d* l_pCMin = _CMin.ptr<Vec3d>(y);
        const Vec3d* l_pF = _F.ptr<Vec3d>(y);
        Vec3d* l_pIdealC = _idealC->ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // calc
            // K = C / {(Cf - C0) * P + C0}
            for (int c = 0; c < 3; ++ c) {
                // normalize
                const double l_nC = l_pC[x][c] / 255.0;
                const double l_nP = (double)l_pP[x][c] / 255.0;
                const double l_nF = (double)l_pF[x][c] / 255.0;
                const double l_nCMax = l_pCMax[x][c] / 255.0;
                const double l_nCMin = l_pCMin[x][c] / 255.0;
                const double l_nPMax = (double)g_maxPrjLuminance[c] / 255.0;
                const double l_nPMin = (double)g_minPrjLuminance[c] / 255.0;
                
                // calculation
                calcReflectanceAtPixel(&(l_pK[x][c]), l_nC, l_nP, l_nCMax, l_nCMin, l_nPMax, l_nPMin, l_nF, &(l_pIdealC[x][c]));
            }
        }
    }
    
    // save
    setKMap(l_K);
    return true;
}
bool AppearanceEnhancement::test_estimateK(const cv::Mat& _answerK, const cv::Mat& _CMax, const cv::Mat& _CMin, const cv::Scalar& _mask){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_projectionImage(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Mat l_captureImage(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Mat l_idealC(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Scalar l_meanDiff(0,0,0,0), l_stddevDiff(0,0,0,0);
    Scalar l_meanEst(0,0,0,0), l_stddevEst(0,0,0,0);
    Scalar l_meanAns(0,0,0,0), l_stddevAns(0,0,0,0);
    ofstream ofs(ESTIMATE_K_FILE_NAME.c_str());
    const Mat l_F(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    
    for (int i = 0; i < 256; ++ i) {
        // projection and capture
        const Scalar l_prjColor(i * _mask[0], i * _mask[1], i * _mask[2]);
        l_projectionImage = l_prjColor;
        l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionImage);
        
        // estimation
        estimateK(l_projectionImage, l_captureImage, _CMax, _CMin, l_F, &l_idealC);
        const Mat l_estK = getKMap();
        
        // calc
        calcMeanStddevOfDiffImage(&l_meanDiff, &l_stddevDiff, _answerK, l_estK);
        meanStdDev(l_estK, l_meanEst, l_stddevEst);
        meanStdDev(_answerK, l_meanAns, l_stddevAns);
        
        // print
        std::cout << i << "\t";
        _print_gnuplot_color6_l(std::cout, l_meanDiff, l_stddevDiff, l_meanEst, l_stddevEst, l_meanAns, l_stddevAns);
        ofs << i << "\t";
        _print_gnuplot_color6_l(ofs, l_meanDiff, l_stddevDiff, l_meanEst, l_stddevEst, l_meanAns, l_stddevAns);
        
        // if you are mistake, you should push delete button then go back
        if (waitKey(30) == CV_BUTTON_DELETE) {
            return false;
        }
    }
    return true;
}

// Fの推定
// model : C = K * {(Cf - C0) * P + C0 + F}
bool AppearanceEnhancement::estimateF(const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _CMax, const cv::Mat& _CMin) {
    // get C
    ProCam* l_procam = getProCam();
    const Size l_camSize = l_procam->getCameraSize_();
    Mat l_F(l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    const Mat l_K = getKMap();
    
    // calc
    int rows = _P.rows, cols = _P.cols;
    if (isContinuous(_P, _C, _CMax, _CMin, l_K, l_F)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3d* l_pF = l_F.ptr<Vec3d>(y);
        const Vec3d* l_pK = l_K.ptr<Vec3d>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3d* l_pC = _C.ptr<Vec3d>(y);
        const Vec3d* l_pCMax = _CMax.ptr<Vec3d>(y);
        const Vec3d* l_pCMin = _CMin.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // calc
            // K = C / {(Cf - C0) * P + C0}
            for (int c = 0; c < 3; ++ c) {
                // normalize
                const double l_nC = l_pC[x][c] / 255.0;
                const double l_nP = (double)l_pP[x][c] / 255.0;
                const double l_nCMax = l_pCMax[x][c] / 255.0;
                const double l_nCMin = l_pCMin[x][c] / 255.0;
                const double l_nPMax = (double)g_maxPrjLuminance[c] / 255.0;
                const double l_nPMin = (double)g_minPrjLuminance[c] / 255.0;

                // calculation
                const double l_interP = (l_nP - l_nPMin) / (l_nPMax - l_nPMin);
                const double l_vrCOnWhiteLight = (l_nCMax - l_nCMin) * l_interP + l_nCMin;
                const double l_nF = l_nC / l_pK[x][c] - l_vrCOnWhiteLight;
                
                // inverse normalize
                //                round0to1(&l_nF);
                l_pF[x][c] = l_nF * 255.0;
            }
        }
    }
    
    // set and save
    setFMap(l_F);
    return true;
}


// estimate K and F
bool AppearanceEnhancement::estimateKFByAmanoModel(const cv::Mat& _P1, const cv::Mat& _P2, const cv::Mat& _C1, const cv::Mat& _C2){
    // init
    const Mat l_CMax = getCfullMap(), l_CMin = getC0Map();
    const Size l_camSize(l_CMax.cols, l_CMax.rows);
    Mat l_K(l_camSize, CV_64FC3, CV_SCALAR_BLACK), l_F(l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    bool l_printFlag = false, l_contFlag = false;
    
    int rows = _P1.rows, cols = _P1.cols;
//    l_contFlag = isContinuous(_P1, _P2, _C1, _C2, l_CMax, l_CMin, l_K, l_F);
//    if (l_contFlag) {
//        cols *= rows;
//        rows = 1;
//    }
    for (int y = 0; y < rows; ++ y) {
        Vec3d* l_pK = l_K.ptr<Vec3d>(y);
        Vec3d* l_pF = l_F.ptr<Vec3d>(y);
        const Vec3d* l_pC1 = _C1.ptr<Vec3d>(y);
        const Vec3d* l_pC2 = _C2.ptr<Vec3d>(y);
        const Vec3b* l_pP1 = _P1.ptr<Vec3b>(y);
        const Vec3b* l_pP2 = _P2.ptr<Vec3b>(y);
        const Vec3d* l_pCMax = l_CMax.ptr<Vec3d>(y);
        const Vec3d* l_pCMin = l_CMin.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            if (isPrintFlag(x, y, rows, l_contFlag)) {
                l_printFlag = true;
            }
            for (int c = 0; c < 3; ++ c) {
                // normalize
                const double l_nC1 = l_pC1[x][c] / 255.0;
                const double l_nC2 = l_pC2[x][c] / 255.0;
                const double l_nP1 = l_pP1[x][c] / 255.0;
                const double l_nP2 = l_pP2[x][c] / 255.0;
                const double l_nCMax = l_pCMax[x][c] / 255.0;
                const double l_nCMin = l_pCMin[x][c] / 255.0;
                double l_nPMax = (double)g_maxPrjLuminance[c] / 255.0;
                double l_nPMin = (double)g_minPrjLuminance[c] / 255.0;

                // calculation
                calcReflectanceAndAmbientLightAtPixel(&(l_pK[x][c]), &(l_pF[x][c]), l_nC1, l_nP1, l_nC2, l_nP2, l_nCMax, l_nCMin, l_nPMax, l_nPMin, l_printFlag);
            }
            l_printFlag = false;
        }
    }
    
    // set
    setKMap(l_K);
    setFMap(l_F);
    return true;
}
bool AppearanceEnhancement::estimateKFByAmanoModel(const cv::Mat& _P1, const cv::Mat& _P2){
    // init
    ProCam* l_procam = getProCam();
    const Size l_camSize = l_procam->getCameraSize_();
    const Mat l_Cfull = getCfullMap(), l_C0 = getC0Map();
    Mat l_C1(l_camSize, CV_64FC3, CV_SCALAR_BLACK), l_C2(l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C1, _P1, false, SLEEP_TIME * 5);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C2, _P2);
    const Mat l_C1d = l_C1.clone() / 255.0;
    const Mat l_C2d = l_C2.clone() / 255.0;
    MY_IMSHOW2(l_C1d, l_C2d);
    Mat l_C1b = l_C1.clone(), l_C2b = l_C2.clone();
    l_C1.convertTo(l_C1b, CV_8UC3);
    l_C2.convertTo(l_C2b, CV_8UC3);
    imwrite("./data/l_C1.png", l_C1b);
    imwrite("./data/l_C2.png", l_C2b);
    
    estimateKFByAmanoModel(_P1, _P2, l_C1, l_C2);
    
    return true;
}

// コアダンプ起こるから修正する
bool AppearanceEnhancement::test_estimateKFByAmanoModel(const cv::Mat& _answerK, const cv::Scalar& _mask){
    // init
    ProCam* l_procam = getProCam();
    const Size l_camSize = l_procam->getCameraSize_();
    Mat l_projectionImage1(l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Mat l_projectionImage2(l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Mat l_captureImage1(l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Mat l_captureImage2(l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Scalar l_mean(0,0,0,0), l_stddev(0,0,0,0);
    Scalar l_meanEstK(0,0,0,0), l_stddevEstK(0,0,0,0);
    Scalar l_meanEstF(0,0,0,0), l_stddevEstF(0,0,0,0);
    ofstream ofs(ESTIMATE_KF_FILE_NAME.c_str());

    // initial projection
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage1, 0, false, SLEEP_TIME * 5);

    // loop
    for (int i = 0; i < 256; ++ i) {
        // projection and capture
//        const Scalar l_prjColor(i * _mask[0], i * _mask[1], i * _mask[2]);
        const Scalar l_prjColor = (double)i * _mask;
        l_projectionImage2 = l_prjColor;
        
        l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage2, l_projectionImage2, false, SLEEP_TIME);
        
        // estimation
        estimateKFByAmanoModel(l_projectionImage1, l_projectionImage2, l_captureImage1, l_captureImage2);
        const Mat l_estK = getKMap(), l_estF = getFMap() / 255.0;
        
        // calc
        calcMeanStddevOfDiffImage(&l_mean, &l_stddev, _answerK, l_estK);
        meanStdDev(l_estK, l_meanEstK, l_stddevEstK);
        meanStdDev(l_estF, l_meanEstF, l_stddevEstF);
        
        // print
        std::cout << i << "\t";
        _print_gnuplot_color6_l(std::cout, l_mean, l_stddev, l_meanEstK, l_stddevEstK, l_meanEstF, l_stddevEstF);
        ofs << i << "\t";
        _print_gnuplot_color6_l(ofs, l_mean, l_stddev, l_meanEstK, l_stddevEstK, l_meanEstF, l_stddevEstF);
        
        // if you are mistake, you should push delete button then go back
        if (waitKey(30) == CV_BUTTON_DELETE) {
            return false;
        }
    }

    return true;
}

bool AppearanceEnhancement::estimateKFByFujiiModel(const cv::Mat& _P1, const cv::Mat& _P2){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_C1(*l_camSize, CV_64FC3, CV_SCALAR_BLACK), l_C2(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Mat l_K(*l_camSize, CV_64FC3, CV_SCALAR_BLACK), l_F(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
//    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C1, _P1, false, SLEEP_TIME * 5);
//    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C2, _P2);
    l_procam->captureFromLinearLightOnProjectorDomainAndColor(&l_C1, _P1);
    l_procam->captureFromLinearLightOnProjectorDomainAndColor(&l_C2, _P2);
    
    int rows = _P1.rows, cols = _P1.cols;
    if (isContinuous(_P1, _P2, l_C1, l_C2, l_K, l_F)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        Vec3d* l_pK = l_K.ptr<Vec3d>(y);
        Vec3d* l_pF = l_F.ptr<Vec3d>(y);
        const Vec3d* l_pC1 = l_C1.ptr<Vec3d>(y);
        const Vec3d* l_pC2 = l_C2.ptr<Vec3d>(y);
        const Vec3b* l_pP1 = _P1.ptr<Vec3b>(y);
        const Vec3b* l_pP2 = _P2.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < 3; ++ c) {
                // normalize
                const double l_nC1 = l_pC1[x][c] / 255.0;
                const double l_nC2 = l_pC2[x][c] / 255.0;
                const double l_nP1 = l_pP1[x][c] / 255.0;
                const double l_nP2 = l_pP2[x][c] / 255.0;
                
                // set matrix
                const Mat l_C12 = (Mat_<double>(2, 2) << l_nC1, -1, l_nC2, -1);
                const Mat l_P12 = (Mat_<double>(2, 1) << l_nP1, l_nP2);
                
                // calculation
                const Mat l_nKF = l_C12.inv() * l_P12;
                
                // set
                // K
                l_pK[x][c] = 1 / l_nKF.at<double>(0, 0);
                // F
                double l_roundF = l_nKF.at<double>(1, 0);
//                round0to1(&l_roundF);
//                l_pF[x][c] = (uchar)(l_roundF * 255);
                l_pF[x][c] = l_roundF * 255;
            }
        }
    }

    // setting
    setKMap(l_K);
    setFMap(l_F);
    
    // evaluate
//    evaluateEstimate(l_C1, _P1, 1);
//    evaluateEstimate(l_C2, _P2, 2);

    return true;
}

////////////////////////////// evaluate method //////////////////////////////
// 推定の評価
bool AppearanceEnhancement::evaluateEstimate(const cv::Mat& _C, const cv::Mat& _P, const int num){
    // make C using model
    Mat l_virtualC = _C.clone();
    calcVirtualC(&l_virtualC, _P);
    
    // calc difference
    Mat l_C = _C.clone() / 255.0;
    Mat l_diffC = _C.clone();
    absdiff(l_C, l_virtualC, l_diffC);
    
    // calc mean and standard deviation
    Scalar l_mean(0,0,0,0), l_stddev(0,0,0,0);
    meanStdDev(l_diffC, l_mean, l_stddev);
    
    // show
    ostringstream oss;
    oss << "diff C and virtualC" << num;
    imshow(oss.str().c_str(), l_diffC);
    ostringstream oss2;
    oss2 << "virtualC" << num;
    imshow(oss2.str().c_str(), l_virtualC);
    _print2(l_mean, l_stddev);
    
    return true;
}

bool AppearanceEnhancement::evaluateEstimationAndProjection(const cv::Mat& _ansK, const cv::Mat& _estK, const cv::Mat& _ansF, const cv::Mat& _estF, const cv::Mat& _targetImage, const cv::Mat& _captureImage, const cv::Mat& _desireC){
    const double l_rate = 0.1;
    // error of estimated K
    _print_diff_mat_content_propaty(l_rate, _ansK, _estK);

    // error of estimated F
    const Mat l_nEstF = _estF.clone() / 255.0;
    const Mat l_nAnsF = _ansF.clone() / 255.0;
    _print_diff_mat_content_propaty(l_rate, l_nEstF, l_nAnsF);
    
    // error of project
    const Mat l_nDesire = _desireC.clone();
    const Mat l_nResult = _captureImage.clone() / 255.0;
    _print_diff_mat_content_propaty(l_rate, l_nDesire, l_nResult);
    
    return true;
}

///////////////////////////////  show method ///////////////////////////////
bool AppearanceEnhancement::showKMap(void){
    const Mat l_K = getKMap();
    MY_IMSHOW(l_K);
    return true;
}
bool AppearanceEnhancement::showFMap(void){
    const Mat l_F_ = getFMap();
    const Mat l_F = l_F_ / 255.0;
    MY_IMSHOW(l_F);
    return true;
}
bool AppearanceEnhancement::showCfullMap(void){
    const Mat l_Cfull_ = getCfullMap();
    const Mat l_Cfull = l_Cfull_ / 255.0;
    MY_IMSHOW(l_Cfull);
    return true;
}
bool AppearanceEnhancement::showC0Map(void){
    const Mat l_C0_ = getC0Map();
    const Mat l_C0 = l_C0_ / 255.0;
    MY_IMSHOW(l_C0);
    return true;
}
bool AppearanceEnhancement::showAll(const int _num, const cv::Mat& _C, const cv::Mat& _P, const cv::Mat& _desireC, const cv::Mat& _answerK, const cv::Mat& _answerF, const cv::Mat& _errorOfMPC, const cv::Mat& _CrOfMPC, const cv::Mat& _vrC){
    const Mat l_C = _C.clone() / 255.0;
    
    // create error image
    Mat l_errorOfProjection = l_C.clone();
    absdiff(_desireC, l_C, l_errorOfProjection);
    const Mat l_estimatedK = getKMap();
    Mat l_errorOfEstimateK = _answerK.clone();
    absdiff(_answerK, l_estimatedK, l_errorOfEstimateK);
    Mat l_errorOfEstimateF = _answerF.clone();
    const Mat l_estimatedF = getFMap();
    absdiff(_answerF, l_estimatedF, l_errorOfEstimateF);
    _print3(_answerK.at<Vec3d>(m_printPoint), l_estimatedK.at<Vec3d>(m_printPoint), l_errorOfEstimateK.at<Vec3d>(m_printPoint));
    _print3(_answerF.at<Vec3d>(m_printPoint), l_estimatedF.at<Vec3d>(m_printPoint), l_errorOfEstimateF.at<Vec3d>(m_printPoint));
    
    // show
    l_errorOfEstimateK.convertTo(l_errorOfEstimateK, CV_8UC3, 255);
    l_errorOfEstimateF.convertTo(l_errorOfEstimateF, CV_8UC3);
    Mat l_answerKDrawing = _answerK.clone();
    Mat l_answerF = _answerF.clone() / 255.0;
    getImageDrawingPrintPoint(&l_answerKDrawing);
    MY_IMSHOW9(l_C, _P, _desireC, _answerK, l_answerF, l_errorOfProjection, l_errorOfEstimateK, l_errorOfEstimateF, _CrOfMPC);
    Mat l_errorOfMPC = abs(_errorOfMPC);
    MY_IMSHOW3(l_errorOfMPC, _vrC, l_answerKDrawing);
    showKMap();
    showFMap();
    showCfullMap();
    showC0Map();
    
    // save (error image only)
    Mat l_answerK;
    _answerK.convertTo(l_answerK, CV_8UC3, 255.0);
    l_answerF.convertTo(l_answerF, CV_8UC3);
    Mat l_C_(l_C.size(), CV_8UC3);
    l_C.convertTo(l_C_, CV_8UC3, 255);
//    Mat l_P_(l_P.size(), CV_8UC3);
//    l_P.convertTo(l_P_, CV_8UC3);
    Mat l_desireC(_desireC.size(), CV_8UC3);
    _desireC.convertTo(l_desireC, CV_8UC3, 255);
    std::ostringstream oss;
    MY_IMWRITE9(OUTPUT_DATA_PATH, _num, oss, l_C_, _P, l_desireC, l_errorOfEstimateK, l_errorOfEstimateF, l_errorOfProjection, l_errorOfMPC, _CrOfMPC, _vrC);
    MY_IMWRITE2(OUTPUT_DATA_PATH, _num, oss, l_answerK, l_answerF);
    
    return true;
    
}
///////////////////////////////  other method ///////////////////////////////
bool AppearanceEnhancement::divideImage(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const cv::Scalar& _distance){
    Mat l_src, l_srcLab, l_dstLab1, l_dstLab2;
    _src.convertTo(l_src, CV_32FC3);
    cv::cvtColor(l_src, l_srcLab, CV_BGR2Lab);
    l_dstLab1 = l_srcLab.clone() + _distance;
    l_dstLab2 = l_srcLab.clone() - _distance;
    cv::cvtColor(l_dstLab1, *_dst1, CV_Lab2BGR);
    cv::cvtColor(l_dstLab2, *_dst2, CV_Lab2BGR);
    _dst1->convertTo(*_dst1, CV_64FC3);
    _dst2->convertTo(*_dst2, CV_64FC3);
    return true;
}
bool AppearanceEnhancement::divideImage(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const double& _distance){
    return divideImage(_dst1, _dst2, _src, Scalar(0, _distance, _distance));
}

// 片方をLabのab平面を移動させ，もう一方は先ほどの移動させたものをRGBに持ってきて
// 対象に移動する
bool AppearanceEnhancement::divideImage2(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const cv::Scalar& _distance){
    Mat l_srcRGB, l_srcLab;
    _src.convertTo(l_srcRGB, CV_32FC3);
    cv::cvtColor(l_srcRGB, l_srcLab, CV_BGR2Lab);
    l_srcLab += _distance;
    Mat l_dstRGB1, l_dstRGB2 = l_srcRGB.clone();
    cv::cvtColor(l_srcLab, l_dstRGB1, CV_Lab2BGR);
    l_dstRGB2 += l_srcRGB - l_dstRGB1;
    
    l_dstRGB1.convertTo(*_dst1, CV_64FC3);
    l_dstRGB2.convertTo(*_dst2, CV_64FC3);
    return true;
}
bool AppearanceEnhancement::divideImage2(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const double& _distance){
    //    return divideImage2_(_dst1, _dst2, _src, Scalar(0, _distance, _distance));
    return divideImage2(_dst1, _dst2, _src, Scalar(_distance, _distance, _distance));
}

// rgbで変化させる
bool AppearanceEnhancement::divideImage3(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const cv::Scalar& _rate){
    int rows = _src.rows, cols = _src.cols;
    double l_diff = 0;
    if (isContinuous(*_dst1, *_dst2, _src)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        Vec3d* l_pDst1 = _dst1->ptr<Vec3d>(y);
        Vec3d* l_pDst2 = _dst2->ptr<Vec3d>(y);
        const Vec3d* l_pSrc = _src.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < 3; ++ c) {
                double l_srcNum = l_pSrc[x][c];
                l_diff = (std::min(l_srcNum, 1.0 - l_srcNum)) * _rate[c];
                l_pDst1[x][c] = l_srcNum + l_diff;
                l_pDst2[x][c] = l_srcNum - l_diff;
            }
        }
    }
    return true;
}
bool AppearanceEnhancement::divideImage3(cv::Mat* const _dst1, cv::Mat* const _dst2, const cv::Mat& _src, const double& _rate){
    return divideImage3(_dst1, _dst2, _src, Scalar(_rate, _rate, _rate));
}

bool AppearanceEnhancement::test_CMaxMin(const cv::Mat& _CMax, const cv::Mat& _CMin){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_captureImage(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    const Mat l_diffCMaxMin = _CMax - _CMin;
    const Mat l_stepDiff = l_diffCMaxMin / 255.0;
    Mat l_CestPrj = _CMin;
    MY_WAIT_KEY();
    
    for (int i = 7; i < 8; ++ i) {
        const int l_col1 = i % 2;
        const int l_col2 = (i / 2) % 2;
        const int l_col3 = (i / 4) % 2;
        Vec3b l_mask(l_col3, l_col2, l_col1);
        ostringstream oss;
        oss <<CHECK_CMAX_MIN_FILE_NAME << l_col3 << l_col2 << l_col1 << ".dat";
        cout << oss.str() << endl;
        ofstream ofs(oss.str().c_str());
        
        for (int prj = 0; prj < 256; ++ prj) {
            // capture
            Vec3b l_color(prj * l_mask[0], prj * l_mask[1], prj * l_mask[2]);
            l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_color);
            
            // get mean and standard deviation
            Vec3d l_meanCap(0, 0, 0), l_stddevCap(0, 0, 0), l_meanPrjCap(0, 0, 0), l_stddevPrjCap(0, 0, 0);
            meanStdDev(l_captureImage, l_meanCap, l_stddevCap);
            meanStdDev(l_CestPrj, l_meanPrjCap, l_stddevPrjCap);
            
            // print
            std::cout << prj << "\t";
            _print_gnuplot_color4_l(std::cout, l_meanCap, l_stddevCap, l_meanPrjCap, l_stddevPrjCap);
            ofs << prj << "\t";
            _print_gnuplot_color4_l(ofs, l_meanCap, l_stddevCap, l_meanPrjCap, l_stddevPrjCap);
            
            MY_IMSHOW(l_captureImage);
            // step up
            l_CestPrj += l_stepDiff;
        }
        ofs.close();
    }
    return true;
}

// 見えの強調を天野先生方式で行う
bool AppearanceEnhancement::doAppearanceEnhancementByAmano(void){
    // init
    setCfullMap();
    setC0Map();
    
    //
    ProCam* l_procam = getProCam();
    const Size l_camSize = l_procam->getCameraSize_();
    bool l_loopFlag = true, l_bDenoise = false;//, clearFlag = false;
    int prj = 255, prj2 = 30;
    Scalar l_projectionColor(255, 255, 255, 0);
    Mat l_projectionImage(l_camSize, CV_8UC3, Scalar(prj, prj, prj));
    Mat l_projectionImageBefore(l_camSize, CV_8UC3, Scalar(prj2, prj2, prj2));
//    Mat l_projectionImage2(l_camSize, CV_8UC3, Scalar(prj, prj, prj));
//    Mat l_projectionImageBefore2(l_camSize, CV_8UC3, Scalar(prj2, prj2, prj2));
    Mat l_projectionImage2(l_camSize, CV_8UC3, Scalar(g_maxPrjLuminance));
    Mat l_projectionImageBefore2(l_camSize, CV_8UC3, Scalar(g_minPrjLuminance));
    Mat l_captureImage(l_camSize, CV_64FC3, Scalar(prj, prj, prj));
    Mat l_captureImageBefore(l_camSize, CV_64FC3, Scalar(prj2, prj2, prj2));
    Mat l_targetImage(l_camSize, CV_64FC3, CV_SCALAR_WHITE);
    Mat l_targetImageBefore(l_camSize, CV_64FC3, CV_SCALAR_WHITE);
    Mat l_answerK(l_camSize, CV_64FC3, CV_SCALAR_D_WHITE);
    Mat l_answerF(l_camSize, CV_64FC3, CV_SCALAR_D_BLACK);
    Mat l_errorOfMPC(l_camSize, CV_64FC3, CV_SCALAR_D_BLACK);
    Mat l_CrOfMPC(l_camSize, CV_64FC3, CV_SCALAR_D_BLACK);
    Mat l_virtualC(l_camSize, CV_64FC3, CV_SCALAR_D_BLACK);
    Mat l_idealC(l_camSize, CV_64FC3, CV_SCALAR_D_BLACK);
    Mat l_desireC(l_camSize, CV_64FC3, CV_SCALAR_D_BLACK);
    double l_enhanceRate = 1.3, l_alphaMPC = 0.1;
    int l_estTarget = 0, l_enhanceType = 0, l_stopTime = -1;
    double l_startTime = 0, l_endTime = 0, l_fps = 0;
    int l_frameNum = 0;
    bool l_calcNextPrj = true;
    Scalar l_prjColor = CV_SCALAR_WHITE, l_currentColor = CV_SCALAR_D_WHITE, l_prjColor2 = CV_SCALAR_BLACK;
    double l_prjLuminance = 255.0, l_prjLuminance2 = 0.0;
    double l_divideSize = 10, l_divideRate = 1.0;
    
    // init
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionImage);

    // loop
    while (l_loopFlag) {
        const Mat l_CMax = getCfullMap();
        const Mat l_CMin = getC0Map();

        // estimate
        const Mat l_KMapBefore = getKMap(), l_FMapBefore = getFMap();
        l_prjColor = l_currentColor * l_prjLuminance;
        l_prjColor2 = l_currentColor * l_prjLuminance2;
        switch (l_estTarget) {
            case 0:
                cout << "estimate K" << endl;
                estimateK(l_projectionImage, l_captureImage, l_CMax, l_CMin, l_answerF, &l_idealC);
                break;
            case 1:
                cout << "estimate F" << endl;
                estimateF(l_projectionImage, l_captureImage, l_CMax, l_CMin);
                break;
            case 2:
                cout << "estimate K and F by Amano model" << endl;
                if (!l_calcNextPrj) {
                    l_projectionImage2 = l_prjColor;
                    l_projectionImageBefore2 = l_prjColor2;
                }
                estimateKFByAmanoModel(l_projectionImage2, l_projectionImageBefore2);
                break;
            case 3:
                cout << "estimate K and F by Fujii model" << endl;
                if (!l_calcNextPrj) {
                    l_projectionImage2 = l_prjColor;
                }
                estimateKFByFujiiModel(l_projectionImage2, l_projectionImageBefore2);
                break;
            default:
                break;
        }
        
        // show
        const Mat l_KMap = getKMap(), l_FMap = getFMap();
        
        // determine target image
        l_targetImageBefore = l_targetImage;
        calcTargetImage(&l_targetImage, l_KMap, l_FMap, l_CMin, l_enhanceRate, l_enhanceType);

        // To shift time
        l_projectionImageBefore = l_projectionImage.clone();
        l_captureImageBefore = l_captureImage.clone();

        // calc next projection image
        if (isAmanoMode()) {
            if (l_calcNextPrj) {
                calcNextProjectionImage(&l_projectionImage, &l_errorOfMPC, &l_CrOfMPC, &l_virtualC, &l_desireC, l_targetImage, l_targetImageBefore, l_captureImageBefore, l_projectionImageBefore, l_KMap, l_FMap, l_FMapBefore, l_CMax, l_CMin, l_alphaMPC);
            } else {
                l_projectionImage = l_prjColor;
            }
            
            // projection
            l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionImage);
        } else {    // Shimizu mode
            Mat l_target1 = l_targetImage.clone(), l_target2 = l_targetImage.clone();
            Mat l_target = l_targetImage.clone() / 255.0;
//            divideImage(&l_target1, &l_target2, l_target, l_divideSize);
            divideImage3(&l_target1, &l_target2, l_target, l_divideRate);
            l_target1 *= 255.0;
            l_target2 *= 255.0;
            
            // calc next projection image
            calcNextProjectionImage(&l_projectionImage2, &l_errorOfMPC, &l_CrOfMPC, &l_virtualC, &l_desireC, l_target1, l_targetImageBefore, l_captureImageBefore, l_projectionImageBefore, l_KMap, l_FMap, l_FMapBefore, l_CMax, l_CMin, l_alphaMPC);
            calcNextProjectionImage(&l_projectionImageBefore2, &l_errorOfMPC, &l_CrOfMPC, &l_virtualC, &l_desireC, l_target2, l_targetImageBefore, l_captureImageBefore, l_projectionImageBefore, l_KMap, l_FMap, l_FMapBefore, l_CMax, l_CMin, l_alphaMPC);
            MY_IMSHOW4(l_target1, l_target2, l_projectionImage2, l_projectionImageBefore2);
            
            // projection
            while (true) {
                l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionImage2);
                l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionImageBefore2);
                if (waitKey(30) == CV_BUTTON_DELETE) {
                    break;
                }
            }
        }
        
        // save and show
        saveAll(l_frameNum);
        showAll(l_frameNum, l_captureImage, l_projectionImage, l_desireC, l_answerK, l_answerF, l_errorOfMPC, l_CrOfMPC, l_virtualC);
        
        // evaluate
        evaluateEstimationAndProjection(l_answerK, l_KMap, l_answerF, l_FMap, l_targetImage, l_captureImage, l_desireC);

        // calc time
        getFps(&l_startTime, &l_endTime, &l_fps);
        cout << l_fps << " fps" << endl;
        
        // check key
        int l_key = waitKey(l_stopTime);
        switch (l_key) {
            // what does calibrate
            case (CV_BUTTON_p):
                cout << "本当にプロジェクタを線形化しますか？(y/n)" << endl;
                if (yes_no()) {
                    l_procam->linearizeOfProjector(true, false);
                }
                break;
            case (CV_BUTTON_g):
                if (l_calcNextPrj) {
                    l_procam->geometricCalibration();
                } else {
                    l_currentColor = CV_SCALAR_D_GREEN;
                    _print(l_currentColor);
                    break;
                }
            case (CV_BUTTON_c): // have to get Cfull and C0 after color calibration
//                l_procam->colorCalibration();
                l_procam->colorCalibration3();
            case (CV_BUTTON_f):
                setCfullMap();
                setC0Map();
                break;
            // what is target to estimate
            case (CV_BUTTON_r):
                if (l_calcNextPrj) {
                    l_estTarget = 0;
                } else {
                    l_currentColor = CV_SCALAR_D_RED;
                    _print(l_currentColor);
                }
                break;
            case (CV_BUTTON_l):
                l_estTarget = 1;
                break;
            case (CV_BUTTON_R):
                if (l_calcNextPrj) {
                    l_estTarget = 2;
                } else {
                    l_currentColor = CV_SCALAR_D_RED;
                    l_prjLuminance = 255.0 / 2.0;
                    _print2(l_currentColor, l_prjLuminance);
                }
                break;
            case (CV_BUTTON_L):
                l_estTarget = 3;
                break;
            // what control
            case (CV_BUTTON_a):
                // 現在推定している反射率と環境光を正解にする
                cout << "l_answerK = l_KMap, l_answerF = l_FMap" << endl;
                l_answerK = l_KMap.clone();
                l_answerF = l_FMap.clone();
                break;
            case (CV_BUTTON_A):
                // 現在推定している環境光を正解にする
                cout << "l_answerF = l_FMap" << endl;
                l_answerF = l_FMap.clone();
                break;
            case (CV_BUTTON_e):
                l_enhanceRate += 0.1;
                _print(l_enhanceRate);
                break;
            case (CV_BUTTON_E):
                l_enhanceRate -= 0.1;
                _print(l_enhanceRate);
                break;
            case (CV_BUTTON_UP):
                if (l_calcNextPrj) {
                    l_alphaMPC += 0.1;
                    _print(l_alphaMPC);
                } else {
                    l_prjLuminance2 = std::min(l_prjLuminance2 + 50, 255.0);
                    _print(l_prjLuminance2);
                }
                break;
            case (CV_BUTTON_DOWN):
                if (l_calcNextPrj) {
                    l_alphaMPC -= 0.1;
                    _print(l_alphaMPC);
                } else {
                    l_prjLuminance2 = std::min(l_prjLuminance2 - 50, 255.0);
                    _print(l_prjLuminance2);
                }
                break;
            case (CV_BUTTON_RIGHT):
                l_prjLuminance = std::min(l_prjLuminance + 50, 255.0);
                _print(l_prjLuminance);
                break;
            case (CV_BUTTON_LEFT):
                l_prjLuminance = std::max(l_prjLuminance - 50, 0.0);
                _print(l_prjLuminance);
                break;
            case (CV_BUTTON_v):
                l_divideSize += 10;
                l_divideRate = std::min(l_divideRate+50/255.0, 1.0);
                _print2(l_divideSize, l_divideRate);
                break;
            case (CV_BUTTON_V):
                l_divideSize -= 10;
                l_divideRate = std::max(l_divideRate-50/255.0, 0.0);
                _print2(l_divideSize, l_divideRate);
                break;
            // what is type of enhancement
            case (CV_BUTTON_2):
                l_enhanceType = 0;
                l_enhanceRate = 1.3;
                break;
            case (CV_BUTTON_3):
                l_enhanceType = 0;
                l_enhanceRate = -1.0;
                break;
            case (CV_BUTTON_4):
                l_enhanceType = 1;
                break;
            case (CV_BUTTON_9):
                l_procam->saveRelationI2C();
                break;
            // check calibration
            case (CV_BUTTON_t):
                l_procam->test_colorCalibration();
                break;
            case (CV_BUTTON_T):
                l_procam->test_linearizeOfProjector();
                break;
            case (CV_BUTTON_q):
                cout << "check estimate K start" << endl;
                test_estimateK(l_answerK, l_CMax, l_CMin);
                cout << "check estimate K finish" << endl;
                break;
            case (CV_BUTTON_Q):
                cout << "check estimate KF start" << endl;
                test_estimateKFByAmanoModel(l_answerK);
                cout << "check estimate KF finish" << endl;
                break;
            case (CV_BUTTON_w):
                if (l_calcNextPrj) {
                    l_answerK = Scalar(1.0, 1.0, 1.0, 0.0);
                    l_answerF = Scalar(0, 0, 0, 0);
                    test_calcNextProjectionImage(l_answerK, l_answerF, l_CMax, l_CMin);
                } else {
                    l_currentColor = CV_SCALAR_D_WHITE;
                    _print(l_currentColor);
                }
                break;
            case (CV_BUTTON_W):
                if (l_calcNextPrj) {
                    cout << "check CMax and CMin" << endl;
                    test_CMaxMin(l_CMax, l_CMin);
                } else {
                    l_currentColor = CV_SCALAR_D_WHITE;
                    l_prjLuminance = 255.0 / 2.0;
                    _print2(l_currentColor, l_prjLuminance);
                }
                break;
            // set next projection color
            case (CV_BUTTON_G):
                l_currentColor = CV_SCALAR_D_RED;
                l_prjLuminance = 255.0 / 2.0;
                _print2(l_currentColor, l_prjLuminance);
                break;
            case (CV_BUTTON_b):
                l_currentColor = CV_SCALAR_D_BLUE;
                _print(l_currentColor);
                break;
            case (CV_BUTTON_B):
                l_currentColor = CV_SCALAR_D_BLUE;
                l_prjLuminance = 255.0 / 2.0;
                _print2(l_currentColor, l_prjLuminance);
                break;
            case (CV_BUTTON_k):
                l_currentColor = CV_SCALAR_D_WHITE;
                l_prjLuminance = 0.0;
                _print2(l_currentColor, l_prjLuminance);
                break;
            // other
            case (CV_BUTTON_s):
                if (l_stopTime == -1) {
                    l_stopTime = 30;
                } else {
                    l_stopTime = -1;
                }
                break;
            case (CV_BUTTON_S):
                l_procam->settingProjectorAndCamera();
                break;
            case (CV_BUTTON_d):
                // denoise flag
                l_procam->switchDenoiseFlag();
                break;
            case (CV_BUTTON_P):
                l_calcNextPrj = !l_calcNextPrj;
                if (l_calcNextPrj) {
                    cout << "l_calcNextPrj on" << endl;
                } else {
                    cout << "l_calcNextPrj off" << endl;
                }
                break;
            case (CV_BUTTON_m): // mode switching
                switchMode();
                break;
            case (CV_BUTTON_DELETE):
                cout << "all clean" << endl;
                initK(l_camSize);
                initF(l_camSize);
//                l_projectionImage = Mat(l_camSize, CV_8UC3, CV_SCALAR_WHITE);
//                l_projectionImage = Scalar(g_maxPrjLuminance);
                l_projectionImage = Scalar(125, 125, 125);
                l_projectionImage2 = Scalar(g_maxPrjLuminance);
                l_projectionImageBefore2 = Scalar(g_minPrjLuminance);
                l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionImage, l_bDenoise);
//                l_targetImage = Mat(l_camSize, CV_64FC3, CV_SCALAR_WHITE);
                l_targetImage = CV_SCALAR_D_WHITE;
                l_answerK = CV_SCALAR_D_WHITE;
                l_answerF = CV_SCALAR_D_BLACK;
                prj = 255;
                break;
            case (CV_BUTTON_ESC):
                l_loopFlag = false;
                break;
            default:
                cout << "you push " << (int)l_key << " key" << endl;
                break;
        }
        
        l_frameNum ++;
    }
    
    return true;
}
