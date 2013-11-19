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
:m_K(3, 1, CV_8UC1), m_F(3, 1, CV_8UC1), m_Cfull(3, 1, CV_8UC1), m_C0(3, 1, CV_8UC1)
{
//    setCfull(_CMaxNum);
//    setC0(_CMinNum);
//    test_RadiometricModel();
    srand((unsigned) time(NULL));
}
AppearanceEnhancement::AppearanceEnhancement(const cv::Size& _prjSize)
:m_procam(_prjSize){
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
    m_KMap = Mat(_camSize, CV_64FC3, Scalar(1.0, 1.0, 1.0));
    
    return true;
}

// 環境光(F)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initF(const cv::Size& _camSize){
    m_FMap = Mat(_camSize, CV_64FC3, Scalar(1.0, 1.0, 1.0));

    return true;
}

// 最大輝度撮影色(Cfull)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initCfull(const cv::Size& _camSize){
    // init Cfull map
    m_CfullMap = Mat(_camSize, CV_64FC3, Scalar(1.0, 1.0, 1.0));
    return true;
}

// 最小輝度撮影色(C0)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initC0(const cv::Size& _camSize){
    // init C0 map
    m_C0Map = Mat(_camSize, CV_64FC3, Scalar(1.0, 1.0, 1.0));
    return true;
}

bool AppearanceEnhancement::initProCam(void){
//    Size prjSize(PRJ_SIZE);
//    m_procam(PRJ_SIZE);
    m_procam.allCalibration();
//    procam.doRadiometricCompensation(100);
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
bool AppearanceEnhancement::setCfullMap(const bool _denoisingFlag){
    ProCam* l_procam = getProCam();
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&m_CfullMap, 255, _denoisingFlag, SLEEP_TIME);
    saveCfull();
    showCfullMap();
    return true;
}
bool AppearanceEnhancement::setCfullMap(const cv::Mat& _Cfull){
    m_CfullMap = _Cfull.clone();
    return true;
}


// m_C0Mapの設定
// input / C0   : 設定する色
// return       : 成功したかどうか
bool AppearanceEnhancement::setC0Map(const bool _denoisingFlag){
    ProCam* l_procam = getProCam();
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&m_C0Map, 0, _denoisingFlag, SLEEP_TIME);
    saveC0();
    showC0Map();
    return true;
}
bool AppearanceEnhancement::setC0Map(const cv::Mat& _C0){
    m_C0Map = _C0.clone();
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

// m_C0の取得
bool AppearanceEnhancement::getC0(cv::Mat* const C0){
    *C0 = m_C0;
    return true;
}

// m_C0Mapの取得
const cv::Mat& AppearanceEnhancement::getC0Map(void){
    return m_C0Map;
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

///////////////////////////////  print method ///////////////////////////////
// 光学モデルの１から２５５までの平均・標準偏差を出力する
bool AppearanceEnhancement::printStandardDeviationOfRadiometricModel(void){
    // init
    Mat ansK = Mat::eye(3, 3, CV_64FC1) * 0.5;
    Mat ansF = Mat::ones(3, 1, CV_64FC1) * 0.1;
    Mat ansK2 = Mat::eye(3, 3, CV_64FC1) * 0.5;
    Mat ansF2 = Mat::ones(3, 1, CV_64FC1) * 0.1;
    Mat estK = Mat::zeros(3, 3, CV_64FC1);
    Mat estF = Mat::zeros(3, 1, CV_64FC1);
    Mat estOnlyK = Mat::zeros(3, 3, CV_64FC1);
    Mat estOnlyF = Mat::zeros(3, 1, CV_64FC1);
    Mat P1 = Mat::ones(3, 1, CV_64FC1);
    Mat P2 = Mat::zeros(3, 1, CV_64FC1);
    Mat C1 = Mat::zeros(3, 1, CV_64FC1);
    Mat C2 = Mat::zeros(3, 1, CV_64FC1);
    const int sampling = 256;
    const double step = 1.0 / sampling;
    //    const int cstStep = 200;
    const int variance_sampling = 100;
    
    //    cout << "index\tansK\tansK2\testK\testOnlyK\tansF\tansF2\testF\testOnlyF" << endl;
    ofstream ofs("output.txt");
    for (int i = 0; i <= sampling; ++ i) {
        // init
        vector<double> vec[4];
        double vec_ave[4] = {0, 0, 0, 0};
        double vec_var[4] = {0, 0, 0, 0};
        double vec_sb[4] = {0, 0, 0, 0};
        double index = step * (double)i;
//        ansK = Mat::eye(3, 3, CV_64FC1) * index;
//        ansK2 = Mat::eye(3, 3, CV_64FC1) * index;
        ansF = Mat::ones(3, 1, CV_64FC1) * index;
        ansF2 = Mat::ones(3, 1, CV_64FC1) * index;
//        ansK = Mat::eye(3, 3, CV_64FC1) * 0;
//        ansK2 = Mat::eye(3, 3, CV_64FC1) * 0;
        //        double up = 0.11, dw = 0.09;
        //        double index_comp = (up - dw) * index + dw;
//        setColor(&P1, index);
        //        setColor(&P1, step * cstStep);
        
        for (int j = 0; j < variance_sampling; ++ j) {
//            calcIdealCamera(&C1, ansK, ansF, P1);
//            calcIdealCamera(&C2, ansK, ansF, P2);
            calcCameraAddedNoise(&C1, ansK, ansF, P1, NOISE_RANGE);
            calcCameraAddedNoise(&C2, ansK2, ansF2, P2, NOISE_RANGE);
//            calcCameraAddedFixNoise(&C1, ansK, ansF, P1, NOISE_RANGE);
//            calcCameraAddedFixNoise(&C2, ansK, ansF, P2, NOISE_RANGE);
            
            // 反射率と環境光を計算
            calcReflectAndAmbient(&estK, &estF, P1, C1, P2, C2);
            calcReflect(&estOnlyK, P1, C1, ansF);
            calcAmbient(&estOnlyF, P1, C1, ansK);
            
            // 推定値をベクターに入れる
            vec[0].push_back(estK.at<double>(0, 0));
            vec[1].push_back(estF.at<double>(0, 0));
            vec[2].push_back(estOnlyK.at<double>(0, 0));
            vec[3].push_back(estOnlyF.at<double>(0, 0));
//            _print(estK);
        }
        
        // calc average, variance and standard derivation
        for (int k = 0; k < 4; ++ k) {
            vec_ave[k] = mean(vec[k]);
            vec_var[k] = var(vec[k]);
            vec_sb[k] = sd(vec[k]);
        }
        
        // output
        _print_excel9(i, vec_ave[0], vec_sb[0], vec_ave[1], vec_sb[1], vec_ave[2], vec_sb[2], vec_ave[3], vec_sb[3]);
        _print_gnuplot9(ofs, i, vec_ave[0], vec_sb[0], vec_ave[1], vec_sb[1], vec_ave[2], vec_sb[2], vec_ave[3], vec_sb[3]);
    }
    ofs.close();
    
    
    return true;
}

// 光学モデルの繰り返し使用による誤差を出力
bool AppearanceEnhancement::printSwitchIteratorError(void){
    // init
    Mat ansK = Mat::eye(3, 3, CV_64FC1) * 1.0;
    Mat ansF = Mat::ones(3, 1, CV_64FC1) * 0.0;
//    Mat estOnlyK = Mat::zeros(3, 3, CV_64FC1);
//    Mat estOnlyF = Mat::zeros(3, 1, CV_64FC1);
//    Mat estOnlyK = ansK.clone();
//    Mat estOnlyF = ansF.clone();
    Mat estOnlyK = Mat::eye(3, 3, CV_64FC1) * 0.95;
    Mat estOnlyF = Mat::ones(3, 1, CV_64FC1) * 0.05;
    Mat P1 = Mat::zeros(3, 1, CV_64FC1);
    Mat C1 = Mat::zeros(3, 1, CV_64FC1);
    Mat desireC = Mat::ones(3, 1, CV_64FC1) * 0.3;
    roundDesireC(&desireC, ansK, ansF);
    const double loopNum = 1000;
    
    ofstream ofs("output_iterator_onlyK.txt");
    cout << "i\tC1\tdesireC\tP1\tansK\testOnlyK\tansF\testOnlyF" << endl;
    for (int i = 0; i < loopNum; ++ i) {
        Mat C0, Cfull;
        getC0(&C0);
        getCfull(&Cfull);

        // カメラ値を取得
//        if ( !calcCameraAddedNoise(&C1, ansK, ansF, P1, NOISE_RANGE) ) return false;
        if ( !calcIdealCamera(&C1, ansK, ansF, P1) ) return false;
        
        // 反射率か環境光を計算
        if (i % 2 == 1) {
            if ( !calcReflect(&estOnlyK, P1, C1, estOnlyF) ) return false;
        } else {
            if ( !calcAmbient(&estOnlyF, P1, C1, estOnlyK) ) return false;
        }
        
        // 目標値の決定
        desireC = estOnlyK.diag() * 0.5;
        roundDesireC(&desireC, ansK, ansF);
        
        _print_gnuplot_mat7(ofs, i, C1, desireC, P1, ansK, estOnlyK, ansF, estOnlyF);
        _print_gnuplot_mat7(std::cout, i, C1, desireC, P1, ansK, estOnlyK, ansF, estOnlyF);

        // 次の投影値を取得
        if ( !calcNextProjection(&P1, desireC, estOnlyK, estOnlyF) ) return false;
    }
    ofs.close();
    
    return true;
}

//
bool AppearanceEnhancement::printSimultaneousIteratorError(void){
    // init
    Mat ansK = Mat::eye(3, 3, CV_64FC1) * 0.5;
    Mat ansF = Mat::ones(3, 1, CV_64FC1) * 0.2;
    Mat estK = Mat::zeros(3, 3, CV_64FC1);
    Mat estF = Mat::zeros(3, 1, CV_64FC1);
    Mat P1 = Mat::zeros(3, 1, CV_64FC1);
    Mat C1 = Mat::zeros(3, 1, CV_64FC1);
    Mat P2 = Mat::zeros(3, 1, CV_64FC1);
    Mat C2 = Mat::zeros(3, 1, CV_64FC1);
    Mat desireC = Mat::ones(3, 1, CV_64FC1) * 0.3;
    const double loopNum = 200;
    
    ofstream ofs("output_iterator_onlyK.txt");
    
    for (int i = 0; i < loopNum; ++ i) {
        Mat C0, Cfull;
        getC0(&C0);
        getCfull(&Cfull);
        double rate = 0.3;
        if (i > 150) {
            rate = 0.5;
        } else if (i > 120) {
            rate = 0.4;
        }
//        ansK = Mat::eye(3, 3, CV_64FC1) * rate;
//        ansF = Mat::ones(3, 1, CV_64FC1) * rate;
//        desireC = Mat::ones(3, 1, CV_64FC1) * ((double)(i+1) / loopNum);
        
        // カメラ値を取得
        if ( !calcCameraAddedNoise(&C1, ansK, ansF, P1, NOISE_RANGE) ) return false;
        
        // 反射率と環境光を推定
        calcReflectAndAmbient(&estK, &estF, P1, C1, P2, C2);

        // print
        _print_gnuplot_mat8(ofs, i, C1, P1, ansK, estK, ansF, estF, Cfull, C0);
        _print_gnuplot_mat8(std::cout, i, C1, P1, ansK, estK, ansF, estF, Cfull, C0);
        
        // 次の投影値を取得
        P2 = P1;
        C2 = C1;
        if ( !calcNextProjection(&P1, desireC, estK, estF) ) return false;
    }
    ofs.close();
    
    return true;
}

// 天野先生論文のシミュレーション結果を出力
bool AppearanceEnhancement::printAmanoMethod(void){
    // init
    Mat ansK = Mat::eye(3, 3, CV_64FC1) * 1.0;
    Mat ansF = Mat::zeros(3, 1, CV_64FC1);
    Mat estOnlyK = Mat::zeros(3, 3, CV_64FC1);
    Mat P1 = Mat::zeros(3, 1, CV_64FC1);
    Mat C1 = Mat::zeros(3, 1, CV_64FC1);
    Mat desireC = Mat::ones(3, 1, CV_64FC1) * 0.3;
    const double loopNum = 200;
    
    ofstream ofs("output_iterator_onlyK.txt");
    cout << "i\tC1\tdesireC\tP1\tansK\testOnlyK\tCfull\tC0" << endl;
    for (int i = 0; i < loopNum; ++ i) {
        // get C0 and Cfull
        Mat C0, Cfull;
        getC0(&C0);
        getCfull(&Cfull);
        
        // カメラ値を取得
        if ( !calcCameraAddedNoise(&C1, ansK, ansF, P1, NOISE_RANGE) ) return false;
//        if ( !calcIdealCamera(&C1, ansK, ansF, P1) ) return false;
        
        // 反射率を計算
        if ( !calcReflect(&estOnlyK, P1, C1, ansF) ) return false;
        
        desireC = estOnlyK.diag() * 0.1;
        
        _print_gnuplot_mat7(ofs, i, C1, desireC, P1, ansK, estOnlyK, Cfull, C0);
        _print_gnuplot_mat7(std::cout, i, C1, desireC, P1, ansK, estOnlyK, Cfull, C0);
        
        // 次の投影値を取得
        if ( !calcNextProjection(&P1, desireC, estOnlyK, ansF) ) return false;
    }
    ofs.close();
    
    return true;
}

// 見えの強調の数値シミュレーション
bool AppearanceEnhancement::printAppearanceEnhancement(void){
    const double l_CMax = 0.99, l_CMin = 0.02;
    const double l_ansK = 0.5, l_ansF = 0.0;
    double l_estK = 0.0, l_estF = 0.0, l_estFBefore = 0.0;
    double l_P = 0, l_C = 0, l_target = 0.0;
    uchar l_Puchar = 0;

    calcCaptureImageAddNoise(&l_C, l_P, l_ansK, l_ansF, l_CMax, l_CMin);
    cout << (int)l_Puchar << endl;
    _print4(l_C, l_target, l_estK, l_P);
    for (int i = 0; i < 256; ++ i) {
        // estimate K
        l_estFBefore = l_estF;
        calcReflectanceAtPixel(&l_estK, l_C, l_P, l_CMax, l_CMin);
        
        // calc target number
        calcTargetImageAtPixel(&l_target, l_estK, l_estF, l_CMin, l_estK * 1.2);
        l_target /= 255.0;
        
        // calc next projection number
        calcNextProjectionImageAtPixel(&l_Puchar, l_target, l_C, l_P, l_estK, l_estF, l_estFBefore, l_CMax, l_CMin);
        l_P = (double)l_Puchar / 255.0;
        l_P = i / 255.0;
        
        // capture
        calcCaptureImageAddNoise(&l_C, l_P, l_ansK, l_ansF, l_CMax, l_CMin);
        
        // print
//        _print4(l_C, l_target, l_estK, l_P);
//        cout << (int)l_Puchar << endl;
        _print_gnuplot3(std::cout, i, l_estK, l_ansK);
    }
    return true;
}
///////////////////////////////  save method ///////////////////////////////
bool AppearanceEnhancement::saveCfull(const std::string& _fileName){
    const Mat l_constCfull = getCfullMap();
    Mat l_Cfull(l_constCfull.rows, l_constCfull.cols, CV_8UC3, CV_SCALAR_BLACK);
    l_constCfull.convertTo(l_Cfull, CV_8UC3);
    imwrite(_fileName, l_Cfull);
    return true;
}
bool AppearanceEnhancement::saveC0(const std::string& _fileName){
    const Mat l_constC0 = getC0Map();
    Mat l_C0(l_constC0.rows, l_constC0.cols, CV_8UC3, CV_SCALAR_BLACK);
    l_constC0.convertTo(l_C0, CV_8UC3);
    imwrite(_fileName, l_C0);
    return true;
}
bool AppearanceEnhancement::saveK(const std::string& _fileName){
    const Mat l_constK = getKMap();
    Mat l_K(l_constK.rows, l_constK.cols, CV_8UC3, CV_SCALAR_BLACK);
    l_constK.convertTo(l_K, CV_8UC3, 255);
    imwrite(_fileName, l_K);
    return true;
}
bool AppearanceEnhancement::saveF(const std::string& _fileName){
    const Mat l_constF = getFMap();
    Mat l_F(l_constF.rows, l_constF.cols, CV_8UC3, CV_SCALAR_BLACK);
    l_constF.convertTo(l_F, CV_8UC3);
    imwrite(_fileName, l_F);
    return true;
}

bool AppearanceEnhancement::saveAll(const cv::Mat& _C, const cv::Mat& _P, const cv::Mat& _targetC, const std::string& _fileNameC, const std::string& _fileNameP, const std::string& _fileNameTarget){
    // save
    imwrite(_fileNameC, _C);
    imwrite(_fileNameP, _P);
    imwrite(_fileNameTarget, _targetC);
    saveK();
    saveF();
    saveCfull();
    saveC0();
    return true;
}


///////////////////////////////  load method ///////////////////////////////
bool AppearanceEnhancement::loadCfull(const std::string& _fileName){
    const Mat l_Cfull = imread(_fileName);
    return setCfullMap(l_Cfull);
}
bool AppearanceEnhancement::loadC0(const std::string& _fileName){
    const Mat l_C0 = imread(_fileName);
    return setC0Map(l_C0);
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
    
    // 丸め込み
//    roundReflectance(&calcK);
//    roundAmbient(&calcF);
    
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
    roundReflectance(&calcK);
    
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
    
    // round calcF
//    roundAmbient(&calcF);
    
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
    double l_targetImageNum = ((1 + _s) * _K - _s * _KGray) * 255;
//    double l_targetImageNum = ((1 + _s) * _K - _s * _KGray) * (128 + _F + _CMin);
//    roundXtoY(&l_targetImageNum, 0, 255);
//    l_pTargetImage[x][c] = (uchar)l_targetImageNum;
    *_targetImage = l_targetImageNum;

    return true;
}

// 次に投影する画像を決定
bool AppearanceEnhancement::calcNextProjectionImage(cv::Mat* const _nextP, const cv::Mat& _targetImage, const cv::Mat& _C, const cv::Mat& _P, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _FBefore, const cv::Mat& _Cfull, const cv::Mat& _C0, const double& _alpha){
    
    // init
    int rows = _targetImage.rows, cols = _targetImage.cols, channel = _targetImage.channels();
    if (isContinuous(*_nextP, _targetImage, _C, _P, _K, _F, _FBefore, _Cfull, _C0)) {
        cols *= rows;
        rows = 1;
    }
    
    // scaning
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pNextP = _nextP->ptr<Vec3b>(y);
        const Vec3d* l_pTargetImage = _targetImage.ptr<Vec3d>(y);
        const Vec3d* l_pC = _C.ptr<Vec3d>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3d* l_pK = _K.ptr<Vec3d>(y);
        const Vec3d* l_pF = _F.ptr<Vec3d>(y);
        const Vec3d* l_pFBefore = _FBefore.ptr<Vec3d>(y);
        const Vec3d* l_pCfull = _Cfull.ptr<Vec3d>(y);
        const Vec3d* l_pC0 = _C0.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < channel; ++ c) {
                // normalize
                const double l_nTargetImage = l_pTargetImage[x][c] / 255.0;
                const double l_nC = l_pC[x][c] / 255.0;
                const double l_nP = l_pP[x][c] / 255.0;
                const double l_nK = l_pK[x][c];
                const double l_nF = l_pF[x][c] / 255.0;
                const double l_nFBefore = l_pFBefore[x][c] / 255.0;
                const double l_nCfull = l_pCfull[x][c] / 255.0;
                const double l_nC0 = l_pC0[x][c] / 255.0;
                
                // calculation
                calcNextProjectionImageAtPixel(&(l_pNextP[x][c]), l_nTargetImage, l_nC, l_nP, l_nK, l_nF, l_nFBefore, l_nCfull, l_nC0);
            }
        }
    }
//    MY_IMSHOW(*_nextP);
    return true;
}
bool AppearanceEnhancement::calcNextProjectionImageAtPixel(uchar* const _nextP, const double& _targetImage, const double& _C, const double& _P, const double& _K, const double& _F, const double& _FBefore, const double& _Cfull, const double& _C0, const double& _alpha){
    // calculation
//    double l_nNextP = (1 - _alpha) * (l_nTargetImage - l_nC) / (l_nK * (l_nCfull - l_nC0)) + l_nP;
    double l_nNextP = ((1 - _alpha) * ((_targetImage - _C) / _K) - _F + _FBefore) / (_Cfull - _C0) + _P;
//    cout << l_nNextP<<" = ((1 - "<<_alpha<<") * (("<<_targetImage<<" - "<<_C<<") / "<<_K<<") - "<<_F<<" + "<<_FBefore<<") / ("<<_Cfull<<" - "<<_C0<<") + "<<_P << endl;
    round0to1(&l_nNextP);
    
    // unnormalize
    *_nextP = (uchar)(l_nNextP * 255);
    return true;
}

// 反射率の計算
bool AppearanceEnhancement::calcReflectanceAtPixel(double* const _K, const double& _nC, const double& _nP, const double& _nCMax, const double& _nCMin){
    // calc
    double l_nCest = (_nCMax - _nCMin) * _nP + _nCMin;
    // avoid 0
    l_nCest = std::max(l_nCest, 1.0/255.0);
    const double l_nC = std::max(_nC, 1.0/255.0);
    double l_K = l_nC / l_nCest;
//    round0to1(&l_K);
    
//    if (l_K > 2.0) {
//        _print_bar;
//        cout <<l_nCest<<" = ("<<_nCMax<<" - "<<_nCMin<<") * "<<_nP<<" + "<<_nCMin<<endl;
//        cout <<l_K<<" = "<<l_nC<<" / "<<l_nCest<<endl;
//    }
    
    // copy
    *_K = l_K;
    return true;
}
bool AppearanceEnhancement::test_calcReflectanceAtPixel(void){
    double l_CMax = 0.99, l_CMin = 0.1;
    double l_ansK = 0.5, l_ansF = 0.0;
    double l_estK = 0.0, l_estF = 0.0, l_estFBefore = 0.0;
    double l_P = 1, l_C = 0, l_target = 0.0;
    uchar l_Puchar = 0;
    
    calcCaptureImageAddNoise(&l_C, l_P, l_ansK, l_ansF, l_CMax, l_CMin);
    cout << (int)l_Puchar << endl;
    _print4(l_C, l_target, l_estK, l_P);
    for (int i = 0; i < 256; ++ i) {
        // capture
        l_P = 0;//i / 255.0;
//        l_CMin = i / 255.0;// / 10;
        l_CMax = i / 255.0;// / 10;
//        l_ansK = i / 255.0;
        calcCaptureImageAddNoise(&l_C, l_P, l_ansK, l_ansF, l_CMax, l_CMin);

        // estimate K
        l_estFBefore = l_estF;
        calcReflectanceAtPixel(&l_estK, l_C, l_P, l_CMax, l_CMin);
        
        // print
        _print_gnuplot3(std::cout, i, l_estK, l_ansK);
    }
    
    return true;
}
bool AppearanceEnhancement::calcReflectanceAndAmbientLightAtPixel(double* const _K, double* const _F, const double& _nC1, const double& _nP1, const double& _nC2, const double& _nP2, const double& _nCMax, const double& _nCMin){
    // set matrix
    const Mat l_C12 = (Mat_<double>(2, 2) << _nC1, -1, _nC2, -1);
    const Mat l_P12 = (Mat_<double>(2, 2) << _nP1, 1, _nP2, 1);
    const Mat l_Cf0 = (Mat_<double>(2, 1) << (_nCMax - _nCMin), _nCMin);
    
    // calculation
    const Mat l_nKF = l_C12.inv() * l_P12 * l_Cf0;
    
    // set
    // K
    *_K = 1 / l_nKF.at<double>(0, 0);
    // F
    double l_nRoundF = l_nKF.at<double>(1, 0);
//    round0to1(&l_nRoundF);
    *_F = l_nRoundF * 255.0;
    return true;
}
bool AppearanceEnhancement::test_calcReflectanceAndAmbientLightAtPixel(void){
    double l_CMax = 0.99, l_CMin = 0.1;
    double l_ansK = 0.5, l_ansF = 0.0;
    double l_estK = 0.0, l_estF = 0.0;
    double l_P1 = 1, l_C1 = 0, l_P2 = 0, l_C2 = 0;
    ofstream ofs(SIM_ESTIMATE_K_FILE_NAME.c_str());
    
    for (int i = 0; i < 256; ++ i) {
        // capture
        l_CMin = i / 255.0;
        calcCaptureImageAddNoise(&l_C1, l_P1, l_ansK, l_ansF, l_CMax, l_CMin);
        calcCaptureImageAddNoise(&l_C2, l_P2, l_ansK, l_ansF, l_CMax, l_CMin);
        
        // estimate K
        calcReflectanceAndAmbientLightAtPixel(&l_estK, &l_estF, l_C1, l_P1, l_C2, l_P2, l_CMax, l_CMin);
        l_estK *= 255;
        double l_ansK255 = l_ansK * 255;
        
        // print
        _print_gnuplot5(std::cout, i, l_estK, l_ansK255, l_estF, l_ansF);
        _print_gnuplot5(ofs, i, l_estK, l_ansK255, l_estF, l_ansF);
    }
    
    return true;
}

// calc capture image from C, P, K, F, CMax, and CMin
bool AppearanceEnhancement::calcCaptureImageAddNoise(double* const _C, const double& _P, const double& _K, const double& _F, const double& _CMax, const double& _CMin, const double& _noiseRange){
    *_C = _K * ((_CMax - _CMin) * _P + _CMin + _F) + (rand() / (double)RAND_MAX) * 2 * _noiseRange - _noiseRange;
    return true;
}
////////////////////////////// estimate method //////////////////////////////
//
bool AppearanceEnhancement::estimateK(const cv::Mat& _P){
    // get Cfull, C0, F
    const Mat l_Cfull = getCfullMap();
    const Mat l_C0 = getC0Map();
    const Mat l_F = getFMap();
    
    // get C
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_C(*l_camSize, CV_64FC3, CV_SCALAR_BLACK), l_K(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C, _P);
    
    // calc
    int rows = _P.rows, cols = _P.cols;
    if (isContinuous(_P, l_C, l_Cfull, l_C0, l_F, l_K)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3d* l_pK = l_K.ptr<Vec3d>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3d* l_pC = l_C.ptr<Vec3d>(y);
        const Vec3d* l_pCfull = l_Cfull.ptr<Vec3d>(y);
        const Vec3d* l_pC0 = l_C0.ptr<Vec3d>(y);
        const Vec3d* l_pF = l_F.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // calc
            // K = C / {(Cf - C0) * P + C0}
            for (int c = 0; c < 3; ++ c) {
                // normalize
                double l_nC = l_pC[x][c] / 255.0;
                double l_nP = (double)l_pP[x][c] / 255.0;
                double l_nCfull = l_pCfull[x][c] / 255.0;
                double l_nC0 = l_pC0[x][c] / 255.0;
                double l_nF = l_pF[x][c] / 255.0;
                
                // calculation
                l_pK[x][c] = l_nC / ((l_nCfull - l_nC0) * l_nP + l_nC0 + l_nF);
//                round0to1(&l_pK[x][c]);
            }
        }
    }
    
    // set and save
    setKMap(l_K);
//    saveK();
    
    return true;
}
bool AppearanceEnhancement::estimateK(const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _CMax, const cv::Mat& _CMin){
    // init
    int rows = _P.rows, cols = _P.cols;
    Mat l_K(rows, cols, CV_64FC3, CV_SCALAR_BLACK);
    if (isContinuous(_P, _C, _CMax, _CMin, l_K)) {
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
        
        for (int x = 0; x < cols; ++ x) {
            // calc
            // K = C / {(Cf - C0) * P + C0}
            for (int c = 0; c < 3; ++ c) {
                // normalize
                double l_nC = l_pC[x][c] / 255.0;
                double l_nP = (double)l_pP[x][c] / 255.0;
                double l_nCMax = l_pCMax[x][c] / 255.0;
                double l_nCMin = l_pCMin[x][c] / 255.0;
                
                // calculation
                calcReflectanceAtPixel(&(l_pK[x][c]), l_nC, l_nP, l_nCMax, l_nCMin);
            }
        }
    }
    
    // save
    setKMap(l_K);
//    saveK();
    return true;
}
bool AppearanceEnhancement::test_estimateK(const cv::Mat& _answerK, const cv::Mat& _CMax, const cv::Mat& _CMin, const cv::Scalar& _mask){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_projectionImage(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Mat l_captureImage(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Scalar l_mean(0,0,0,0), l_stddev(0,0,0,0);
    Scalar l_meanEst(0,0,0,0), l_stddevEst(0,0,0,0);
    Scalar l_meanAns(0,0,0,0), l_stddevAns(0,0,0,0);
    ofstream ofs(ESTIMATE_K_FILE_NAME.c_str());
    
    for (int i = 0; i < 256; ++ i) {
        // projection and capture
        const Scalar l_prjColor(i * _mask[0], i * _mask[1], i * _mask[2]);
        l_projectionImage = l_prjColor;
        l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionImage);
        
        // estimation
        estimateK(l_projectionImage, l_captureImage, _CMax, _CMin);
        Mat l_estK = getKMap();
        
        // calc
        calcMeanStddevOfDiffImage(&l_mean, &l_stddev, _answerK, l_estK);
        meanStdDev(l_estK, l_meanEst, l_stddevEst);
        meanStdDev(_answerK, l_meanAns, l_stddevAns);
        
        // print
        std::cout << i << "\t";
        _print_gnuplot_color6_l(std::cout, l_mean, l_stddev, l_meanEst, l_stddevEst, l_meanAns, l_stddevAns);
        ofs << i << "\t";
        _print_gnuplot_color6_l(ofs, l_mean, l_stddev, l_meanEst, l_stddevEst, l_meanAns, l_stddevAns);
    }
    return true;
}

// Fの推定
// model : C = K * {(Cf - C0) * P + C0 + F}
bool AppearanceEnhancement::estimateF(const cv::Mat& _P){
    // get Cfull, C0, K
    const Mat l_Cfull = getCfullMap();
    const Mat l_C0 = getC0Map();
    const Mat l_K = getKMap();
    
    // get C
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_C(*l_camSize, CV_64FC3, CV_SCALAR_BLACK), l_F(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C, _P);
    
    // calc
    int rows = _P.rows, cols = _P.cols;
    if (isContinuous(_P, l_C, l_Cfull, l_C0, l_K, l_F)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3d* l_pF = l_F.ptr<Vec3d>(y);
        const Vec3d* l_pK = l_K.ptr<Vec3d>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3d* l_pC = l_C.ptr<Vec3d>(y);
        const Vec3d* l_pCfull = l_Cfull.ptr<Vec3d>(y);
        const Vec3d* l_pC0 = l_C0.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // calc
            // K = C / {(Cf - C0) * P + C0}
            for (int c = 0; c < 3; ++ c) {
                // normalize
                double l_nC = l_pC[x][c] / 255.0;
                double l_nP = (double)l_pP[x][c] / 255.0;
                double l_nCfull = l_pCfull[x][c] / 255.0;
                double l_nC0 = l_pC0[x][c] / 255.0;
                
                // calculation
                double l_nF = l_nC / l_pK[x][c] - ((l_nCfull - l_nC0) * l_nP + l_nC0);
                
                // inverse normalize
//                round0to1(&l_nF);
                l_pF[x][c] = l_nF * 255.0;
            }
        }
    }

    // set and save
    setFMap(l_F);
    saveF();
    
    return true;
}

// estimate K and F
bool AppearanceEnhancement::estimateKFByAmanoModel(const cv::Mat& _P1, const cv::Mat& _P2){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    const Mat l_Cfull = getCfullMap(), l_C0 = getC0Map();
    Mat l_C1(*l_camSize, CV_64FC3, CV_SCALAR_BLACK), l_C2(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C1, _P1, false, SLEEP_TIME * 5);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C2, _P2);
    Mat l_K(*l_camSize, CV_64FC3, CV_SCALAR_BLACK), l_F(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    
    int rows = _P1.rows, cols = _P1.cols;
    if (isContinuous(_P1, _P2, l_C1, l_C2, l_Cfull, l_C0, l_K, l_F)) {
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
        const Vec3d* l_pCfull = l_Cfull.ptr<Vec3d>(y);
        const Vec3d* l_pC0 = l_C0.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < 3; ++ c) {
                // normalize
                const double l_nC1 = l_pC1[x][c] / 255.0;
                const double l_nC2 = l_pC2[x][c] / 255.0;
                const double l_nP1 = l_pP1[x][c] / 255.0;
                const double l_nP2 = l_pP2[x][c] / 255.0;
                const double l_nCfull = l_pCfull[x][c] / 255.0;
                const double l_nC0 = l_pC0[x][c] / 255.0;
                
                // set matrix
//                const Mat l_C12 = (Mat_<double>(2, 2) << l_nC1, -1, l_nC2, -1);
//                const Mat l_P12 = (Mat_<double>(2, 2) << l_nP1, 1, l_nP2, 1);
//                const Mat l_Cf0 = (Mat_<double>(2, 1) << (l_nCfull - l_nC0), l_nC0);
//                
//                // calculation
//                const Mat l_nKF = l_C12.inv() * l_P12 * l_Cf0;
//                
//                // set
//                // K
//                l_pK[x][c] = 1 / l_nKF.at<double>(0, 0);
//                // F
//                double l_nRoundF = l_nKF.at<double>(1, 0);
////                round0to1(&l_nRoundF);
////                l_pF[x][c] = (uchar)(l_nRoundF * 255);
//                l_pF[x][c] = l_nRoundF * 255.0;
                
                calcReflectanceAndAmbientLightAtPixel(&(l_pK[x][c]), &(l_pF[x][c]), l_nC1, l_nP1, l_nC2, l_nP2, l_nCfull, l_nC0);
            }
        }
    }
    
    // set
    setKMap(l_K);
    setFMap(l_F);
    
    // save
//    saveK();
//    saveF();
    
    return true;
}
bool AppearanceEnhancement::test_estimateKFByAmanoModel(const cv::Mat& _answerK, const cv::Scalar& _mask){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_projectionImage1(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Mat l_projectionImage2(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    Scalar l_mean(0,0,0,0), l_stddev(0,0,0,0);
    Scalar l_meanEstK(0,0,0,0), l_stddevEstK(0,0,0,0);
    Scalar l_meanEstF(0,0,0,0), l_stddevEstF(0,0,0,0);
    ofstream ofs(ESTIMATE_KF_FILE_NAME.c_str());
    
    for (int i = 0; i < 256; ++ i) {
        // projection and capture
        const Scalar l_prjColor(i * _mask[0], i * _mask[1], i * _mask[2]);
        l_projectionImage1 = l_prjColor;
        
        // estimation
        estimateKFByAmanoModel(l_projectionImage1, l_projectionImage2);
        Mat l_estK = getKMap(), l_estF = getFMap();
        
        // calc
        calcMeanStddevOfDiffImage(&l_mean, &l_stddev, _answerK, l_estK);
        meanStdDev(l_estK, l_meanEstK, l_stddevEstK);
        meanStdDev(l_estF, l_meanEstF, l_stddevEstF);
        
        // print
        std::cout << i << "\t";
        _print_gnuplot_color6_l(std::cout, l_mean, l_stddev, l_meanEstK, l_stddevEstK, l_meanEstF, l_stddevEstF);
        ofs << i << "\t";
        _print_gnuplot_color6_l(ofs, l_mean, l_stddev, l_meanEstK, l_stddevEstK, l_meanEstF, l_stddevEstF);
    }

    return true;
}

bool AppearanceEnhancement::estimateKFByFujiiModel(const cv::Mat& _P1, const cv::Mat& _P2){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_C1(*l_camSize, CV_64FC3, CV_SCALAR_BLACK), l_C2(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    Mat l_K(*l_camSize, CV_64FC3, CV_SCALAR_BLACK), l_F(*l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C1, _P1, false, SLEEP_TIME * 5);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C2, _P2);
    
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
    
    // save
//    saveK();
//    saveF();

    return true;
}

////////////////////////////// evaluate method //////////////////////////////
bool AppearanceEnhancement::evaluateK(const cv::Mat& _ansK){
    // get difference
    MY_IMSHOW(_ansK);
    Mat l_estK = getKMap();
    Mat l_diffK(l_estK.rows, l_estK.cols, CV_64FC3, CV_SCALAR_BLACK);
    absdiff(_ansK, l_estK, l_diffK);
    MY_IMSHOW(l_diffK);
    
    // show mean and standard deviation
    Scalar l_meanK(0.0, 0.0, 0.0), l_stddevK(0.0, 0.0, 0.0);
    meanStdDev(l_diffK, l_meanK, l_stddevK);
    _print2(l_meanK, l_stddevK);
    return true;
}
bool AppearanceEnhancement::evaluateF(const cv::Mat& _ansF){
    // get difference
    MY_IMSHOW(_ansF);
    Mat l_estF = getKMap();
    Mat l_diffF(l_estF.rows, l_estF.cols, CV_64FC3, CV_SCALAR_BLACK);
    absdiff(_ansF, l_estF, l_diffF);
    MY_IMSHOW(l_diffF);
    
    // show mean and standard deviation
    Scalar l_meanF(0.0, 0.0, 0.0), l_stddevF(0.0, 0.0, 0.0);
    meanStdDev(l_diffF, l_meanF, l_stddevF);
    _print2(l_meanF, l_stddevF);

    return true;
}
bool AppearanceEnhancement::evaluateEstimationAndProjection(const cv::Mat& _ansK, const cv::Mat& _estK, const cv::Mat& _targetImage, const cv::Mat& _captureImage){
    
    // get mean and standard deviation
    cv::Scalar l_estMean(0, 0, 0, 0), l_estStddev(0, 0, 0, 0);
    cv::Scalar l_prjMean(0, 0, 0, 0), l_prjStddev(0, 0, 0, 0);
    calcMeanStddevOfDiffImage(&l_estMean, &l_estStddev, _ansK, _estK);
    calcMeanStddevOfDiffImage(&l_prjMean, &l_prjStddev, _targetImage, _captureImage);
    
    // print
//    _print2(l_estMean, l_estStddev);
//    _print2(l_prjMean, l_prjStddev);
    _print_gnuplot_color4_l(std::cout, l_estMean, l_estStddev, l_prjMean, l_prjStddev);
    
    return true;
}

///////////////////////////////  round method ///////////////////////////////
// desireCを光学モデルを用いて範囲を決め，その範囲に丸める
// outpute / _desireC   : 丸めるdesireC
// input / _K           : 反射率
// input / _F           : 環境光
bool AppearanceEnhancement::roundDesireC(cv::Mat* const _desireC, const cv::Mat& _K, const cv::Mat& _F){
    // 範囲を計算
    const int rows = _desireC->rows, cols = _desireC->cols;
    Mat rangeTop = Mat::zeros(rows, cols, CV_64FC1);
    Mat rangeDown = Mat::zeros(rows, cols, CV_64FC1);
    calcRangeOfDesireC(&rangeTop, &rangeDown, _K, _F);
    
    // 丸め込み
    roundXtoYForMat(_desireC, rangeDown, rangeTop);
    
    return true;
}

// 反射率を丸め込む
bool AppearanceEnhancement::roundReflectance(cv::Mat* const _K){
    // error processing
    Mat l_K = Mat::zeros(3, 3, CV_64FC1);
    if ( isDifferentSize(*_K, l_K) ) {
        cerr << "Mat size is different" << endl;
        ERROR_PRINT2(*_K, l_K);
        return false;
    }
    l_K.release();
    
    // 丸め込み
    round0to1ForMat(_K);
    
    return true;
}

// 環境光を丸め込む
bool AppearanceEnhancement::roundAmbient(cv::Mat* const _F){
    // error processing
    Mat l_F = Mat::zeros(3, 1, CV_64FC1);
    if ( isDifferentSize(*_F, l_F) ) {
        cerr << "Mat size is different" << endl;
        ERROR_PRINT2(*_F, l_F);
        return false;
    }
    l_F.release();
    
    // 範囲の計算
    Mat Cfull;
    getCfull(&Cfull);
    Mat one(3, 1, CV_64FC1, 1.0);
    Mat rangeBottom(3, 1, CV_64FC1, 0.0);
    Mat rangeTop(3, 1, CV_64FC1, 0.0);
    rangeTop = one - Cfull;
    
    // 丸め込み
    roundXtoYForMat(_F, rangeBottom, rangeTop);
    
    return true;
}

///////////////////////////////  show method ///////////////////////////////
bool AppearanceEnhancement::showKMap(void){
    const Mat l_K = getKMap();
    MY_IMSHOW(l_K);
    return true;
}
bool AppearanceEnhancement::showFMap(void){
    Mat l_F = getFMap();
    l_F /= 255.0;
    MY_IMSHOW(l_F);
    return true;
}
bool AppearanceEnhancement::showCfullMap(void){
    const Mat l_Cfull = getCfullMap();
    MY_IMSHOW(l_Cfull);
    return true;
}
bool AppearanceEnhancement::showC0Map(void){
    Mat l_C0 = getC0Map();
    l_C0 /= 255.0;
    MY_IMSHOW(l_C0);
    return true;
}
bool AppearanceEnhancement::showAll(const cv::Mat& _captureImage, const cv::Mat& _projectionImage, const cv::Mat& _targetImage, const cv::Mat& _answerK){
    const Mat l_captureImage = _captureImage / 255.0;
    Mat l_projectorImage = _projectionImage;
    l_projectorImage.convertTo(l_projectorImage, CV_64FC3, 1.0/255.0);
    const Mat l_targetImage = _targetImage / 255.0;
    MY_IMSHOW(l_captureImage);
    MY_IMSHOW(l_projectorImage);
    MY_IMSHOW(l_targetImage);
    MY_IMSHOW(_answerK);
    
    showKMap();
    showFMap();
    
    return true;
    
}
///////////////////////////////  other method ///////////////////////////////
// 光学モデルのテスト ( C=K{(C_full-C_0)P + C_0 + F} )
// return   : テストが合ってるかどうか
bool AppearanceEnhancement::test_RadiometricModel(void){

//    printStandardDeviationOfRadiometricModel();
//    printSwitchIteratorError();
//    printSimultaneousIteratorError();
//    printAmanoMethod();
    
    printAppearanceEnhancement();
    
    return true;
}

// 対象の彩度を強調させる
bool AppearanceEnhancement::doAppearanceCrealy(cv::Mat* const _P, const double _s){
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_desireC(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
//    Mat _P(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    const Mat l_K = getKMap();
    const Mat l_F = getFMap();
    const Mat l_Cfull = getCfullMap();
    const Mat l_C0 = getC0Map();
    const Mat grayTransMat = (Mat_<double>(1, 3) << 0.114478, 0.586611, 0.298912);
    
    int rows = _P->rows, cols = _P->cols;
    if (isContinuous(l_desireC, *_P, l_K, l_F, l_Cfull, l_C0)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pDesireC = l_desireC.ptr<Vec3b>(y);
        Vec3b* l_pP = _P->ptr<Vec3b>(y);
        const Vec3d* l_pK = l_K.ptr<Vec3d>(y);
        const Vec3b* l_pF = l_F.ptr<Vec3b>(y);
        const Vec3b* l_pCfull = l_Cfull.ptr<Vec3b>(y);
        const Vec3b* l_pC0 = l_C0.ptr<Vec3b>(y);

        for (int x = 0; x < cols; ++ x) {
            // calc gray image
//            const Mat l_matK = (Mat_<double>(3, 1) << l_pK[x][0] ,l_pK[x][4] ,l_pK[x][8]);
            const Mat l_matK = Mat(l_pK[x]);
            const Mat l_gray = grayTransMat * l_matK;
            
            for (int c = 0; c < 3; ++ c) {
                // normalize
                const double l_nF = l_pF[x][c] / 255.0;
                const double l_nCfull = l_pCfull[x][c] / 255.0;
                const double l_nC0 = l_pC0[x][c] / 255.0;
                
                // calc desire C
                const double l_nDesireColor = (1 + _s) * l_matK.at<double>(c, 0) - _s * l_gray.at<double>(0, 0);
                double l_roundDesireColor = l_nDesireColor * 255;
                roundXtoY(&l_roundDesireColor, 0, 255);
                l_pDesireC[x][c] = (uchar)l_roundDesireColor;
                
                // calc P
                // C=K{(Cf-C0)P+C0+F}
                // P = (C/K - C0 - F) / (Cf-C0)
//                const double l_nP = (l_desireColor / l_pK[x][c+c*3] - (l_nC0 + l_nF)) / (l_nCfull - l_nC0);
                const double l_nP = (l_nDesireColor / l_matK.at<double>(c, 0) - (l_nC0 + l_nF)) / (l_nCfull - l_nC0);
                
                double l_roundP = l_nP * 255;
                roundXtoY(&l_roundP, 0, 255);
                l_pP[x][c] = (uchar)l_roundP;
            }
        }
    }
    
    // projection
    Mat l_C(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C, *_P);
    
    // show
    Mat l_diffC = l_desireC.clone();
    absdiff(l_desireC, l_C, l_diffC);
    Vec3d l_diffAverage(0,0,0);
    calcAverageOfImage_d(&l_diffAverage, l_diffC);
    _print(l_diffAverage);
    MY_IMSHOW(l_desireC);
    MY_IMSHOW(*_P);
    MY_IMSHOW(l_C);
    MY_IMSHOW(l_diffC);
    
    return true;
}

// 見えの強調を天野先生方式で行う
bool AppearanceEnhancement::doAppearanceEnhancementByAmano(void){
    // init
    setCfullMap();
    setC0Map();
    Mat l_CMax = getCfullMap();
    Mat l_CMin = getC0Map();
    
    //
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    bool loopFlag = true;//, clearFlag = false;
    int prj = 255, prj2 = 30;
    Mat l_projectionImage(*l_camSize, CV_8UC3, Scalar(prj, prj, prj));
    Mat l_projectionImageBefore(*l_camSize, CV_8UC3, Scalar(prj2, prj2, prj2));
    Mat l_projectionImage2(*l_camSize, CV_8UC3, Scalar(prj, prj, prj));
    Mat l_projectionImageBefore2(*l_camSize, CV_8UC3, Scalar(prj2, prj2, prj2));
    Mat l_captureImage(*l_camSize, CV_64FC3, Scalar(prj, prj, prj));
    Mat l_captureImageBefore(*l_camSize, CV_64FC3, Scalar(prj2, prj2, prj2));
    Mat l_targetImage(*l_camSize, CV_64FC3, CV_SCALAR_WHITE);
    Mat l_answerK(*l_camSize, CV_64FC3, CV_SCALAR_WHITE);
    Mat l_answerF(*l_camSize, CV_64FC3, CV_SCALAR_WHITE);
    double l_enhanceRate = 1.3, l_alphaMPC = 0.1;
    int l_estTarget = 0;
    
    while (loopFlag) {
        // estimate
        Mat l_KMapBefore = getKMap(), l_FMapBefore = getFMap();
        switch (l_estTarget) {
            case 0:
                cout << "estimate K" << endl;
                estimateK(l_projectionImage, l_captureImage, l_CMax, l_CMin);
                break;
            case 1:
                cout << "estimate F" << endl;
                estimateF(l_projectionImage);
                break;
            case 2:
                cout << "estimate K and F by Amano model" << endl;
                estimateKFByAmanoModel(l_projectionImage2, l_projectionImageBefore2);
                break;
            case 3:
                cout << "estimate K and F by Fujii model" << endl;
                estimateKFByFujiiModel(l_projectionImage2, l_projectionImageBefore2);
                break;
            default:
                break;
        }
        
        // show
        const Mat l_KMap = getKMap(), l_FMap = getFMap();
        
        // determine target image
        calcTargetImage(&l_targetImage, l_KMap, l_FMap, l_CMin, l_enhanceRate, 0);

        // To shift time
        l_projectionImageBefore = l_projectionImage.clone();
        l_captureImageBefore = l_captureImage.clone();

        // calc next projection image
        calcNextProjectionImage(&l_projectionImage, l_targetImage, l_captureImageBefore, l_projectionImageBefore, l_KMap, l_FMap, l_FMapBefore, l_CMax, l_CMin, l_alphaMPC);
        
        // projection
        l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionImage);
        
        // save and show
        saveAll(l_captureImage, l_projectionImage, l_targetImage);
        showAll(l_captureImage, l_projectionImage, l_targetImage, l_answerK);
        
        // evaluate
        evaluateEstimationAndProjection(l_answerK, l_KMap, l_targetImage, l_captureImage);
        
        // check key
        int key = waitKey(-1);
        switch (key) {
            // what is calibration
            case (CV_BUTTON_g):
                l_procam->geometricCalibration();
            case (CV_BUTTON_c): // have to get Cfull and C0 after color calibration
                l_procam->colorCalibration();
            case (CV_BUTTON_f):
                setCfullMap();
                setC0Map();
                l_CMax = getCfullMap();
                l_CMin = getC0Map();
                break;
            case (CV_BUTTON_C): // have to get Cfull and C0 after color calibration
                l_procam->colorCalibration(true);
            case (CV_BUTTON_F):
                setCfullMap(true);
                setC0Map(true);
                l_CMax = getCfullMap();
                l_CMin = getC0Map();
                break;
            // what is target to estimate
            case (CV_BUTTON_r):
                l_estTarget = 0;
                break;
            case (CV_BUTTON_l):
                l_estTarget = 1;
                break;
            case (CV_BUTTON_R):
                l_estTarget = 2;
                break;
            case (CV_BUTTON_L):
                l_estTarget = 3;
                break;
            // what control
            case (CV_BUTTON_a):
                // 現在推定している反射率を正解にする
                cout << "l_answerK = l_KMap" << endl;
                l_answerK = l_KMap;
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
                l_alphaMPC += 0.1;
                _print(l_alphaMPC);
                break;
            case (CV_BUTTON_DOWN):
                l_alphaMPC -= 0.1;
                _print(l_alphaMPC);
                break;
            case (CV_BUTTON_RIGHT):
                prj = std::min(prj + 10, 255);
                _print2(prj, prj2);
                l_projectionImage2 = Mat(*l_camSize, CV_8UC3, Scalar(prj, prj, prj));
                break;
            case (CV_BUTTON_LEFT):
                prj = std::max(prj - 10, 0);
                _print2(prj, prj2);
                l_projectionImage2 = Mat(*l_camSize, CV_8UC3, Scalar(prj, prj, prj));
                break;
            // what is type of enhancement
            case (CV_BUTTON_2):
                l_enhanceRate = 1.3;
                break;
            case (CV_BUTTON_3):
                l_enhanceRate = -1.0;
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
                test_estimateK(l_answerK, l_CMax, l_CMin, Scalar(1, 1, 1));
                cout << "check estimate K finish" << endl;
                break;
            case (CV_BUTTON_Q):
                cout << "check estimate KF start" << endl;
                test_estimateKFByAmanoModel(l_answerK);
                cout << "check estimate KF finish" << endl;
                break;
            // other
            case (CV_BUTTON_DELETE):
                cout << "all clean" << endl;
                initK(*l_camSize);
                initF(*l_camSize);
                l_projectionImage = Mat(*l_camSize, CV_8UC3, CV_SCALAR_WHITE);
                l_captureImage = Mat(*l_camSize, CV_64FC3, CV_SCALAR_WHITE);
                l_targetImage = Mat(*l_camSize, CV_64FC3, CV_SCALAR_WHITE);
                prj = 255;
                break;
            case (CV_BUTTON_ESC):
                loopFlag = false;
                break;
            default:
                cout << "you push " << (int)key << " key" << endl;
                break;
        }
    }
    
    return true;
}
