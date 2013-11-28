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
    setProCam(procam);
    const cv::Size* cameraSize = procam->getCameraSize();
    initColorMixingMatrixMap(*cameraSize);
    initAllCImages();
}

///////////////////////////////  destructor ///////////////////////////////
LinearizerOfProjector::~LinearizerOfProjector(void){
    cout << "LinearizerOfProjector is deleted (" << this << ")" << endl;
}

///////////////////////////////  set method ///////////////////////////////
// プロカムのセッティング
bool LinearizerOfProjector::setProCam(ProCam* procam){
    m_procam = procam;
    return true;
}

// m_ColorMixingMatrixの全設定
// input / _aMat    : 設定する配列
// return           : 成功したかどうか
bool LinearizerOfProjector::setColorMixMatMap(const cv::Mat_<Vec9d>& _aMat){
    // error processing
    const Mat_<Vec9d>* l_cmmm = getColorMixMatMap();
    if ( !isEqualSizeAndType(_aMat, *l_cmmm) ) {
        ERROR_PRINT("size or type is different");
        _print_name(*l_cmmm);
        printMatPropaty(*l_cmmm);
        _print_name(_aMat);
        printMatPropaty(_aMat);
        return false;
    }
    
    // deep copy
    m_colorMixingMatrixMap = _aMat.clone();
    return true;
}


// 応答特性マップを設定
// output / _responseMap    : 応答特性マップ
// return                   : 成功したかどうか
bool LinearizerOfProjector::setResponseMap(cv::Mat_<cv::Vec3b>* const _responseMapP2I, cv::Mat_<cv::Vec3b>* const _responseMapI2P, const cv::Mat_<cv::Vec3b>& _CImage, const uchar _INum){
    // error
    ProCam* l_procam = getProCam();
    const Size* l_prjSize = l_procam->getProjectorSize();
    const Size* l_camSize = l_procam->getCameraSize();
    const Size l_responseMapP2ICompSize(_responseMapP2I->cols / 256, _responseMapP2I->rows);
    const Size l_CImageSize(_CImage.cols, _CImage.rows);
    if (*l_prjSize != l_responseMapP2ICompSize || *l_camSize != l_CImageSize) {
        cerr << "size is different" << endl;
        ERROR_PRINT4(*l_prjSize, l_responseMapP2ICompSize, *l_camSize, l_CImageSize);
        exit(-1);
    } else if (!isEqualSizeAndType(*_responseMapP2I, *_responseMapI2P)) {
        cerr << "different size of type" << endl;
        _print_mat_propaty(*_responseMapP2I);
        _print_mat_propaty(*_responseMapI2P);
        exit(-1);
    }
    
    // calc P on Camera Space
    Mat l_PImageOnCameraSpace(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);    // カメラ座標系におけるP（=V^{-1}C）
    convertCameraImageToProjectorOne(&l_PImageOnCameraSpace, _CImage);
    MY_IMSHOW(l_PImageOnCameraSpace);
    
    // convert P on Camera Space to Projector Space
    Mat l_PImageOnProjectorSpace(*l_prjSize, CV_8UC3, CV_SCALAR_BLACK);    // カメラ座標系におけるP（=V^{-1}C）
    l_procam->convertProjectorDomainToCameraOne(&l_PImageOnProjectorSpace, l_PImageOnCameraSpace);
//    MY_IMSHOW(l_PImageOnProjectorSpace);
    
    // create check mat
    const Mat l_whiteCameraImage(l_camSize->height, l_camSize->width, CV_8UC3, CV_SCALAR_WHITE);
    Mat l_whiteImageOnProjectorDomain(l_prjSize->height, l_prjSize->width, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->convertProjectorDomainToCameraOne(&l_whiteImageOnProjectorDomain, l_whiteCameraImage);
    Vec3b l_nonProjectionColor(0, 0, 0);
    
    // setting
    int PRows = l_prjSize->height, PCols = l_prjSize->width, PCh = _responseMapP2I->channels();
    for (int y = 0; y < PRows; ++ y) {
        const Vec3b* l_pPImageOnPS = l_PImageOnProjectorSpace.ptr<Vec3b>(y);
        Vec3b* l_pResponseMapP2I = _responseMapP2I->ptr<Vec3b>(y);
        Vec3b* l_pResponseMapI2P = _responseMapI2P->ptr<Vec3b>(y);
        Vec3b* l_checkMap = l_whiteImageOnProjectorDomain.ptr<Vec3b>(y);
        
        for (int x = 0; x < PCols; ++ x) {
            // 投影しない箇所の設定を飛ばす
            if (l_checkMap[x] == l_nonProjectionColor) {
                l_pResponseMapP2I[x + 255] = CV_VEC3B_BLACK;
                l_pResponseMapI2P[x + 255] = CV_VEC3B_BLACK;
                continue;
            }
            
            // 投影する場所のみの設定
            for (int ch = 0; ch < PCh; ++ ch) {
                // setting P2I
                const int responseIndex = x * 256 + l_pPImageOnPS[x][ch];   // l_pResponseMapP2Iのインデックス
                if (l_pPImageOnPS[x][ch] == 255 || l_pResponseMapP2I[responseIndex][ch] == 0) { // 文字が被り，別の値で変更をしないようにする
                    l_pResponseMapP2I[responseIndex][ch] = _INum;
                }

                // setting I2P
                const int responseIndexI2P = x * 256 + _INum;   // l_pResponseMapI2Pのインデックス
                l_pResponseMapI2P[responseIndexI2P][ch] = l_pPImageOnPS[x][ch];
            }
        }
    }
    
    return true;
}

// m_allCImagesの設定
bool LinearizerOfProjector::setAllCImages(const cv::Mat& _allCImages){
    // error handle
    const Mat* l_allCImages_true = getAllCImages();
    if (!isEqualSizeAndType(_allCImages, *l_allCImages_true)) {
        cerr << "setting mat size is different" << endl;
        _print_mat_propaty(_allCImages);
        _print_mat_propaty(*l_allCImages_true);
        exit(-1);
    }
    
    // set
    m_allCImages = _allCImages.clone();
    
    return true;
}

// m_allCImagesにある輝度の画像を設定
bool LinearizerOfProjector::setCImages(const cv::Mat& _CImage, const int _luminance){
    // set
    return insertMatForDeepDepthMat(&m_allCImages, _CImage, _luminance);
}

///////////////////////////////  get method ///////////////////////////////
//
ProCam* LinearizerOfProjector::getProCam(void){
    return m_procam;
}

// m_colorMixingMatrixMapポインタの取得
const cv::Mat_<Vec9d>* LinearizerOfProjector::getColorMixMatMap(void){
    return &m_colorMixingMatrixMap;
}

// m_allCImagesの取得
const cv::Mat* LinearizerOfProjector::getAllCImages(void){
    return &m_allCImages;
}

///////////////////////////////  init method ///////////////////////////////
// cmmMapの初期化
bool LinearizerOfProjector::initColorMixingMatrixMap(const cv::Size& _cameraSize){
    // init
    const Vec9d l_initV(0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0);
    
    // initialize
    m_colorMixingMatrixMap = Mat_<Vec9d>(_cameraSize);
    const int rows = _cameraSize.height, cols = _cameraSize.width;
    for (int y = 0; y < rows; ++ y) {
        Vec9d* p_cmmm = m_colorMixingMatrixMap.ptr<Vec9d>(y);
        for (int x = 0; x < cols; ++ x) {
            p_cmmm[x] = l_initV;
        }
    }
    return true;
}

// m_allCImagesの初期化
bool LinearizerOfProjector::initAllCImages(void){
    ProCam* l_procam = getProCam();
    const Size* camSize = l_procam->getCameraSize();
    const Size allImgSize(camSize->width * 256, camSize->height);
    
    m_allCImages = Mat(allImgSize, CV_8UC3, CV_SCALAR_BLACK);
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

bool LinearizerOfProjector::saveAllC(const char* fileName, const cv::Point& _pt){
    ProCam* l_procam = getProCam();
    const Mat* l_allCImages = getAllCImages();
    l_procam->savePrintProjectorResponse(fileName, _pt, *l_allCImages);
    return true;
}

bool LinearizerOfProjector::saveAllCImages(const char* _fileName){
    ofstream ofs;
    ofs.open(_fileName, ios_base::out | ios_base::trunc | ios_base::binary);
    if (!ofs) {
        ERROR_PRINT2(_fileName, "is Not Found");
        exit(-1);
    }

    const Mat* l_allCImages = getAllCImages();
    const int rows = l_allCImages->rows, cols = l_allCImages->cols, channels = l_allCImages->channels();
    
    ofs.write((char*)&rows, sizeof(int));
    ofs.write((char*)&cols, sizeof(int));

    for (int y = 0; y < rows; ++ y) {
        const Vec3b* l_pAllCImages = l_allCImages->ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int c = 0; c < channels; ++ c) {
                ofs.write((char*)&l_pAllCImages[x][c], sizeof(char));
            }
        }
    }
    
    return true;
}

///////////////////////////////  load method ///////////////////////////////
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

bool LinearizerOfProjector::loadAllCImages(const char* _fileName){
    // init
    ifstream ifs(_fileName);
    if (!ifs) {
        cerr << _fileName << "is Not Found" << endl;
        exit(-1);
    }
    
    // get size
    Size mapSize(0, 0);
    ifs.read((char*)&mapSize.height, sizeof(int));
    ifs.read((char*)&mapSize.width, sizeof(int));
    Mat l_allCImages(mapSize, CV_8UC3);
    
    // read
    const int rows = l_allCImages.rows, cols = l_allCImages.cols;
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pAllCImages = l_allCImages.ptr<Vec3b>(y);
        
        for (int x = 0; x < cols; ++ x) {
            for (int i = 0; i < 9; ++ i) {
                ifs.read((char*)&l_pAllCImages[x][i], sizeof(char));
            }
        }
    }
    
    // setting
    setAllCImages(l_allCImages);
    return true;
}

///////////////////////////////  show method ///////////////////////////////

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
            p_VRed[x] = Vec3d(p_cmmm[x][2], p_cmmm[x][1], p_cmmm[x][0]);
            p_VGreen[x] = Vec3d(p_cmmm[x][5], p_cmmm[x][4], p_cmmm[x][3]);
            p_VBlue[x] = Vec3d(p_cmmm[x][8], p_cmmm[x][7], p_cmmm[x][6]);
        }
    }
    
    // show
    MY_IMSHOW(l_VRed);
    MY_IMSHOW(l_VGreen);
    MY_IMSHOW(l_VBlue);
    return true;
}

//
bool LinearizerOfProjector::showAllCImages(void){
//    const Mat* l_allCImages = getAllCImages();
    
    return true;
}

///////////////////////////////  other method ///////////////////////////////
// プロジェクタの線形化を行うメソッド
// input / responseOfProjector  : 線形化のルックアップテーブルを入れる配列
// return   : 成功したかどうか
bool LinearizerOfProjector::doLinearlize(cv::Mat_<cv::Vec3b>* const _responseOfProjector, cv::Mat_<cv::Vec3b>* const _responseMapP2I){
    ///////////////// create V map /////////////////
    // 色変換行列の生成
    cout << "creating Color Mixing Matrix..." << endl;
    if( !calcColorMixingMatrix() ) return false;
    cout << "created Color Mixing Matrix" << endl;
    
    // show V map
    showVMap();

    ///////////////// create projector response function /////////////////
    // プロジェクタの応答特性を計算
    cout << "creating response function..." << endl;
    if ( !calcResponseFunction(_responseOfProjector, _responseMapP2I)) return false;
    cout << "created response function" << endl;
    
    cout << "linealize is finish" << endl;
    return true;
}

// 色変換行列の算出
bool LinearizerOfProjector::calcColorMixingMatrix(void){
    // init
    ProCam *procam = getProCam();
    Size *cameraSize = procam->getCameraSize();
    
    // init capture image
    uchar depth8x3 = CV_8UC3;
    cv::Mat black_cap   (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat red_cap     (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat green_cap   (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat blue_cap    (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat white_cap    (*cameraSize, depth8x3, CV_SCALAR_BLACK);

    // capture from some color light
    procam->captureFromLightOnProjectorDomain(&black_cap, CV_VEC3B_BLACK, true, SLEEP_TIME * 5);
    procam->captureFromLightOnProjectorDomain(&red_cap, CV_VEC3B_RED, true);
    procam->captureFromLightOnProjectorDomain(&green_cap, CV_VEC3B_GREEN, true);
    procam->captureFromLightOnProjectorDomain(&blue_cap, CV_VEC3B_BLUE, true);
    procam->captureFromLightOnProjectorDomain(&white_cap, CV_VEC3B_WHITE, true);

    // show image
    imshow("black_cap", black_cap);
    imshow("red_cap", red_cap);
    imshow("green_cap", green_cap);
    imshow("blue_cap", blue_cap);
    imshow("white_cap", white_cap);
    waitKey(30);
    
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
    
    // image divided by any color element
    normalizeByAnyColorChannel(&diffRedAndBlack, CV_RED);
    normalizeByAnyColorChannel(&diffGreenAndBlack, CV_GREEN);
    normalizeByAnyColorChannel(&diffBlueAndBlack, CV_BLUE);
    
    // create V map
    createVMap(diffRedAndBlack, diffGreenAndBlack, diffBlueAndBlack);
    
    // save
    cout << "saving Color Mixing Matrix..." << endl;
    if ( !saveColorMixingMatrixOfByte(CMM_MAP_FILE_NAME_BYTE) ) return false;
    cout << "saved Color Mixing Matrix" << endl;
    
    return true;
}

// より正確な色変換行列を取得する
bool LinearizerOfProjector::calcMoreDetailColorMixingMatrix(void){
    // init
    ProCam *procam = getProCam();
    Size *cameraSize = procam->getCameraSize();
    
    // init capture image
    uchar depth8x3 = CV_8UC3;
    cv::Mat black_cap   (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat red_cap     (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat green_cap   (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat blue_cap    (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    cv::Mat white_cap    (*cameraSize, depth8x3, CV_SCALAR_BLACK);
    
    // capture from some color light
    procam->captureFromLightOnProjectorDomain(&black_cap, CV_VEC3B_BLACK, true, SLEEP_TIME * 2);
    procam->captureFromLightOnProjectorDomain(&red_cap, CV_VEC3B_RED, true);
    procam->captureFromLightOnProjectorDomain(&green_cap, CV_VEC3B_GREEN, true);
    procam->captureFromLightOnProjectorDomain(&blue_cap, CV_VEC3B_BLUE, true);
    procam->captureFromLightOnProjectorDomain(&white_cap, CV_VEC3B_WHITE, true);
    
    // show image
    MY_IMSHOW(black_cap);
    MY_IMSHOW(red_cap);
    MY_IMSHOW(green_cap);
    MY_IMSHOW(blue_cap);
    MY_IMSHOW(white_cap);
    
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
    //    imshow("black_cap2", black_cap);
    //    imshow("white_cap2", white_cap);
    //    imshow("diffWtoB_cap", diffWhiteAndBlack);
    
    // show difference image
    //    imshow("diffRedAndBlack", diffRedAndBlack);
    //    imshow("diffGreenAndBlack", diffGreenAndBlack);
    //    imshow("diffBlueAndBlack", diffBlueAndBlack);
    
    // image divided by any color element
    normalizeByAnyColorChannel(&diffRedAndBlack, CV_RED);
    normalizeByAnyColorChannel(&diffGreenAndBlack, CV_GREEN);
    normalizeByAnyColorChannel(&diffBlueAndBlack, CV_BLUE);
    //    imshow("diffRedAndBlack2", diffRedAndBlack);
    //    imshow("diffGreenAndBlack2", diffGreenAndBlack);
    //    imshow("diffBlueAndBlack2", diffBlueAndBlack);
    
    // create V map
    createVMap(diffRedAndBlack, diffGreenAndBlack, diffBlueAndBlack);
    
    // save
    cout << "saving Color Mixing Matrix..." << endl;
    if ( !saveColorMixingMatrixOfByte(CMM_MAP_FILE_NAME_BYTE) ) return false;
    cout << "saved Color Mixing Matrix" << endl;
    
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
bool LinearizerOfProjector::calcResponseFunction(cv::Mat_<cv::Vec3b>* const _responseMap, cv::Mat_<cv::Vec3b>* const _responseMapP2I){
    // init
    ProCam* l_procam = getProCam();     // ProCamへのポインタ
    const Size* const l_cameraSize = l_procam->getCameraSize();         // カメラサイズ
    const Size* const l_projectorSize = l_procam->getProjectorSize();   // プロジェクタサイズ
    Mat camColor = Mat::zeros(3, 1, CV_64FC1);
    Mat camImage = Mat::zeros(*l_cameraSize, CV_8UC3), prjImage = Mat::zeros(*l_projectorSize, CV_8UC3);
    Mat_<cv::Vec3b> l_responseImage(*l_cameraSize);
    Mat_<Vec3b> l_responseMap = _responseMap->clone(); // _responseMapの一時的な置き場
    Mat_<Vec3b> l_responseMapP2I = _responseMap->clone();

    // projection RGB * luminance
    // scanning all luminance[0-255] of projector
    int prjLuminance = 0;
    _print(prjLuminance);
    l_procam->captureFromLightOnProjectorDomain(&camImage, prjLuminance, true, SLEEP_TIME * 2);
    for (prjLuminance = 0; prjLuminance < 256; prjLuminance += PROJECTION_LUMINANCE_STEP) {
        _print(prjLuminance);

        // capture from projection image
        l_procam->captureFromLightOnProjectorDomain(&camImage, prjLuminance, true);
        MY_IMSHOW(camImage);
        ostringstream oss;
        oss << PROJECTOR_RESPONSE_C_IMAGE_PATH << prjLuminance << ".png" << endl;
        imwrite(oss.str().c_str(), camImage);
        waitKey(1);
        
        // set inverce response function(P2I and I2P)
        setResponseMap(&l_responseMapP2I, &l_responseMap, camImage, prjLuminance);
        
        // set all C images
        setCImages(camImage, prjLuminance);
    }
    
    // interpolation projector response P to I
    l_procam->interpolationProjectorResponseP2I(&l_responseMapP2I);
    l_procam->interpolateProjectorResponseP2IAtOutOfCameraArea(&l_responseMapP2I);
    l_procam->medianBlurForProjectorResponseP2I(&l_responseMapP2I, l_responseMapP2I);
    
    // set and save
    // P2I
    l_procam->setProjectorResponseP2I(l_responseMapP2I);
    l_procam->saveProjectorResponseP2IForByte(PROJECTOR_RESPONSE_P2I_FILE_NAME_BYTE);
    // I2P
    l_procam->setProjectorResponse(l_responseMap);
    l_procam->saveProjectorResponseForByte(PROJECTOR_RESPONSE_I2P_FILE_NAME_BYTE);
    // all C images
    saveAllCImages();
    
    // deep copy
    *_responseMap = l_responseMap.clone();          // I2P
    *_responseMapP2I = l_responseMapP2I.clone();    // P2I
    
    //test
//    test_responseFunction();
    
    return true;
}

// 線形化出来てるかを線形化した投影光で０から２５５まで投影し，直線になるかを確認
void LinearizerOfProjector::test_responseFunction(void){
    //
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_captureImage(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    
    for (int i = 0; i < 256; ++ i) {
        l_procam->captureFromLinearLightOnProjectorDomain(&l_captureImage, i);
        Vec3b l_color = l_captureImage.at<Vec3b>(100, 100);
        _print2(i, l_color);
    }
}

// CからPを取得する
// output / _response   : 取得した応答特性
// input / _I           : 出力色
// input / _CImage      : 取得画像
// return               : 成功したかどうか
bool LinearizerOfProjector::convertCImage2PImage(cv::Mat_<cv::Vec3b>* const _PImageOnCameraDomain, const cv::Mat_<cv::Vec3b>& _CImage){
    // init
    int rows = _CImage.rows, cols = _CImage.cols;
    if (_CImage.isContinuous()) {
        cols *= rows;
        rows = 1;
    }

    const Mat_<Vec9d>* l_cmmMap = getColorMixMatMap();
    Mat_<double> l_V = Mat::zeros(3, 3, CV_64FC1);
    Vec9d l_VVec(0,0,0,0,0,0,0,0,0);
    
    // scanning all pixel
    for (int y = 0; y < rows; ++ y) {
        // init pointer
        Vec3b* p_response = _PImageOnCameraDomain->ptr<Vec3b>(y);
        const cv::Vec3b* p_CImage = _CImage.ptr<cv::Vec3b>(y);
        const Vec9d* p_cmmMap = l_cmmMap->ptr<Vec9d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            convertVecToMat(&l_V, p_cmmMap[x]);
            calcP(&p_response[x], p_CImage[x], l_V);
        }
    }
    
    return true;
}

// 入力と出力の関係から応答特性を取得
// output / _response   : 計算した応答特性
// input / _C           : 撮影色(線形化済み)
// input / _V           : 色変換行列
// return               : 成功したかどうか
bool LinearizerOfProjector::calcP(cv::Vec3b* const _response, const cv::Vec3b& _C, const cv::Mat& _V){
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
//    _print2(_C, l_rgbC);
    Mat l_CMat(l_rgbC);         // Vec3b -> Mat
    l_CMat.convertTo(l_CMat, MAT_TYEP_DOUBLE);  // uchar -> double

    // compute response function
    Mat P = invV * l_CMat;      // V^{-1} * C
//    _print4(P, _C, _I, _V);
    
    // inverce convert
    Vec3b l_vecP(P);    // Mat -> Vec3b
    Vec3b l_vecP_bgr(0, 0, 0);
    convertRGBtoBGR(&l_vecP_bgr, l_vecP);
    
    *_response = l_vecP_bgr;
    return true;
}
bool LinearizerOfProjector::calcPByVec(cv::Vec3b* const _response, const cv::Vec3b& _C, const Vec9d& _VVec){
    Mat_<double> l_V = Mat::zeros(3, 3, CV_64FC1);
    convertVecToMat(&l_V, _VVec);
    return calcP(_response, _C, l_V);
}

// 光学補正を行う
// input / _desiredImage    : このような見た目にして欲しい画像
bool LinearizerOfProjector::doRadiometricCompensation(const cv::Mat& _desiredImage, const int _waitTimeNum){
    // init
    ProCam* l_procam = getProCam();

    // P = V^{-1}C
    // 色空間をカメラ空間からプロジェクタ空間へ変換
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_projectionImageOnCameraSpace(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    convertCameraImageToProjectorOne(&l_projectionImageOnCameraSpace, _desiredImage);
    
    // project desired image
    Mat l_cameraImageFromDesiredImageProjection(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->captureFromLightOnProjectorDomain(&l_cameraImageFromDesiredImageProjection, _desiredImage, false, _waitTimeNum);

    // project compensated image
    Mat l_cameraImage(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->captureFromLinearLightOnProjectorDomain(&l_cameraImage, l_projectionImageOnCameraSpace, false, _waitTimeNum);
    
    // calc difference
    Vec3d l_diffC(0.0, 0.0, 0.0), l_diffPDC(0.0, 0.0, 0.0);
    getAvgOfDiffMat2(&l_diffC, _desiredImage, l_cameraImage);
    getAvgOfDiffMat2(&l_diffPDC, _desiredImage, l_cameraImageFromDesiredImageProjection);
    int index = (int)_desiredImage.at<Vec3b>(0,0)[0];
    _print_gnuplot7(cout, index, l_diffC[2], l_diffC[1], l_diffC[0], l_diffPDC[2], l_diffPDC[1], l_diffPDC[0]);
    Vec3b l_aveC, l_avePDC;
    calcAverageOfImage(&l_aveC, l_cameraImage);
    calcAverageOfImage(&l_avePDC, l_cameraImageFromDesiredImageProjection);
    _print3(index, l_avePDC, l_aveC);
    
    // show images
    imshow("desired C", _desiredImage);
    imshow("P on Camera Domain", l_projectionImageOnCameraSpace);
//    imshow("P on Projector Domain", l_projectionImageOnProjectorSpace);
//    imshow("I", l_LProjectionImage);
    imshow("C", l_cameraImage);
    imshow("C when projection desired C", l_cameraImageFromDesiredImageProjection);
    waitKey(30);
    
    return true;
}

bool LinearizerOfProjector::doRadiometricCompensation(const cv::Vec3b& _desiredColor, const int _waitTimeNum){
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    const Scalar l_color(_desiredColor);
    Mat l_image(*l_camSize, CV_8UC3, l_color);
    return doRadiometricCompensation(l_image, _waitTimeNum);
}
bool LinearizerOfProjector::doRadiometricCompensation(const uchar& _desiredColorNumber, const int _waitTimeNum){
    return doRadiometricCompensation(Vec3b(_desiredColorNumber, _desiredColorNumber, _desiredColorNumber), _waitTimeNum);
}

// P = V^{-1} * C を計算して，CからPを算出する
// output / _prjImg : P
// input / _camImg  : C
// return           : 成功したかどうか
bool LinearizerOfProjector::convertCameraImageToProjectorOne(cv::Mat* const _prjImg, const cv::Mat&  _camImg){
    // init
    const Mat_<Vec9d>* l_VMap = getColorMixMatMap();
    
    // error processing
    if (!isEqualSize(*_prjImg, _camImg, *l_VMap)) {
        cerr << "different size or type" << endl;
        _print_mat_propaty(*_prjImg);
        _print_mat_propaty(_camImg);
        _print_mat_propaty(*l_VMap);
        exit(-1);
    }
    
    // scanning all pixel
    Mat l_prjImg(_prjImg->rows, _prjImg->cols, CV_8UC3, CV_SCALAR_BLACK);
    int rows = _camImg.rows, cols = _camImg.cols;
    if (isContinuous(*_prjImg, _camImg)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pPrjImg = l_prjImg.ptr<Vec3b>(y);
        const Vec3b* l_pCamImg = _camImg.ptr<Vec3b>(y);
        const Vec9d* l_pVMap = l_VMap->ptr<Vec9d>(y);
        
        for (int x = 0; x < cols; ++ x) {
            // bgr -> rgb
            Vec3b l_rgbC(0, 0, 0);
            convertBGRtoRGB(&l_rgbC, l_pCamImg[x]);
            
            // Vec -> Mat
            Mat_<double> l_V(3, 3, CV_64FC1);
            convertVecToMat(&l_V, l_pVMap[x]);
            Mat l_C(l_rgbC);
            
            // uchar -> double
            l_C.convertTo(l_C, CV_64FC1, 1.0 / 255.0);
            
            // get inverse V
            Mat l_invV = l_V.inv();

            // P = V^{-1}C
            Mat l_P(3, 1, CV_64FC1);
            l_P = l_invV * l_C;
            
            // double -> uchar
            l_P.convertTo(l_P, CV_8UC1, 255);
            
            // Mat -> Vec
            Vec3b l_vecP(l_P);
            
            // rgb -> bgr
            Vec3b l_bgrP(0, 0, 0);
            convertRGBtoBGR(&l_bgrP, l_vecP);
            
            // copy
            l_pPrjImg[x] = l_bgrP;
        }
    }
    
    // deep copy
    *_prjImg = l_prjImg.clone();
    
    return true;
}
