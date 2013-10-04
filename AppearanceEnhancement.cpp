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
AppearanceEnhancement::AppearanceEnhancement(const cv::Size& _prjSize)
:m_procam(_prjSize){
    initCfull(Size(1,1));
    initC0(Size(1,1));
//    init();
}
///////////////////////////////  denstructor ///////////////////////////////
///////////////////////////////  init method ///////////////////////////////
// 全体の初期化
bool AppearanceEnhancement::init(void){
    srand((unsigned) time(NULL));
    return initRadiometricModel();
}

// 光学モデルに必要なモデルの初期化
// return   : 成功したかどうか
bool AppearanceEnhancement::initRadiometricModel(void){
    initProCam();
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    if ( !initP() ) return false;
    if ( !initC() ) return false;
    if ( !initK(*l_camSize) ) return false;
    if ( !initF(*l_camSize) ) return false;
    if ( !initCfull(*l_camSize) ) return false;
    if ( !initC0(*l_camSize) ) return false;
    return true;
}

// カメラ色(C)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initC(void){
    m_C = cv::Mat::zeros(3, 1, CV_64FC1);
    return true;
}

// プロジェクタ色(P)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initP(void){
    m_P = cv::Mat::zeros(3, 1, CV_64FC1);
    return true;
}

// 反射率(K)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initK(const cv::Size& _camSize){
    // init K
    m_K = cv::Mat::eye(3, 3, CV_64FC1);
    
    // init K map
//    const Vec9d l_vec(1, 0, 0,
//                      0, 1, 0,
//                      0, 0, 1);
//    m_KMap = Mat_<Vec9d>(_camSize);
//    const int rows = m_KMap.rows, cols = m_KMap.cols;
//    for (int y = 0; y < rows; ++ y) {
//        Vec9d* l_pKMap = m_KMap.ptr<Vec9d>(y);
//        for (int x = 0; x < cols; ++ x) {
//            l_pKMap[x] = l_vec;
//        }
//    }
    m_KMap = Mat(_camSize, CV_64FC3, Scalar(1.0, 1.0, 1.0));
    
    return true;
}

// 環境光(F)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initF(const cv::Size& _camSize){
    m_F = cv::Mat::zeros(3, 1, CV_64FC1);
    
    m_FMap = Mat(_camSize, CV_8UC3, CV_SCALAR_BLACK);

    return true;
}

// 最大輝度撮影色(Cfull)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initCfull(const cv::Size& _camSize){
    // init Cfull
    m_Cfull = cv::Mat::zeros(3, 1, CV_64FC1);
    
    // init Cfull map
    m_CfullMap = Mat(_camSize, CV_8UC3, CV_SCALAR_BLACK);
    
    return true;
}

// 最小輝度撮影色(C0)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initC0(const cv::Size& _camSize){
    // init C0
    m_C0 = cv::Mat::zeros(3, 1, CV_64FC1);

    // init C0 map
    m_C0Map = Mat(_camSize, CV_8UC3, CV_SCALAR_BLACK);
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
// 光学モデルに必要な行列の全体設定
bool AppearanceEnhancement::setRadiometricModel(const cv::Mat& C, const cv::Mat& P, const cv::Mat& K, const cv::Mat& F, const cv::Mat& Cfull, const cv::Mat& C0){
    setC(C);
    setP(P);
    setK(K);
    setF(F);
    setCfull(Cfull);
    setC0(C0);
    return true;
}

// カメラの設定
// input / C    : 設定する色
// return       : 成功したかどうか
bool AppearanceEnhancement::setC(const cv::Mat& C){
    m_C = C;
    return true;
}

// プロジェクタの設定
// input / P    : 設定する色
// return       : 成功したかどうか
bool AppearanceEnhancement::setP(const cv::Mat& P){
    m_P = P;
    return true;
}

// 反射率の設定
// input / K    : 設定する反射率
// return       : 成功したかどうか
bool AppearanceEnhancement::setK(const cv::Mat& K){
    m_K = K;
    return true;
}

// 環境光の設定
// input / F    : 設定する環境光
// return       : 成功したかどうか
bool AppearanceEnhancement::setF(const cv::Mat& F){
    m_F = F;
    return true;
}
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


// 最大輝度撮影色の設定
// input / Cfull    : 設定する色
// return           : 成功したかどうか
bool AppearanceEnhancement::setCfull(const cv::Mat& _Cfull){
    return setColor(&m_Cfull, _Cfull);
}
bool AppearanceEnhancement::setCfull(const double& red, const double& blue, const double& green){
    Mat Cfull = (Mat_<double>(3, 1) << red, blue, green);
    return setCfull(Cfull);
}
bool AppearanceEnhancement::setCfull(const double& _luminance){
    return setCfull(_luminance, _luminance, _luminance);
}

// m_CfullMapの設定
bool AppearanceEnhancement::setCfullMap(void){
    ProCam* l_procam = getProCam();
    l_procam->captureOfProjecctorColorFromLinearFlatGrayLightOnProjectorDomain(&m_CfullMap, 255);
    saveCfull();
    MY_IMSHOW(m_CfullMap);
    return true;
}
bool AppearanceEnhancement::setCfullMap(const cv::Mat& _Cfull){
    m_CfullMap = _Cfull.clone();
    return true;
}


// 最小輝度撮影色の設定
// input / C0   : 設定する色
// return       : 成功したかどうか
bool AppearanceEnhancement::setC0(const cv::Mat& _C0){
    setColor(&m_C0, _C0);
    return true;
}
bool AppearanceEnhancement::setC0(const double& red, const double& blue, const double& green){
    Mat C0 = (Mat_<double>(3, 1) << red, blue, green);
    return setC0(C0);
}
bool AppearanceEnhancement::setC0(const double& luminance){
    return setC0(luminance, luminance, luminance);
}

// m_C0Mapの設定
bool AppearanceEnhancement::setC0Map(void){
    ProCam* l_procam = getProCam();
    l_procam->captureOfProjecctorColorFromLinearFlatGrayLightOnProjectorDomain(&m_C0Map, 0);
    saveC0();
    MY_IMSHOW(m_C0Map);
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
//
/*bool printData(std::ofstream* ofs, const cv::Mat& data){
    // 行列へアクセス
    int rows = data.rows, cols = data.cols;   // 行と列の大きさ
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        const double *dst = data.ptr<double>(y);
        
        for (int x = 0; x < cols; ++ x) {
            *ofs << dst[x] << "\t";
        }
        *ofs << std::endl;
    }
    
    return true;
}*/

//
bool printData(std::ostream& os, const int index, const cv::Mat& _C, const cv::Mat& _P, const cv::Mat& _K){
    _print_gnuplot4(os, index, _C.at<double>(0, 0), _P.at<double>(0, 0), _K.at<double>(0, 0));
//    _print_gnuplot_mat3(os, index, _C, _P, _K);
    return true;
}
bool printData(std::ostream& os, const int index, const cv::Mat& ansK1, const cv::Mat& ansK2, const cv::Mat& estK, const cv::Mat& ansF1, const cv::Mat& ansF2, const cv::Mat& estF){
    _print_gnuplot7(os, index, ansK1.at<double>(0, 0), ansK2.at<double>(0, 0), estK.at<double>(0, 0), ansF1.at<double>(0, 0), ansF2.at<double>(0, 0), estF.at<double>(0, 0));
    return true;
}

// 現在光学モデルで用いている値を出力
void printRadiometricModel(std::ostream& os, const cv::Mat& _C, const cv::Mat& _P, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _Cfull, const cv::Mat& _C0){
    _print_gnuplot6(os, _C.at<double>(0, 0), _P.at<double>(0, 0), _K.at<double>(0, 0), _F.at<double>(0, 0), _Cfull.at<double>(0, 0), _C0.at<double>(0, 0));
}

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
            //        calcIdealCamera(&C1, ansK, ansF, P1);
            //        calcIdealCamera(&C2, ansK, ansF, P2);
            calcCameraAddedNoise(&C1, ansK, ansF, P1, NOISE_RANGE);
            calcCameraAddedNoise(&C2, ansK2, ansF2, P2, NOISE_RANGE);
            //        calcCameraAddedFixNoise(&C1, ansK, ansF, P1, NOISE_RANGE);
            //        calcCameraAddedFixNoise(&C2, ansK, ansF, P2, NOISE_RANGE);
            
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

///////////////////////////////  save method ///////////////////////////////
bool AppearanceEnhancement::saveCfull(const std::string& _fileName){
    const Mat l_Cfull = getCfullMap();
    imwrite(_fileName, l_Cfull);
    return true;
}
bool AppearanceEnhancement::saveC0(const std::string& _fileName){
    const Mat l_C0 = getC0Map();
    imwrite(_fileName, l_C0);
    return true;
}
bool AppearanceEnhancement::saveK(const std::string& _fileName){
    const Mat l_K = getKMap();
    imwrite(_fileName, l_K);
    return true;
}
bool AppearanceEnhancement::saveF(const std::string& _fileName){
    const Mat l_F = getFMap();
    imwrite(_fileName, l_F);
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
    roundAmbient(&calcF);
    
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
bool AppearanceEnhancement::calcTargetImage(cv::Mat* const _targetImage, const cv::Mat& _Cest, const double& _s, const int _enhanceType){
    // init
    int rows = _Cest.rows, cols = _Cest.cols;
    Mat l_targetImage(rows, cols, CV_8UC3, CV_SCALAR_BLACK);
    
    // to be gray scale
    Mat l_grayCest(rows, cols, CV_8UC1, 0);
    cvtColor(_Cest, l_grayCest, CV_BGR2GRAY);
    
    // scan
    if (isContinuous(l_targetImage, _Cest, l_grayCest)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pTargetImage = l_targetImage.ptr<Vec3b>(y);
        const Vec3b* l_pCest = _Cest.ptr<Vec3b>(y);
        const uchar* l_pGrayCest = l_grayCest.ptr<uchar>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < 3; ++ c) {
                // calc desire C
                l_pTargetImage[x][c] = (1 + _s) * (double)l_pCest[x][c] - _s * (double)l_pGrayCest[x];
            }
        }
    }
    
    // copy
    *_targetImage = l_targetImage.clone();

    return true;
}

// 次に投影する画像を決定
bool AppearanceEnhancement::calcNextProjectionImage(cv::Mat* const _nextP, const cv::Mat& _targetImage, const cv::Mat& _C, const cv::Mat& _P, const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _Cfull, const cv::Mat& _C0, const double& _alpha){
    // init
    int rows = _targetImage.rows, cols = _targetImage.cols, channel = _targetImage.channels();
    if (isContinuous(*_nextP, _targetImage, _C, _P, _K, _F, _Cfull, _C0)) {
        cols *= rows;
        rows = 1;
    }
    
    // scaning
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pNextP = _nextP->ptr<Vec3b>(y);
        const Vec3b* l_pTargetImage = _targetImage.ptr<Vec3b>(y);
        const Vec3b* l_pC = _C.ptr<Vec3b>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3d* l_pK = _K.ptr<Vec3d>(y);
        const Vec3b* l_pF = _F.ptr<Vec3b>(y);
        const Vec3b* l_pCfull = _Cfull.ptr<Vec3b>(y);
        const Vec3b* l_pC0 = _C0.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < channel; ++ c) {
                // normalize
                const double l_nTargetImage = l_pTargetImage[x][c] / 255.0;
                const double l_nC = l_pC[x][c] / 255.0;
                const double l_nP = l_pP[x][c] / 255.0;
                const double l_nK = l_pK[x][c];
                const double l_nF = l_pF[x][c] / 255.0;
                const double l_nCfull = l_pCfull[x][c] / 255.0;
                const double l_nC0 = l_pC0[x][c] / 255.0;
                
                // calculation
                const double l_nNextP = (1 - _alpha) * (l_nTargetImage - l_nC) / (l_nK * (l_nCfull - l_nC0)) + l_nP;
                
                // unnormalize
                l_pNextP[x][c] = (uchar)(l_nNextP * 255);
            }
        }
    }
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
    Mat l_C(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C, _P);
    
    // calc
    int rows = _P.rows, cols = _P.cols;
    if (isContinuous(_P, l_C, l_Cfull, l_C0, l_F, m_KMap)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3d* l_pK = m_KMap.ptr<Vec3d>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3b* l_pC = l_C.ptr<Vec3b>(y);
        const Vec3b* l_pCfull = l_Cfull.ptr<Vec3b>(y);
        const Vec3b* l_pC0 = l_C0.ptr<Vec3b>(y);
        const Vec3b* l_pF = l_F.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // calc
            // K = C / {(Cf - C0) * P + C0}
            for (int c = 0; c < 3; ++ c) {
                // normalize
                double l_nC = (double)l_pC[x][c] / 255.0;
                double l_nP = (double)l_pP[x][c] / 255.0;
                double l_nCfull = (double)l_pCfull[x][c] / 255.0;
                double l_nC0 = (double)l_pC0[x][c] / 255.0;
                double l_nF = (double)l_pF[x][c] / 255.0;
                
                // calculation
                l_pK[x][c] = l_nC / ((l_nCfull - l_nC0) * l_nP + l_nC0 + l_nF);
                round0to1(&l_pK[x][c]);
            }
        }
    }
    
    // save
    saveK();
    
    return true;
}
bool AppearanceEnhancement::estimateK(const cv::Mat& _P, const cv::Mat& _C, const cv::Mat& _CMax, const cv::Mat& _CMin){
    // init
    int rows = _P.rows, cols = _P.cols;
    if (isContinuous(_P, _C, _CMax, _CMin, m_KMap)) {
        cols *= rows;
        rows = 1;
    }
    
    // scan
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3d* l_pK = m_KMap.ptr<Vec3d>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3b* l_pC = _C.ptr<Vec3b>(y);
        const Vec3b* l_pCMax = _CMax.ptr<Vec3b>(y);
        const Vec3b* l_pCMin = _CMin.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // calc
            // K = C / {(Cf - C0) * P + C0}
            for (int c = 0; c < 3; ++ c) {
                // normalize
                double l_nC = (double)l_pC[x][c] / 255.0;
                double l_nP = (double)l_pP[x][c] / 255.0;
                double l_nCMax = (double)l_pCMax[x][c] / 255.0;
                double l_nCMin = (double)l_pCMin[x][c] / 255.0;
                
                // calculation
                l_pK[x][c] = l_nC / ((l_nCMax - l_nCMin) * l_nP + l_nCMin);
                round0to1(&l_pK[x][c]);
            }
        }
    }
    
    // save
    saveK();
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
    Mat l_C(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C, _P);
    
    // calc
    int rows = _P.rows, cols = _P.cols;
    if (isContinuous(_P, l_C, l_Cfull, l_C0, l_K, m_FMap)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3b* l_pF = m_FMap.ptr<Vec3b>(y);
        const Vec3d* l_pK = l_K.ptr<Vec3d>(y);
        const Vec3b* l_pP = _P.ptr<Vec3b>(y);
        const Vec3b* l_pC = l_C.ptr<Vec3b>(y);
        const Vec3b* l_pCfull = l_Cfull.ptr<Vec3b>(y);
        const Vec3b* l_pC0 = l_C0.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // calc
            // K = C / {(Cf - C0) * P + C0}
            for (int c = 0; c < 3; ++ c) {
                // normalize
                double l_nC = (double)l_pC[x][c] / 255.0;
                double l_nP = (double)l_pP[x][c] / 255.0;
                double l_nCfull = (double)l_pCfull[x][c] / 255.0;
                double l_nC0 = (double)l_pC0[x][c] / 255.0;
                
                // calculation
                double l_nF = l_nC / l_pK[x][c] - ((l_nCfull - l_nC0) * l_nP + l_nC0);
                
                // inverse normalize
                round0to1(&l_nF);
                l_pF[x][c] = (char)(l_nF * 255.0);
            }
        }
    }

    // save
    saveF();
    
    return true;
}

// estimate K and F
bool AppearanceEnhancement::estimateKFByAmanoModel(const cv::Mat& _P1, const cv::Mat& _P2){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    const Mat l_Cfull = getCfullMap(), l_C0 = getC0Map();
    Mat l_C1(*l_camSize, CV_8UC3, CV_SCALAR_BLACK), l_C2(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C1, _P1, SLEEP_TIME * 5);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C2, _P2);
    
    int rows = _P1.rows, cols = _P1.cols;
    if (isContinuous(_P1, _P2, l_C1, l_C2, l_Cfull, l_C0, m_KMap, m_FMap)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        Vec3d* l_pK = m_KMap.ptr<Vec3d>(y);
        Vec3b* l_pF = m_FMap.ptr<Vec3b>(y);
        const Vec3b* l_pC1 = l_C1.ptr<Vec3b>(y);
        const Vec3b* l_pC2 = l_C2.ptr<Vec3b>(y);
        const Vec3b* l_pP1 = _P1.ptr<Vec3b>(y);
        const Vec3b* l_pP2 = _P2.ptr<Vec3b>(y);
        const Vec3b* l_pCfull = l_Cfull.ptr<Vec3b>(y);
        const Vec3b* l_pC0 = l_C0.ptr<Vec3b>(y);
        
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
                const Mat l_C12 = (Mat_<double>(2, 2) << l_nC1, -1, l_nC2, -1);
                const Mat l_P12 = (Mat_<double>(2, 2) << l_nP1, 1, l_nP2, 1);
                const Mat l_Cf0 = (Mat_<double>(2, 1) << (l_nCfull - l_nC0), l_nC0);
                
                // calculation
                const Mat l_nKF = l_C12.inv() * l_P12 * l_Cf0;
                
                // set
                // K
                l_pK[x][c] = 1 / l_nKF.at<double>(0, 0);
                // F
                double l_nRoundF = l_nKF.at<double>(1, 0);
                round0to1(&l_nRoundF);
                l_pF[x][c] = (uchar)(l_nRoundF * 255);
            }
        }
    }
    
    // save
    saveK();
    saveF();
        
    return true;
}
bool AppearanceEnhancement::estimateKFByFujiiModel(const cv::Mat& _P1, const cv::Mat& _P2){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_C1(*l_camSize, CV_8UC3, CV_SCALAR_BLACK), l_C2(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C1, _P1, SLEEP_TIME * 5);
    l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_C2, _P2);
//    Mat _P1(*l_camSize, CV_8UC3, CV_SCALAR_BLACK), _P2(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
//    l_procam->convertColorSpaceOfProjectorToCamera(&l_P1onC, _P1);
//    l_procam->convertColorSpaceOfProjectorToCamera(&l_P2onC, _P2);
    
    int rows = _P1.rows, cols = _P1.cols;
    if (isContinuous(_P1, _P2, l_C1, l_C2, m_KMap, m_FMap)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        Vec3d* l_pK = m_KMap.ptr<Vec3d>(y);
        Vec3b* l_pF = m_FMap.ptr<Vec3b>(y);
        const Vec3b* l_pC1 = l_C1.ptr<Vec3b>(y);
        const Vec3b* l_pC2 = l_C2.ptr<Vec3b>(y);
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
                round0to1(&l_roundF);
                l_pF[x][c] = (uchar)(l_roundF * 255);
            }
        }
    }

    // save
    saveK();
    saveF();

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
bool AppearanceEnhancement::evaluateKF(void){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    
    // make answer to reflectance
    cout << "Please shut out ambient light" << endl;
    Mat l_ansK(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearFlatGrayLightOnProjectorDomain(&l_ansK, 255);
    
    // make answer to ambient light
    cout << "Please set the standard white board" << endl;
    Mat l_ansF(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->captureOfProjecctorColorFromLinearFlatGrayLightOnProjectorDomain(&l_ansF, 0);
    
    // evaluation
    evaluateK(l_ansK);
    evaluateF(l_ansF);
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
    Mat one = Mat::ones(3, 1, CV_64FC1);
    Mat rangeBottom = Mat::zeros(3, 1, CV_64FC1);
    Mat rangeTop = one - Cfull;
    
    // 丸め込み
    roundXtoYForMat(_F, rangeBottom, rangeTop);
    
    return true;
}

///////////////////////////////  show method ///////////////////////////////
//bool AppearanceEnhancement::showKMap(void){
//    const Mat_<Vec9d> l_KVec9d = getKMap();
//    Mat l_K(l_KVec9d.rows, l_KVec9d.cols, CV_64FC3);
//    getDiagonalImage(&l_K, l_KVec9d);
//    
//    MY_IMSHOW(l_K);
//    
//    return true;
//}

///////////////////////////////  other method ///////////////////////////////
// 光学モデルのテスト ( C=K{(C_full-C_0)P + C_0 + F} )
// return   : テストが合ってるかどうか
bool AppearanceEnhancement::test_RadiometricModel(void){
    setCfull(0.8);
    setC0(0.1);

    printStandardDeviationOfRadiometricModel();
//    printSwitchIteratorError();
//    printSimultaneousIteratorError();
//    printAmanoMethod();
    
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
    Mat l_Cfull = getCfullMap();
    Mat l_C0 = getC0Map();
    
    //
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    bool loopFlag = true, clearFlag = false;
    int prj = 255, prj2 = 30;
    Mat l_projectionImage(*l_camSize, CV_8UC3, Scalar(prj, prj, prj));
    Mat l_projectionImageBefore(*l_camSize, CV_8UC3, Scalar(prj2, prj2, prj2));
    Mat l_captureImage(*l_camSize, CV_8UC3, Scalar(prj, prj, prj));
    Mat l_captureImageBefore(*l_camSize, CV_8UC3, Scalar(prj2, prj2, prj2));
    Mat l_targetImage(*l_camSize, CV_8UC3, CV_SCALAR_WHITE);
    double l_enhanceRate = 1.3;
    int l_estTarget = 0;
    
    while (loopFlag) {
        // estimate
        switch (l_estTarget) {
            case 0:
                cout << "estimate K" << endl;
//                estimateK(l_projectionImage);
                estimateK(l_projectionImage, l_captureImage, l_Cfull, l_C0);
                break;
            case 1:
                cout << "estimate F" << endl;
                estimateF(l_projectionImage);
                break;
            case 2:
                cout << "estimate K and F by Amano model" << endl;
                estimateKFByAmanoModel(l_projectionImage, l_projectionImageBefore);
                break;
            case 3:
                cout << "estimate K and F by Fujii model" << endl;
                estimateKFByFujiiModel(l_projectionImage, l_projectionImageBefore);
                break;
            case 4:
                cout << "evaluate estimated K and F" << endl;
                evaluateKF();
                break;
            default:
                break;
        }
        
        // show
        Mat l_KMap = getKMap(), l_FMap = getFMap();
        MY_IMSHOW(l_KMap);
        MY_IMSHOW(l_FMap);
        
        // print average
        Vec3d l_diffK(0,0,0), l_diffF(0,0,0);
        calcAverageOfImage_d(&l_diffK, l_KMap);
        calcAverageOfImage_d(&l_diffF, l_FMap);
        _print2(l_diffK, l_diffF);
        
        // determine target image
        calcTargetImage(&l_targetImage, l_KMap, l_enhanceRate, 0);

        // To shift time
        l_projectionImageBefore = l_projectionImage.clone();
        l_captureImageBefore = l_captureImage.clone();

        // calc next projection image
        calcNextProjectionImage(&l_projectionImage, l_targetImage, l_captureImageBefore, l_projectionImageBefore, l_KMap, l_FMap, l_Cfull, l_C0);
        
        // projection
        l_procam->captureOfProjecctorColorFromLinearLightOnProjectorDomain(&l_captureImage, l_projectionImage);
        
        // appearance enhance
//        if (clearFlag) doAppearanceCrealy(&l_projectionImage, 1.3);
        
        // check key
        int key = waitKey(-1);
        switch (key) {
            case (CV_BUTTON_g):
                l_procam->geometricCalibration();
            case (CV_BUTTON_c): // have to get Cfull and C0 after color calibration
                l_procam->colorCalibration();
            case (CV_BUTTON_f):
                setCfullMap();
                setC0Map();
                l_Cfull = getCfullMap();
                l_C0 = getC0Map();
                break;
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
            case (CV_BUTTON_a):
                l_estTarget = 4;
                break;
            case (CV_BUTTON_DOWN):
                clearFlag = !clearFlag;
                break;
            case (CV_BUTTON_RIGHT):
                prj = std::min(prj + 10, 255);
                _print2(prj, prj2);
                l_projectionImage = Mat(*l_camSize, CV_8UC3, Scalar(prj, prj, prj));
                break;
            case (CV_BUTTON_LEFT):
                prj = std::max(prj - 10, 0);
                _print2(prj, prj2);
                l_projectionImage = Mat(*l_camSize, CV_8UC3, Scalar(prj, prj, prj));
                break;
            case (CV_BUTTON_DELETE):
                initK(*l_camSize);
                initF(*l_camSize);
                l_projectionImage = Mat(*l_camSize, CV_8UC3, CV_SCALAR_WHITE);
                l_targetImage = Mat(*l_camSize, CV_8UC3, CV_SCALAR_WHITE);
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
