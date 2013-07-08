//
//  LinearizerOfProjector.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/07.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//
/*

 以下の論文を参考にプロジェクタの線形化を行うクラス
 Nayar, Shree K., et al. "A projection system with radiometric compensation for screen imperfections." ICCV Workshop on Projector-Camera Systems (PROCAMS). Vol. 3. 2003.
 
 １．色変換行列を算出
 １．１．３x３の行列を準備
 １．２．対角成分を１に設定
 １．３．RGBと黒を投影・撮影
 １．４．それらの差分より他の成分を決定
 2．プロジェクタの線形化
 ２．１．０から２５５までの値を投影・撮影
 ２．２．それらに対する値のルックアップテーブルを作成

 */

#include "LinearizerOfProjector.h"
#include "myOpenCV.h"
#include "ProCam.h"
#include "common.h"

using namespace std;
using namespace cv;

///////////////////////////////  constructor ///////////////////////////////
LinearizerOfProjector::LinearizerOfProjector(void){
//    initColorMixingMatrix();
}

LinearizerOfProjector::LinearizerOfProjector(ProCam* procam){
//    cout << "setting LinearizerOfProjector..." << endl;
//    _print_name(*procam);
    setProCam(procam);
//    _print_name(*m_procam);
    const cv::Size* cameraSize = m_procam->getCameraSize();
    initColorMixingMatrix(cameraSize->area());
//    cout << "finish LinearizerOfProjector" << endl;
}

///////////////////////////////  destructor ///////////////////////////////
LinearizerOfProjector::~LinearizerOfProjector(void){
//    m_procam->~ProCam();
    cout << "LinearizerOfProjector is deleted (" << this << ")" << endl;
}

///////////////////////////////  set method ///////////////////////////////
// プロカムのセッティング
bool LinearizerOfProjector::setProCam(ProCam* procam){
    m_procam = procam;
//    _print_name(*procam);
    return true;
}

// m_ColorMixingMatrix（色変換行列）の設定
// input / colMix   : 設定する行列
// input / index    : 設定する場所
// return           : 成功したかどうか
bool LinearizerOfProjector::setColorMixMat(const cv::Mat& mat, const int index){
    // error processing
    if (m_aColorMixingMatrix.size() <= index || index < 0) {
        std::cerr << "size is strange number" << std::endl;
        ERROR_PRINT(index);
        return false;
    } else if ( isDifferentSize(mat, m_aColorMixingMatrix.at(index)) ) {
        std::cerr << "mat size is different" << std::endl;
        ERROR_PRINT(mat);
        return false;
    }

    // 代入
    m_aColorMixingMatrix.at(index) = mat;
    
    return true;
}

// m_ColorMixingMatrixの全設定
// input / _aMat    : 設定する配列
// return           : 成功したかどうか
bool LinearizerOfProjector::setColorMixMat(const std::vector<cv::Mat>* _aMat){
    // error processing
    const unsigned long size = m_aColorMixingMatrix.size();
    if (_aMat->size() != size) {
        std::cerr << "vector size is different" << std::endl;
        ERROR_PRINT(size);
        return false;
    }
    
    // assign
    std::vector<cv::Mat>::iterator itrMem = m_aColorMixingMatrix.begin();
    std::vector<cv::Mat>::const_iterator itrArg = _aMat->begin();
    for (; itrMem != m_aColorMixingMatrix.end(); ++ itrMem, ++ itrArg) {
        *itrMem = *itrArg;
    }
    
    return true;
}

// 応答特性マップに一つの深さの応答特性を設定
// output / _responseMap    : 応答特性マップ
// input / _response        : 深さ_depthの応答特性
// input / _depth           : 深さ
// input / _maxDepth        : 深さの最大値（=256）
// return                   : 成功したかどうか
bool LinearizerOfProjector::setResponseMap(cv::Mat_<cv::Vec3b>* const _responseMap, const cv::Mat_<cv::Vec3b>& _response, const int _depth, const int _maxDepth){
    // init
    int rows = _response.rows, cols = _response.cols;
    if ( isContinuous(*_responseMap, _response) ) {
        cols *= rows;
        rows = 1;
    }
    
    for (int y = 0; y < rows; ++ y) {
        cv::Vec3b* p_responseMap = _responseMap->ptr<cv::Vec3b>(y);
        const cv::Vec3b* p_response = _response.ptr<cv::Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            p_responseMap[x * _maxDepth + _depth] = p_response[x];
        }
    }
    return true;
}


///////////////////////////////  get method ///////////////////////////////
// m_ColorMixingMatrix（色変換行列）の取得
// output / colMix  : 色変換行列を入れる行列
// return           : 成功したかどうか
//bool LinearizerOfProjector::getColorMixingMatrix(cv::Mat** colMix){
//    colMix = m_aColorMixingMatrix;
//    return true;
//}

//
bool LinearizerOfProjector::getProCam(ProCam* const procam){
    *procam = *m_procam;
    return true;
}
ProCam* LinearizerOfProjector::getProCam(void){
//    _print_name(*m_procam);
    return m_procam;
}

// m_ColorMixingMatrix（色変換行列）の取得
// input / index    : 取得したい行列の番号
// return           : 取得した行列
cv::Mat* LinearizerOfProjector::getColorMixMat(const int index){
    // error processing
    if (m_aColorMixingMatrix.size() <= index || index < 0) {
        std::cerr << "size is strange number" << std::endl;
        ERROR_PRINT(index);
        return false;
    }
    
    // return
    return &(m_aColorMixingMatrix.at(index));
}

const std::vector<cv::Mat>* LinearizerOfProjector::getColorMixMat(void){
    return &m_aColorMixingMatrix;
}

///////////////////////////////  init method ///////////////////////////////
// init method
bool LinearizerOfProjector::initColorMixingMatrix(const int _mixMatLength){
    cv::Mat initMat = cv::Mat::zeros(3, 3, CV_64FC1);
    for (int i = 0; i < _mixMatLength; ++ i) {
        m_aColorMixingMatrix.push_back(initMat);
    }
//    vector<cv::Mat>::iterator itr;
//    for (itr = m_aColorMixingMatrix.begin(); itr != m_aColorMixingMatrix.end(); ++ itr) {
//        _print(*itr);
//    }
    return true;
}

///////////////////////////////  other method ///////////////////////////////
// プロジェクタの線形化を行うメソッド
// input / responseOfProjector  : 線形化のルックアップテーブルを入れる配列
// return   : 成功したかどうか
bool LinearizerOfProjector::linearlize(cv::Mat_<cv::Vec3b>* const _responseOfProjector){
    // 色変換行列の生成
    cout << "creating Color Mixing Matrix..." << endl;
    if( !calcColorMixingMatrix() ) return false;
    cout << "created Color Mixing Matrix" << endl;
    
    cout << "push any key" << endl;
    waitKey(-1);

    // プロジェクタの応答特性を計算
    cout << "creating response function..." << endl;
    if ( !calcResponseFunction(_responseOfProjector)) return false;
    cout << "created response function" << endl;
    
    cout << "linealize is finish" << endl;
    return true;
}

// 色変換行列の算出
bool LinearizerOfProjector::calcColorMixingMatrix(void){
    // init
    ProCam *procam = getProCam();
//    _print_name(*procam);
    Size *cameraSize = procam->getCameraSize(), *projectorSize = procam->getProjectorSize();
    
    // init capture image
    uchar depth8x3 = CV_8UC3;
    cv::Mat black_cap   (*cameraSize, depth8x3, cv::Scalar(0, 0, 0));
    cv::Mat red_cap     (*cameraSize, depth8x3, cv::Scalar(0, 0, 255));
    cv::Mat green_cap   (*cameraSize, depth8x3, cv::Scalar(0, 255, 0));
    cv::Mat blue_cap    (*cameraSize, depth8x3, cv::Scalar(255, 0, 0));
    {
        // init projection image
        cv::Mat black_img   (*projectorSize, depth8x3, cv::Scalar(0, 0, 0));
        cv::Mat red_img     (*projectorSize, depth8x3, cv::Scalar(0, 0, 255));
        cv::Mat green_img   (*projectorSize, depth8x3, cv::Scalar(0, 255, 0));
        cv::Mat blue_img    (*projectorSize, depth8x3, cv::Scalar(255, 0, 0));

        // projection and capture image
        procam->captureFromLight(&black_cap, black_img);
        procam->captureFromLight(&red_cap, red_img);
        procam->captureFromLight(&green_cap, green_img);
        procam->captureFromLight(&blue_cap, blue_img);
    }
    
    // show image
    imshow("black_cap", black_cap);
    imshow("red_cap", red_cap);
    imshow("green_cap", green_cap);
    imshow("blue_cap", blue_cap);
    
    // translate bit depth (uchar[0-255] -> double[0-1])
    uchar depth64x3 = CV_64FC3;
    black_cap.convertTo(black_cap, depth64x3, 1.0/255.0);
    red_cap.convertTo(red_cap, depth64x3, 1.0/255.0);
    green_cap.convertTo(green_cap, depth64x3, 1.0/255.0);
    blue_cap.convertTo(blue_cap, depth64x3, 1.0/255.0);
    
    // calc difference[-1-1]
    Mat diffRedAndBlack = red_cap - black_cap;
    Mat diffGreenAndBlack = green_cap - black_cap;
    Mat diffBlueAndBlack = blue_cap - black_cap;
    black_cap.release();
    red_cap.release();
    green_cap.release();
    blue_cap.release();
    
    // show difference image
    imshow("diffRedAndBlack", diffRedAndBlack);
    imshow("diffGreenAndBlack", diffGreenAndBlack);
    imshow("diffBlueAndBlack", diffBlueAndBlack);
    
    // image divided by any color element
    Point point(0, 0);
    const Vec3d* redV = getPixelNumd(diffRedAndBlack, point);
    _print_vector(*redV);
    normalizeByAnyColorChannel(&diffRedAndBlack, CV_RED);
    normalizeByAnyColorChannel(&diffGreenAndBlack, CV_GREEN);
    normalizeByAnyColorChannel(&diffBlueAndBlack, CV_BLUE);
    imshow("diffRedAndBlack2", diffRedAndBlack);
    imshow("diffGreenAndBlack2", diffGreenAndBlack);
    imshow("diffBlueAndBlack2", diffBlueAndBlack);
//    const Vec3d* redV2 = getPixelNumd(diffRedAndBlack, point);
//    _print_vector(*redV2);
    
    // create V map
    createVMap(diffRedAndBlack, diffGreenAndBlack, diffBlueAndBlack);
        
    return true;
}

// Vマップを画像を用いて生成
// input / _diffR2BL    : 計算に用いる赤ー黒の差分
// input / _diffG2BL    : 計算に用いる緑ー黒の差分
// input / _diffB2BL    : 計算に用いる青ー黒の差分
// return               : 成功したかどうか
bool LinearizerOfProjector::createVMap(const cv::Mat& _normalR2BL, const cv::Mat& _normalG2BL, const cv::Mat& _normalB2BL){
    // init
    std::vector<cv::Mat> l_VMap;
    Mat V = Mat::zeros(3, 3, CV_64FC1);
    const int ch = _normalR2BL.channels();
    int rows = _normalR2BL.rows, cols = _normalR2BL.cols;
    if (isContinuous(_normalR2BL, _normalG2BL, _normalB2BL)) {
        cols *= rows;
        rows = 1;
    }
    
    // scan all pixel of camera
    for (int y = 0; y < rows; ++ y) {
        const double* pNormalR = _normalR2BL.ptr<double>(y);
        const double* pNormalG = _normalG2BL.ptr<double>(y);
        const double* pNormalB = _normalB2BL.ptr<double>(y);

        for (int x = 0; x < cols; ++ x) {
            // assigne V
            V.at<double>(0, 0) = pNormalR[x * ch + CV_RED];
            V.at<double>(0, 1) = pNormalR[x * ch + CV_GREEN];
            V.at<double>(0, 2) = pNormalR[x * ch + CV_BLUE];
            
            V.at<double>(1, 0) = pNormalG[x * ch + CV_RED];
            V.at<double>(1, 1) = pNormalG[x * ch + CV_GREEN];
            V.at<double>(1, 2) = pNormalG[x * ch + CV_BLUE];
            
            V.at<double>(2, 0) = pNormalB[x * ch + CV_RED];
            V.at<double>(2, 1) = pNormalB[x * ch + CV_GREEN];
            V.at<double>(2, 2) = pNormalB[x * ch + CV_BLUE];
//            _print(V);
            
            // push V
            l_VMap.push_back(V);
        }
    }
    
    // setting
    _print(l_VMap[0]);
    setColorMixMat(&l_VMap);
    return true;
}

// 応答特性を計算する
// output / _responseOfProjector    : 計算した応答特性を入れる変数
// return                           : 成功したかどうか
bool LinearizerOfProjector::calcResponseFunction(cv::Mat_<cv::Vec3b>* const _responseMap){
    // init
//    const std::vector<cv::Mat>* l_VMap = getColorMixMat();
    ProCam* l_procam = getProCam();     // ProCamへのポインタ
    const Size* const l_cameraSize = l_procam->getCameraSize();         // カメラサイズ
    const Size* const l_projectorSize = l_procam->getProjectorSize();   // プロジェクタサイズ
    Mat camColor = Mat::zeros(3, 1, CV_64FC1);
    Mat camImage = Mat::zeros(*l_cameraSize, CV_8UC3), prjImage = Mat::zeros(*l_projectorSize, CV_8UC3);
    Mat_<cv::Vec3b> l_responseImage(*l_cameraSize);
    const Size l_responseMapSize(l_cameraSize->width * 256, l_cameraSize->height);
    Mat_<cv::Vec3b> l_responseMap(l_responseMapSize);

    // projection RGB * luminance
    // scanning all luminance[0-255] of projector
    for (int prjLuminance = 100; prjLuminance < 256; prjLuminance+=10) {
        // create flat color image
        cv::Vec3b prjColor(prjLuminance, prjLuminance, prjLuminance);
        prjImage = cv::Scalar(prjColor);
        _print2(prjLuminance, prjColor);

        // capture from projection image
        l_procam->captureFromLight(&camImage, prjImage);

        // calc response function
        getResponseOfAllPixel(&l_responseImage, prjColor, camImage);
        imshow("l_responseImage", l_responseImage);
        
        // set response map
        setResponseMap(&l_responseMap, l_responseImage, prjLuminance, 256);
    }
    
    // 代入
    *_responseMap = l_responseMap.clone();
    return true;
}

// 入力と出力の関係から応答特性を取得
// output / _response   : 計算した応答特性
// input / _I           : 投影色(非線形)
// input / _C           : 撮影色(線形化済み)
// input / _V           : 色変換行列
// return               : 成功したかどうか
bool LinearizerOfProjector::getResponse(cv::Vec3b* const _response, const cv::Vec3b& _I, const cv::Vec3b& _C, const cv::Mat& _V){
    // error processing
    const int MAT_TYEP_DOUBLE = CV_64FC1;
    Mat m3x3 = Mat::zeros(3, 3, MAT_TYEP_DOUBLE);
    if ( !isEqualSizeAndType(m3x3, _V) ) {
        ERROR_PRINT2("_V is not 3x3 and CV_64FC1 Matrix", _V);
        return false;
    }
    m3x3.release();
    
    // init and translate
    Mat invV = _V.inv();    // V^{-1}
    Mat l_CMat(_C);         // translate Vec3b to Mat
    l_CMat.convertTo(l_CMat, MAT_TYEP_DOUBLE);
    Mat l_responseMat(*_response);  // translate Vec3b to Mat
    Mat l_IMat(_I);                 // translate Vec3b to Mat
    l_responseMat.convertTo(l_responseMat, MAT_TYEP_DOUBLE);
    l_IMat.convertTo(l_IMat, MAT_TYEP_DOUBLE);

    // compute response function
    Mat P = invV * l_CMat;      // V^{-1} * C
    _print4(P, _C, _I, _V);
    
    // return _response
    Vec3b l_vecP(P);
    *_response = l_vecP;
    return true;
}

// 全画素の応答特性を取得する
// output / _response   : 取得した応答特性
// input / _I           : 出力色
// input / _CImage      : 取得画像
// return               : 成功したかどうか
bool LinearizerOfProjector::getResponseOfAllPixel(cv::Mat_<cv::Vec3b>* const _response, const cv::Vec3b& _I, const cv::Mat_<cv::Vec3b>& _CImage){
    // init
    int rows = _CImage.rows, cols = _CImage.cols;
    if (_CImage.isContinuous()) {
        cols *= rows;
        rows = 1;
    }
    const std::vector<cv::Mat>* l_aMixMat = getColorMixMat();
    std::vector<cv::Mat>::const_iterator l_itrMixMat = l_aMixMat->begin();
    
    // scanning all pixel
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3b* p_response = _response->ptr<Vec3b>(y);
        const cv::Vec3b* p_CImage = _CImage.ptr<cv::Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x, ++ l_itrMixMat) {
            getResponse(&p_response[x], _I, p_CImage[x], *l_itrMixMat);
//            _print(p_response[x]);
        }
    }
    
    return true;
}

