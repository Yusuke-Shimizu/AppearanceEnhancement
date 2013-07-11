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

#include <fstream>  // ofstream
#include "LinearizerOfProjector.h"
#include "myOpenCV.h"
#include "ProCam.h"
#include "common.h"

using namespace std;
using namespace cv;

///////////////////////////////  constructor ///////////////////////////////
LinearizerOfProjector::LinearizerOfProjector(ProCam* procam){
//    cout << "setting LinearizerOfProjector..." << endl;
    setProCam(procam);
    const cv::Size* cameraSize = m_procam->getCameraSize();
    initColorMixingMatrixMap(*cameraSize);
//    cout << "finish LinearizerOfProjector" << endl;
}

///////////////////////////////  destructor ///////////////////////////////
LinearizerOfProjector::~LinearizerOfProjector(void){
    cout << "LinearizerOfProjector is deleted (" << this << ")" << endl;
}

///////////////////////////////  set method ///////////////////////////////
// プロカムのセッティング
bool LinearizerOfProjector::setProCam(ProCam* procam){
    m_procam = procam;
//    _print_name(*procam);
    return true;
}

// m_ColorMixingMatrixの全設定
// input / _aMat    : 設定する配列
// return           : 成功したかどうか
//bool LinearizerOfProjector::setColorMixMat(const std::vector<cv::Mat>* _aMat){
//    // error processing
//    const unsigned long size = m_aColorMixingMatrix.size();
//    if (_aMat->size() != size) {
//        std::cerr << "vector size is different" << std::endl;
//        ERROR_PRINT(size);
//        return false;
//    }
//    
//    // assign
////    std::vector<cv::Mat>::iterator itrMem = m_aColorMixingMatrix.begin();
////    std::vector<cv::Mat>::const_iterator itrArg = _aMat->begin();
////    for (; itrMem != m_aColorMixingMatrix.end(); ++ itrMem, ++ itrArg) {
////        *itrMem = *itrArg;
////    }
//    std::copy(_aMat->begin(), _aMat->end(), m_aColorMixingMatrix.begin());
//    return true;
//}

// m_ColorMixingMatrixの全設定
// input / _aMat    : 設定する配列
// return           : 成功したかどうか
bool LinearizerOfProjector::setColorMixMatMap(const cv::Mat_<Vec9d>& _aMat){
    // error processing
    if ( !isEqualSizeAndType(_aMat, m_colorMixingMatrixMap) ) {
        ERROR_PRINT("size or type is different");
        _print_name(_aMat);
        printMatPropaty(_aMat);
        return false;
    }
    
    // deep copy
    m_colorMixingMatrixMap = _aMat.clone();
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
//
bool LinearizerOfProjector::getProCam(ProCam* const procam){
    *procam = *m_procam;
    return true;
}
ProCam* LinearizerOfProjector::getProCam(void){
//    _print_name(*m_procam);
    return m_procam;
}

// m_colorMixingMatrixMapポインタの取得
const cv::Mat_<Vec9d>* LinearizerOfProjector::getColorMixMatMap(void){
    return &m_colorMixingMatrixMap;
}

///////////////////////////////  init method ///////////////////////////////
// cmmMapの初期化
bool LinearizerOfProjector::initColorMixingMatrixMap(const cv::Size& _cameraSize){
    // init
    const Vec9d l_initV(0, 0, 0, 0, 0, 0, 0, 0, 0);
    
    // initialize
    m_colorMixingMatrixMap = Mat_<Vec9d>(_cameraSize);
    const int rows = _cameraSize.height, cols = _cameraSize.width;
    for (int y = 0; y < rows; ++ y) {
        Vec9d* p_cmmm = m_colorMixingMatrixMap.ptr<Vec9d>(y);
        for (int x = 0; x < cols; ++ x) {
            p_cmmm[x] = l_initV;
//            _print(p_cmmm[x]);
        }
    }
    return true;
}
///////////////////////////////  save method ///////////////////////////////
bool LinearizerOfProjector::saveColorMixingMatrix(const char* fileName){
    // init
    ofstream ofs;
    ofs.open(fileName, ios_base::out | ios_base::trunc);
    if (!ofs) {
        ERROR_PRINT2(fileName, "is Not Found");
        return false;
    }
//    const vector<Mat>* l_aColorMixingMatrix = getColorMixMat();
    const Mat_<Vec9d>* l_cmmMap = getColorMixMatMap();
    
    //
    const int rows = l_cmmMap->rows, cols = l_cmmMap->cols;
    for (int y = 0; y < rows; ++ y) {
        const Vec9d* p_cmmMap = l_cmmMap->ptr<Vec9d>(y);
        for (int x = 0; x < cols; ++ x) {
            ofs << p_cmmMap[x] << "|";
        }
        ofs << endl;
    }

    return true;
}

// byte出力
bool LinearizerOfProjector::saveColorMixingMatrixOfByte(const char* fileName){
    // init
    ofstream ofs;
    ofs.open(fileName, ios_base::out | ios_base::trunc | ios_base::binary);
    if (!ofs) {
        ERROR_PRINT2(fileName, "is Not Found");
        return false;
    }
    const Mat_<Vec9d>* l_cmmMap = getColorMixMatMap();
    
    //
    const int rows = l_cmmMap->rows, cols = l_cmmMap->cols;
    ofs.write((char*)&rows, sizeof(int));
    ofs.write((char*)&cols, sizeof(int));
    for (int y = 0; y < rows; ++ y) {
        const Vec9d* p_cmmMap = l_cmmMap->ptr<Vec9d>(y);
        for (int x = 0; x < cols; ++ x) {
            for (int i = 0; i < 9; ++ i) {
                ofs.write((char*)&p_cmmMap[x][i], sizeof(double));
            }
        }
    }

    return true;
}
///////////////////////////////  load method ///////////////////////////////
bool LinearizerOfProjector::loadColorMixingMatrix(const char* fileName){
    return true;
}

//
bool LinearizerOfProjector::loadColorMixingMatrixOfByte(const char* fileName){
    // init
    ifstream ifs(fileName);
    if (!ifs) {
        ERROR_PRINT2(fileName, "is Not Found");
        exit(-1);
    }
    
    // get size
    Size mapSize(0, 0);
    ifs.read((char*)&mapSize.height, sizeof(int));
    ifs.read((char*)&mapSize.width, sizeof(int));
    Mat_<Vec9d> l_cmmMap(mapSize);
    
    // read
    const int rows = l_cmmMap.rows, cols = l_cmmMap.cols;
    for (int y = 0; y < rows; ++ y) {
        const Vec9d* p_cmmMap = l_cmmMap.ptr<Vec9d>(y);
        for (int x = 0; x < cols; ++ x) {
            for (int i = 0; i < 9; ++ i) {
                ifs.read((char*)&p_cmmMap[x][i], sizeof(double));
            }
        }
    }
    
    // setting
    setColorMixMatMap(l_cmmMap);
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
    
    // show V map
    showVMap();
    
    // save
    cout << "saving Color Mixing Matrix..." << endl;
//    if ( !saveColorMixingMatrix(CMM_MAP_FILE_NAME) ) {
//        ERROR_PRINT("save dame ppo-i");
//        exit(-1);
//    }
    if ( !saveColorMixingMatrixOfByte(CMM_MAP_FILE_NAME_BYTE) ) {
        ERROR_PRINT("save dame ppo-i");
        exit(-1);
    }
    cout << "saved Color Mixing Matrix" << endl;

    // load
//    cout << "loading Color Mixing Matrix..." << endl;
//    if ( !loadColorMixingMatrixOfByte(CMM_MAP_FILE_NAME_BYTE) ) {
//        ERROR_PRINT("save dame ppo-i");
//        exit(-1);
//    }
//    cout << "loaded Color Mixing Matrix" << endl;

    // キー待機
    MY_WAIT_KEY();

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
    cv::Mat white_cap    (*cameraSize, depth8x3, cv::Scalar(255, 255, 255));
    {
        // init projection image
        cv::Mat black_img   (*projectorSize, depth8x3, cv::Scalar(0, 0, 0));
        cv::Mat red_img     (*projectorSize, depth8x3, cv::Scalar(0, 0, 255));
        cv::Mat green_img   (*projectorSize, depth8x3, cv::Scalar(0, 255, 0));
        cv::Mat blue_img    (*projectorSize, depth8x3, cv::Scalar(255, 0, 0));
        cv::Mat white_img    (*projectorSize, depth8x3, cv::Scalar(255, 255, 255));

        // projection and capture image
        procam->captureFromLight(&black_cap, black_img);
        procam->captureFromLight(&red_cap, red_img);
        procam->captureFromLight(&green_cap, green_img);
        procam->captureFromLight(&blue_cap, blue_img);
        procam->captureFromLight(&white_cap, white_img);
    }
    
    // show image
    imshow("black_cap", black_cap);
    imshow("red_cap", red_cap);
    imshow("green_cap", green_cap);
    imshow("blue_cap", blue_cap);
    imshow("white_cap", white_cap);
    
    // translate bit depth (uchar[0-255] -> double[0-1])
    uchar depth64x3 = CV_64FC3;
    double rate = 1.0 / 255.0;
    black_cap.convertTo(black_cap, depth64x3, rate);
    red_cap.convertTo(red_cap, depth64x3, rate);
    green_cap.convertTo(green_cap, depth64x3, rate);
    blue_cap.convertTo(blue_cap, depth64x3, rate);
    white_cap.convertTo(white_cap, depth64x3, rate);

    // calc difference[-1-1]
    Mat diffRedAndBlack = red_cap - black_cap;
    Mat diffGreenAndBlack = green_cap - black_cap;
    Mat diffBlueAndBlack = blue_cap - black_cap;
    Mat diffWhiteAndBlack = white_cap - black_cap;
//    _print3(black_cap.at<Vec3d>(100, 100), white_cap.at<Vec3d>(100, 100), diffWhiteAndBlack.at<Vec3d>(100, 100));
//    imshow("black_cap2", black_cap);
//    imshow("white_cap2", white_cap);
//    imshow("diffWtoB_cap", diffWhiteAndBlack);
    black_cap.release();
    red_cap.release();
    green_cap.release();
    blue_cap.release();
    white_cap.release();

    // show difference image
//    imshow("diffRedAndBlack", diffRedAndBlack);
//    imshow("diffGreenAndBlack", diffGreenAndBlack);
//    imshow("diffBlueAndBlack", diffBlueAndBlack);
    
    // image divided by any color element
    Point point(0, 0);
    const Vec3d* redV = getPixelNumd(diffRedAndBlack, point);
    _print_vector(*redV);
    normalizeByAnyColorChannel(&diffRedAndBlack, CV_RED);
    normalizeByAnyColorChannel(&diffGreenAndBlack, CV_GREEN);
    normalizeByAnyColorChannel(&diffBlueAndBlack, CV_BLUE);
//    imshow("diffRedAndBlack2", diffRedAndBlack);
//    imshow("diffGreenAndBlack2", diffGreenAndBlack);
//    imshow("diffBlueAndBlack2", diffBlueAndBlack);
    const Vec3d* redV2 = getPixelNumd(diffRedAndBlack, point);
    _print_vector(*redV2);
    
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
    const int ch = _normalR2BL.channels();
    int rows = _normalR2BL.rows, cols = _normalR2BL.cols;
    Mat_<Vec9d> l_cmmm = Mat::zeros(rows, cols, CV_64FC(9));
    if (isContinuous(_normalR2BL, _normalG2BL, _normalB2BL, l_cmmm)) {
        cols *= rows;
        rows = 1;
    }
    
    // scan all pixel of camera
    Mat V = Mat::zeros(3, 3, CV_64FC1);
    Vec9d l_VVec(0, 0, 0, 0, 0, 0, 0, 0, 0);
    for (int y = 0; y < rows; ++ y) {
        const double* pNormalR = _normalR2BL.ptr<double>(y);
        const double* pNormalG = _normalG2BL.ptr<double>(y);
        const double* pNormalB = _normalB2BL.ptr<double>(y);
//        const Vec3d* l_pNormalR = _normalR2BL.ptr<Vec3d>(y);
//        const Vec3d* l_pNormalG = _normalG2BL.ptr<Vec3d>(y);
//        const Vec3d* l_pNormalB = _normalB2BL.ptr<Vec3d>(y);
        Vec9d* p_cmmm = l_cmmm.ptr<Vec9d>(y);

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
            
            // push V
            convertMatToVec(&l_VVec, V);    // mat -> vec
            p_cmmm[x] = l_VVec;
        }
    }
    
    // setting
    setColorMixMatMap(l_cmmm);
    return true;
}

// 応答特性を計算する
// output / _responseOfProjector    : 計算した応答特性を入れる変数
// return                           : 成功したかどうか
bool LinearizerOfProjector::calcResponseFunction(cv::Mat_<cv::Vec3b>* const _responseMap){
    // init
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
//        _print2(prjLuminance, prjColor);

        // capture from projection image
        l_procam->captureFromLight(&camImage, prjImage);

        // calc response function
        getResponseOfAllPixel(&l_responseImage, camImage);
        imshow("l_responseImage", l_responseImage);
        
        // set response map
        setResponseMap(&l_responseMap, l_responseImage, prjLuminance, 256);
        Vec3b* l_pResMap = l_responseMap.ptr<Vec3b>(0);
        _print2(prjLuminance, l_pResMap[prjLuminance]);
        _print2(prjLuminance, l_responseMap.at<Vec3b>(0, prjLuminance));
    }
    
    // 代入
    *_responseMap = l_responseMap.clone();
    return true;
}

// 全画素の応答特性を取得する
// output / _response   : 取得した応答特性
// input / _I           : 出力色
// input / _CImage      : 取得画像
// return               : 成功したかどうか
bool LinearizerOfProjector::getResponseOfAllPixel(cv::Mat_<cv::Vec3b>* const _response, const cv::Mat_<cv::Vec3b>& _CImage){
    // init
    int rows = _CImage.rows, cols = _CImage.cols;
    if (_CImage.isContinuous()) {
        cols *= rows;
        rows = 1;
    }
//    const std::vector<cv::Mat>* l_aMixMat = getColorMixMat();
//    std::vector<cv::Mat>::const_iterator l_itrMixMat = l_aMixMat->begin();
    const Mat_<Vec9d>* l_cmmMap = getColorMixMatMap();
    Mat_<double> l_V = Mat::zeros(3, 3, CV_64FC1);
    Vec9d l_VVec(0,0,0,0,0,0,0,0,0);
    
    // scanning all pixel
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3b* p_response = _response->ptr<Vec3b>(y);
        const cv::Vec3b* p_CImage = _CImage.ptr<cv::Vec3b>(y);
        const Vec9d* p_cmmMap = l_cmmMap->ptr<Vec9d>(y);
        
        for (int x = 0; x < cols; ++ x/*, ++ l_itrMixMat*/) {
            convertVecToMat(&l_V, p_cmmMap[x]);
//            getResponse(&p_response[x], p_CImage[x], *l_itrMixMat);
            getResponse(&p_response[x], p_CImage[x], l_V);
//            _print(p_response[x]);
        }
    }
    
    return true;
}

// 入力と出力の関係から応答特性を取得
// output / _response   : 計算した応答特性
// input / _I           : 投影色(非線形)
// input / _C           : 撮影色(線形化済み)
// input / _V           : 色変換行列
// return               : 成功したかどうか
bool LinearizerOfProjector::getResponse(cv::Vec3b* const _response, const cv::Vec3b& _C, const cv::Mat& _V){
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
    Vec3b l_rgbC(0, 0, 0);
    convertBGRtoRGB(&l_rgbC, _C);   // bgr -> rgb
    Mat l_CMat(l_rgbC);         // Vec3b -> Mat
    l_CMat.convertTo(l_CMat, MAT_TYEP_DOUBLE);  // uchar -> double

    // compute response function
    Mat P = invV * l_CMat;      // V^{-1} * C
//    _print4(P, _C, _I, _V);
    
    // return _response
    Vec3b l_vecP(P);    // Mat -> Vec3b
    *_response = l_vecP;
    return true;
}

// V mapの視覚化
bool LinearizerOfProjector::showVMap(void){
    // init
    const Mat_<Vec9d>* l_cmmm = getColorMixMatMap();
    int rows = l_cmmm->rows, cols = l_cmmm->cols;
    Mat_<Vec3d> l_VRed(rows, cols), l_VGreen(rows, cols), l_VBlue(rows, cols);
    if (isContinuous(*l_cmmm, l_VRed, l_VGreen, l_VBlue)) {
        cols *= rows;
        rows = 1;
    }
    
    // create V images
    for (int y = 0; y < rows; ++ y) {
        const Vec9d* p_cmmm = l_cmmm->ptr<Vec9d>(y);
        Vec3d* p_VRed = l_VRed.ptr<Vec3d>(y);
        Vec3d* p_VGreen = l_VGreen.ptr<Vec3d>(y);
        Vec3d* p_VBlue = l_VBlue.ptr<Vec3d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            p_VRed[x] = Vec3d(p_cmmm[x][0], p_cmmm[x][1], p_cmmm[x][2]);
            p_VBlue[x] = Vec3d(p_cmmm[x][3], p_cmmm[x][4], p_cmmm[x][5]);
            p_VGreen[x] = Vec3d(p_cmmm[x][6], p_cmmm[x][7], p_cmmm[x][8]);
        }
    }
    
    // show
    MY_IMSHOW(l_VRed);
    MY_IMSHOW(l_VGreen);
    MY_IMSHOW(l_VBlue);
    return true;
}
