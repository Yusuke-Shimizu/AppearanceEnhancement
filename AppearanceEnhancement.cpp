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
#include "myMath.h"

using namespace std;
using namespace cv;

///////////////////////////////  constructor ///////////////////////////////
AppearanceEnhancement::AppearanceEnhancement(void){
    init();
}
///////////////////////////////  denstructor ///////////////////////////////
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

// 最大輝度撮影色の設定
// input / Cfull    : 設定する色
// return           : 成功したかどうか
bool AppearanceEnhancement::setCfull(const cv::Mat& Cfull){
    m_Cfull = Cfull;
    return true;
}

// 最小輝度撮影色の設定
// input / C0   : 設定する色
// return       : 成功したかどうか
bool AppearanceEnhancement::setC0(const cv::Mat& C0){
    m_C0 = C0;
    return true;
}

///////////////////////////////  get method ///////////////////////////////
// m_Cfullの取得
bool AppearanceEnhancement::getCfull(cv::Mat* Cfull){
    *Cfull = m_Cfull;
    return true;
}

// m_C0の取得
bool AppearanceEnhancement::getC0(cv::Mat* C0){
    *C0 = m_C0;
    return true;
}

///////////////////////////////  init method ///////////////////////////////
// 全体の初期化
bool AppearanceEnhancement::init(void){
    srand((unsigned) time(NULL));
    return initRadiometricModel();
}

// 光学モデルに必要なモデルの初期化
// return   : 成功したかどうか
bool AppearanceEnhancement::initRadiometricModel(void){
    if ( !initP() ) return false;
    if ( !initC() ) return false;
    if ( !initK() ) return false;
    if ( !initF() ) return false;
    if ( !initCfull() ) return false;
    if ( !initC0() ) return false;
    return true;
}

// カメラ色(C)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initC(void){
    Mat C = cv::Mat::zeros(3, 1, CV_64FC1);
    setC(C);
    return true;
}

// プロジェクタ色(P)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initP(void){
    Mat P = cv::Mat::zeros(3, 1, CV_64FC1);
    setP(P);
    return true;
}

// 反射率(K)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initK(void){
    Mat K = cv::Mat::eye(3, 3, CV_64FC1);
    setK(K);
    return true;
}

// 環境光(F)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initF(void){
    Mat F = cv::Mat::zeros(3, 1, CV_64FC1);
    setF(F);
    return true;
}

// 最大輝度撮影色(Cfull)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initCfull(void){
    Mat Cfull = cv::Mat::zeros(3, 1, CV_64FC1);
    setCfull(Cfull);
    return true;
}

// 最小輝度撮影色(C0)の初期化
// return   : 初期化出来たかどうか
bool AppearanceEnhancement::initC0(void){
    Mat C0 = cv::Mat::zeros(3, 1, CV_64FC1);
    setC0(C0);
    return true;
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
bool printData(std::ofstream* ofs, const int index, const cv::Mat& ansK1, const cv::Mat& ansK2, const cv::Mat& estK, const cv::Mat& ansF1, const cv::Mat& ansF2, const cv::Mat& estF){
    //_print_gnuplot7(ofs, index, ansK1.at<double>(0, 0), ansK2.at<double>(0, 0), estK.at<double>(0, 0), ansF1.at<double>(0, 0), ansF2.at<double>(0, 0), estF.at<double>(0, 0));
    //*ofs << "test" << endl;
    return true;
}
bool printData1(std::ofstream* ofs){
    return true;
}
void testtest(std::ostream& ost){
    
}

///////////////////////////////  other method ///////////////////////////////
// 光学モデルのテスト ( C=K{(C_full-C_0)P + C_0 + F} )
// return   : テストが合ってるかどうか
bool AppearanceEnhancement:: test_RadiometricModel(void){
    // init
    Mat ansK = Mat::eye(3, 3, CV_64FC1) * 0.5;
    Mat ansF = Mat::ones(3, 1, CV_64FC1) * 0.1;
    Mat ansK2 = Mat::eye(3, 3, CV_64FC1) * 0.5;
    Mat ansF2 = Mat::ones(3, 1, CV_64FC1) * 0.1;
    Mat estK = Mat::zeros(3, 3, CV_64FC1);
    Mat estF = Mat::zeros(3, 1, CV_64FC1);
    Mat estOnlyK = Mat::zeros(3, 3, CV_64FC1);
    Mat estOnlyF = Mat::zeros(3, 1, CV_64FC1);
    Mat P1 = Mat::zeros(3, 3, CV_64FC1);
    Mat P2 = Mat::zeros(3, 3, CV_64FC1);
    Mat C1 = Mat::zeros(3, 3, CV_64FC1);
    Mat C2 = Mat::zeros(3, 3, CV_64FC1);
    Mat Cfull = Mat::ones(3, 1, CV_64FC1) * 0.8;
    Mat C0 = Mat::ones(3, 1, CV_64FC1) * 0.2;
    setCfull(Cfull);
    setC0(C0);
    Cfull.release();
    C0.release();
    const int sampling = 1000;
    const double step = 1.0 / sampling;
    const int cstStep = 200;
//    const int samplingNum = 100;
    vector<double> vec;

    cout << "index\tansK\tansK2\testK\testOnlyK\tansF\tansF2\testF\testOnlyF" << endl;
    ofstream ofs("output.txt");
    for (int i = 1; i <= sampling; ++ i) {
        double up = 0.11, dw = 0.09;
        double index = step * (double)i;
        double index_comp = (up - dw) * index + dw;
        setColor(&P1, step * (double)i);
//        setColor(&P1, step * cstStep);
//        ansK = Mat::eye(3, 3, CV_64FC1) * index;
//        ansK2 = Mat::eye(3, 3, CV_64FC1) * index;
//        ansF = Mat::eye(3, 1, CV_64FC1) * index;
//        ansF2 = Mat::eye(3, 1, CV_64FC1) * index;
//        ansK2 = Mat::eye(3, 3, CV_64FC1) * ((step * (double)i * (up-dw))+dw);
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
        
//        double diffEstK, diffEstOnlyK, diffEstF, diffEstOnlyF;
//        getAvgOfDiffMat(&diffEstK       , ansK, estK);
//        getAvgOfDiffMat(&diffEstOnlyK   , ansK, estOnlyK);
//        getAvgOfDiffMat(&diffEstF       , ansF, estF);
//        getAvgOfDiffMat(&diffEstOnlyF   , ansF, estOnlyF);
        
        vec.push_back(estK.at<double>(0, 0));
        
        _print_gnuplot9(ofs, index, ansK.at<double>(0, 0), ansK2.at<double>(0, 0), estK.at<double>(0, 0), estOnlyK.at<double>(0, 0), ansF.at<double>(0, 0), ansF2.at<double>(0, 0), estF.at<double>(0, 0), estOnlyF.at<double>(0, 0));
        _print_excel9(index, ansK.at<double>(0, 0), ansK2.at<double>(0, 0), estK.at<double>(0, 0), estOnlyK.at<double>(0, 0), ansF.at<double>(0, 0), ansF2.at<double>(0, 0), estF.at<double>(0, 0), estOnlyF.at<double>(0, 0));
    }
    ofs.close();
    
//    double estKvar = var(vec);
//    _print(estKvar);
    
    return true;
}

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
    
    // get estimated K
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

