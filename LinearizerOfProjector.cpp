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
    test_interpolateSimpleProjectorResponseP2I();
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
    
    // Color Convert
    // calc P on Camera Space
    Mat l_PImageOnCameraSpace(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);    // カメラ座標系におけるP（=V^{-1}C）
    convertCameraImageToProjectorOne(&l_PImageOnCameraSpace, _CImage);
    MY_IMSHOW(l_PImageOnCameraSpace);
    
    // Geometric Convert
    // convert P on Camera Space to Projector Space
    Mat l_PImageOnProjectorSpace(*l_prjSize, CV_8UC3, CV_SCALAR_BLACK);    // カメラ座標系におけるP（=V^{-1}C）
    l_procam->convertProjectorDomainToCameraOne(&l_PImageOnProjectorSpace, l_PImageOnCameraSpace);
    
    // create check mat
    const Mat l_whiteCameraImage(l_camSize->height, l_camSize->width, CV_8UC3, CV_SCALAR_WHITE);
    Mat l_whiteImageOnProjectorDomain(l_prjSize->height, l_prjSize->width, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->convertProjectorDomainToCameraOne(&l_whiteImageOnProjectorDomain, l_whiteCameraImage);
    const Vec3b l_nonProjectionColor(0, 0, 0);
    
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

// 応答特性マップを設定
// @input / _averageColor   : 設定するPの色
// @input / _INum           : 設定するIの値
// @output / _responseMap    : 応答特性マップ
// @return                   : 成功したかどうか
// ProjectorResponseの設定メソッドの為、ProCamクラスに移動すべき
bool LinearizerOfProjector::setSimpleResponseMap(const cv::Vec3b& _setColor, const uchar _INum){
    // error
    ProCam* l_procam = getProCam();
    const int l_channel = 3;
    
    // set response map at red, green, and blue
    for (int ch = 0; ch < l_channel; ++ ch) {
        // setting I2P
        l_procam->setSimpleProjectorResponseI2P(_setColor[ch], _INum, ch);

        // setting P2I
        const uchar l_responseNum = l_procam->getSimpleProjectorResponseP2I(_setColor[ch], ch);
        if (_setColor[ch] == 255 || l_responseNum == 0) { // 文字が被り，別の値で変更をしないようにする
            l_procam->setSimpleProjectorResponseP2I(_INum, _setColor[ch], ch);
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

// C=VPからVPを計算し推定したCを出力
bool LinearizerOfProjector::saveEstimatedC(void){
    ProCam* l_procam = getProCam();
    const Size l_camSize = l_procam->getCameraSize_();
    Mat l_estimatedC(l_camSize, CV_64FC3, CV_SCALAR_BLACK);
    
    for (int i = 1; i < 8; ++ i) {
        if (i == 3 || i >= 5) {
            continue;
        }
        const int l_col1 = i % 2;
        const int l_col2 = (i / 2) % 2;
        const int l_col3 = (i / 4) % 2;
        Vec3b l_mask(l_col3, l_col2, l_col1);
        ostringstream oss;
        oss <<EST_C_DATA_PATH << l_col3 << l_col2 << l_col1 << ".dat";
        cout << oss.str() << endl;
        ofstream ofs(oss.str().c_str());
        
        for (int prj = 0; prj < 256; ++ prj) {
            // make C
            Vec3b l_color(prj * l_mask[0], prj * l_mask[1], prj * l_mask[2]);
            estimateC(&l_estimatedC, l_color);
            
            // get mean and standard deviation
            Vec3d l_mean(0, 0, 0), l_stddev(0, 0, 0);
            meanStdDev(l_estimatedC, l_mean, l_stddev);
            
            // print
            std::cout << prj << "\t";
            _print_gnuplot2(std::cout, l_mean, l_stddev);
            ofs << prj << "\t";
            _print_gnuplot_color2_l(ofs, l_mean, l_stddev);
        }
        ofs.close();
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

///////////////////////////////  caprj method ///////////////////////////////
bool LinearizerOfProjector::captureFromLightOnGeoP_ColP(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, ProCam* _procam, const bool _denoiseFlag, const int _waitTimeNum){
    // capture from projection image
    Mat l_captureImage(_captureImage->rows, _captureImage->cols, CV_8UC3, CV_SCALAR_BLACK);
    _procam->captureFromLightOnProjectorDomain(&l_captureImage, _projectionImage, true, _waitTimeNum);
    
    // Color Convert
    convertCameraImageToProjectorOne(_captureImage, l_captureImage);
    return true;
}
bool LinearizerOfProjector::captureFromLightOnGeoP_ColP(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, ProCam* _procam, const bool _denoiseFlag, const int _waitTimeNum){
    const Size* l_prjSize = _procam->getCameraSize();
    const Mat l_projectionImage(*l_prjSize, CV_8UC3, Scalar(_projectionColor));
    return captureFromLightOnGeoP_ColP(_captureImage, l_projectionImage, _procam, _denoiseFlag, _waitTimeNum);
}
bool LinearizerOfProjector::captureFromLightOnGeoP_ColP(cv::Mat* const _captureImage, const uchar _projectionNumber, ProCam* _procam, const bool _denoiseFlag, const int _waitTimeNum){
    const Vec3b l_projectionColor(_projectionNumber, _projectionNumber, _projectionNumber);
    return captureFromLightOnGeoP_ColP(_captureImage, l_projectionColor, _procam, _denoiseFlag, _waitTimeNum);
}

// 線形化したものを投影・撮影
bool LinearizerOfProjector::captureFromLinearLightOnGeoP_ColP(cv::Mat* const _captureImage, const cv::Mat& _projectionImage, ProCam* _procam, const bool _denoiseFlag, const int _waitTimeNum){
    
    return true;
}
bool LinearizerOfProjector::captureFromLinearLightOnGeoP_ColP(cv::Mat* const _captureImage, const cv::Vec3b& _projectionColor, ProCam* _procam, const bool _denoiseFlag, const int _waitTimeNum){
    return true;
}
bool LinearizerOfProjector::captureFromLinearLightOnGeoP_ColP(cv::Mat* const _captureImage, const uchar _projectionNumber, ProCam* _procam, const bool _denoiseFlag, const int _waitTimeNum){
    return true;
}

//////////////////////////////  convert method //////////////////////////////
bool LinearizerOfProjector::convertColorSpace(cv::Mat* const _dst, const cv::Mat& _src, const bool _useVFlag){
    // init
    const Mat_<Vec9d>* l_VMap = getColorMixMatMap();
    
    // error processing
    if (!isEqualSize(*_dst, _src, *l_VMap)) {
        cerr << "different size or type" << endl;
        _print_mat_propaty(*_dst);
        _print_mat_propaty(_src);
        _print_mat_propaty(*l_VMap);
        exit(-1);
    }
    
    // scanning all pixel
    Mat l_prjImg(_dst->rows, _dst->cols, CV_8UC3, CV_SCALAR_BLACK);
    int rows = _src.rows, cols = _src.cols;
    if (isContinuous(*_dst, _src)) {
        cols *= rows;
        rows = 1;
    }
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pPrjImg = l_prjImg.ptr<Vec3b>(y);
        const Vec3b* l_pCamImg = _src.ptr<Vec3b>(y);
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
            
            // get convert matrix
            Mat l_convertMat = l_V.inv();
            if (_useVFlag) {
                l_convertMat = l_V;
            }
            
            // P = V^{-1}C
            Mat l_P(3, 1, CV_64FC1);
            l_P = l_convertMat * l_C;
            
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
    *_dst = l_prjImg.clone();
    
    return true;
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

//
bool LinearizerOfProjector::convertColorSpaceUseV(cv::Mat* const _dst, const cv::Mat& _src){
    convertColorSpace(_dst, _src, true);
    return true;
}

///////////////////////////////  other method ///////////////////////////////
/**
 * @brief プロジェクタの線形化を行うメソッド
 * @output _responseOfProjector : 線形化ルックアップテーブルを入れる配列
 * @output _responseMapP2I      : _responseMapの逆関数
 * @return 成功したかどうか
 */
bool LinearizerOfProjector::doLinearlize(cv::Mat_<cv::Vec3b>* const _responseOfProjector, cv::Mat_<cv::Vec3b>* const _responseMapP2I){
    ///////////////// create V map /////////////////
    // 色変換行列の生成
    cout << "creating Color Mixing Matrix..." << endl;
//    if( !calcColorMixingMatrix() ) return false;
    calcMoreDetailColorMixingMatrix();
    cout << "created Color Mixing Matrix" << endl;
    
    // show V map
    showVMap();

    ///////////////// create projector response function /////////////////
    // プロジェクタの応答特性を計算
    cout << "creating response function..." << endl;
    if ( !calcResponseFunction(_responseOfProjector)) return false;
//    if ( !calcSimpleResponseFunction()) return false;
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
    while (true) {
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

        cout << "キャリブレーションは正しいですか？(y/n)" << endl;
        if (yes_no()) {
            break;
        }
    }
    
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
    // init projection color
    std::vector<Vec3b> l_vecPrjColor;
    l_vecPrjColor.push_back(Vec3b(0, 0, 0));        // 0
    l_vecPrjColor.push_back(Vec3b(0, 0, g_maxPrjLuminance[2]));      // 1
    l_vecPrjColor.push_back(Vec3b(0, g_maxPrjLuminance[1], 0));      // 2
    l_vecPrjColor.push_back(Vec3b(g_maxPrjLuminance[0], 0, 0));      // 3
    l_vecPrjColor.push_back(Vec3b(0, g_maxPrjLuminance[1], g_maxPrjLuminance[2]));    // 4
    l_vecPrjColor.push_back(Vec3b(g_maxPrjLuminance[0], g_maxPrjLuminance[1], 0));    // 5
    l_vecPrjColor.push_back(Vec3b(g_maxPrjLuminance[0], 0, g_maxPrjLuminance[2]));    // 6
    l_vecPrjColor.push_back(Vec3b(g_maxPrjLuminance[0], g_maxPrjLuminance[1], g_maxPrjLuminance[2]));  // 7
    
    // get capture
    ProCam *l_procam = getProCam();
    const Size l_camSize(l_procam->getCameraSize_());
    std::vector<cv::Mat> l_vecCaptureImage;
    l_procam->switchOnDenoiseFlag();
    while (true) {
        Mat l_captureImage(l_camSize, CV_8UC3, CV_SCALAR_BLACK);
        l_procam->captureFromLightOnProjectorDomain(&l_captureImage, CV_VEC3B_BLACK, true, SLEEP_TIME);
        for (vector<Vec3b>::const_iterator l_itrPrj = l_vecPrjColor.begin();
             l_itrPrj != l_vecPrjColor.end();
             ++ l_itrPrj) {
            // capture
            l_procam->captureFromLightOnProjectorDomain(&l_captureImage, *l_itrPrj);
            
            // push
            l_vecCaptureImage.push_back(l_captureImage);
        }
        
        int i = 0;
        for (vector<Mat>::const_iterator l_itrCapImg = l_vecCaptureImage.begin();
             l_itrCapImg != l_vecCaptureImage.end();
             ++ l_itrCapImg, ++ i) {
            ostringstream oss;
            oss << "linearize" << i;
            imshow(oss.str().c_str(), *l_itrCapImg);
        }

        // 投影撮影出来てるなら抜ける
        cout << "Is capture image true?" << endl;
        if (yes_no()) {
            break;
        }
    }
    l_procam->switchOffDenoiseFlag();
    
    // translate bit depth (uchar[0-255] -> double[0-1])
    std::vector<cv::Mat> l_vecCaptureImage_d;
    const double l_rateUC2D = 1.0 / 255.0;
    for (vector<Mat>::const_iterator l_itrCapImg = l_vecCaptureImage.begin();
         l_itrCapImg != l_vecCaptureImage.end();
         ++ l_itrCapImg) {
        Mat l_captureImage_d(l_camSize, CV_64FC3, CV_SCALAR_D_BLACK);
        l_itrCapImg->convertTo(l_captureImage_d, CV_64FC3, l_rateUC2D);
        l_vecCaptureImage_d.push_back(l_captureImage_d);
    }
    
    // create front and back image
    std::vector<Mat> l_vecFront, l_vecBack;
    // change red
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 1, 0);
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 4, 2);
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 6, 3);
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 7, 5);
    // change green
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 2, 0);
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 4, 1);
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 5, 3);
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 7, 6);
    // change blue
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 3, 0);
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 6, 1);
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 5, 2);
    addTwoVector(&l_vecFront, &l_vecBack, l_vecCaptureImage_d, 7, 4);
    
    // calc difference[-1-1]
    std::vector<Mat> l_vecDiff;
    std::vector<Mat>::const_iterator l_itrFront = l_vecFront.begin();
    std::vector<Mat>::const_iterator l_itrBack = l_vecBack.begin();
    for (; l_itrFront != l_vecFront.end() || l_itrBack != l_vecBack.end();
         ++ l_itrFront, ++ l_itrBack) {
        Mat l_diffImage(l_camSize, CV_64FC3, CV_SCALAR_D_BLACK);
        l_diffImage = *l_itrFront - *l_itrBack;
        l_vecDiff.push_back(l_diffImage);
    }
    
    // devide mat along channel to change
    std::vector<Mat> l_vecDiffRed, l_vecDiffGreen, l_vecDiffBlue;
    const int l_length = (int)l_vecDiff.size() / 3;
    for (int i = 0; i < l_length; ++ i) {
        // normalize
        Mat l_diffImgRed = l_vecDiff[i];
        Mat l_diffImgGreen = l_vecDiff[i + l_length];
        Mat l_diffImgBlue = l_vecDiff[i + l_length * 2];
        normalizeByAnyColorChannel(&l_diffImgRed, CV_RED);
        normalizeByAnyColorChannel(&l_diffImgGreen, CV_GREEN);
        normalizeByAnyColorChannel(&l_diffImgBlue, CV_BLUE);

        l_vecDiffRed.push_back(l_diffImgRed);
        l_vecDiffGreen.push_back(l_diffImgGreen);
        l_vecDiffBlue.push_back(l_diffImgBlue);
    }
    
    // calc mean
    std::vector<Mat>::iterator l_itrDiffRed = l_vecDiffRed.begin();
    std::vector<Mat>::iterator l_itrDiffGreen = l_vecDiffGreen.begin();
    std::vector<Mat>::iterator l_itrDiffBlue = l_vecDiffBlue.begin();
    Mat l_sumRed(l_camSize, CV_64FC3, CV_SCALAR_D_BLACK);
    Mat l_sumGreen(l_camSize, CV_64FC3, CV_SCALAR_D_BLACK);
    Mat l_sumBlue(l_camSize, CV_64FC3, CV_SCALAR_D_BLACK);
    for (; l_itrDiffRed != l_vecDiffRed.end() ||
         l_itrDiffGreen != l_vecDiffGreen.end() ||
         l_itrDiffBlue != l_vecDiffBlue.end();
         ++ l_itrDiffRed, ++ l_itrDiffGreen, ++ l_itrDiffBlue) {
        l_sumRed += *l_itrDiffRed;
        l_sumGreen += *l_itrDiffGreen;
        l_sumBlue += *l_itrDiffBlue;
    }
    l_sumRed /= l_length;
    l_sumGreen /= l_length;
    l_sumBlue /= l_length;
    
    // create V map
    createVMap(l_sumRed, l_sumGreen, l_sumBlue);
    
    // save
    cout << "saving Color Mixing Matrix..." << endl;
    if ( !saveColorMixingMatrixOfByte(CMM_MAP_FILE_NAME_BYTE) ) return false;
    cout << "saved Color Mixing Matrix" << endl;
    
    return true;
}
inline bool LinearizerOfProjector::addTwoVector(std::vector<cv::Mat>* _dst1, std::vector<cv::Mat>* _dst2, const std::vector<cv::Mat>& _src, const int _ref1, const int _ref2){
    _dst1->push_back(_src[_ref1]);
    _dst2->push_back(_src[_ref2]);
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
//        const double* pNormalR = _normalR2BL.ptr<double>(y);
//        const double* pNormalG = _normalG2BL.ptr<double>(y);
//        const double* pNormalB = _normalB2BL.ptr<double>(y);
        const Vec3d* l_pNormalR = _normalR2BL.ptr<Vec3d>(y);
        const Vec3d* l_pNormalG = _normalG2BL.ptr<Vec3d>(y);
        const Vec3d* l_pNormalB = _normalB2BL.ptr<Vec3d>(y);
        Vec9d* p_cmmm = l_cmmm.ptr<Vec9d>(y);

        for (int x = 0; x < cols; ++ x) {
            // assigne V
            for (int c = 0; c < ch; ++ c) {
//                V.at<double>(0, c) = l_pNormalR[x][ch - 1 - c];
//                V.at<double>(1, c) = l_pNormalG[x][ch - 1 - c];
//                V.at<double>(2, c) = l_pNormalB[x][ch - 1 - c];
                
                V.at<double>(c, 0) = l_pNormalR[x][ch - 1 - c];
                V.at<double>(c, 1) = l_pNormalG[x][ch - 1 - c];
                V.at<double>(c, 2) = l_pNormalB[x][ch - 1 - c];
            }

            // push V
            convertMatToVec(&l_VVec, V);    // mat -> vec
            p_cmmm[x] = l_VVec;
        }
    }
    
    // setting
    setColorMixMatMap(l_cmmm);
    return true;
}
bool LinearizerOfProjector::test_createVMap(void){
    const Size l_size(10, 8);
    Mat l_red(l_size, CV_64FC3, Scalar(0.0, 0.1, 0.2));
    Mat l_blue(l_size, CV_64FC3, Scalar(0.3, 0.4, 0.5));
    Mat l_green(l_size, CV_64FC3, Scalar(0.6, 0.7, 0.8));
    createVMap(l_red, l_green, l_blue);
    
    return true;
}

// 応答特性を計算する
// output / _responseOfProjector    : 計算した応答特性を入れる変数
// return                           : 成功したかどうか
/**
 * @brief 応答特性を計算
 * @output _responseMap    : 線形化ルックアップテーブルを入れる配列
 * @output _responseMapP2I : _responseMapの逆関数
 * @return 成功可否
 */
bool LinearizerOfProjector::calcResponseFunction(cv::Mat_<cv::Vec3b>* const _responseMap){
    // init
    ProCam* l_procam = getProCam();     // ProCamへのポインタ
    const Size* const l_cameraSize = l_procam->getCameraSize();         // カメラサイズ
    const Size* const l_projectorSize = l_procam->getProjectorSize();   // プロジェクタサイズ
    const Size l_prjResSize = l_procam->getProjectorResponseI2P()->size(); // プロジェクタ応答特性サイズ
    Mat l_camImage = Mat::zeros(*l_cameraSize, CV_8UC3);
    Mat prjImage = Mat::zeros(*l_projectorSize, CV_8UC3);
    Mat_<Vec3b> l_responseMap(l_prjResSize); // _responseMapの一時的な置き場
    Mat_<Vec3b> l_responseMapP2I(l_prjResSize);
    
    // projection RGB * luminance
    // scanning all luminance[0-255] of projector
    int prjLuminance = 0;
    _print(prjLuminance);
    Mat l_blackImage(*l_cameraSize, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->captureFromLightOnProjectorDomain(&l_blackImage, prjLuminance, true, SLEEP_TIME * 2);
    for (prjLuminance = 0; prjLuminance < 256; prjLuminance += PROJECTION_LUMINANCE_STEP) {
        _print(prjLuminance);
        
        // capture from projection image
        l_procam->captureFromLightOnProjectorDomain(&l_camImage, prjLuminance, true);
        if (USE_LOOK_LIKE_ANOTHER_FLAG) {
            l_camImage -= l_blackImage;
        }
        //        MY_IMSHOW(camImage);
        ostringstream oss;
        oss << PROJECTOR_RESPONSE_C_IMAGE_PATH << prjLuminance << ".png" << endl;
        imwrite(oss.str().c_str(), l_camImage);
        waitKey(1);
        
        // set inverce response function(P2I and I2P)
        setResponseMap(&l_responseMapP2I, &l_responseMap, l_camImage, prjLuminance);
        
        // set all C images
        setCImages(l_camImage, prjLuminance);
    }
    
    // interpolation projector response P to I
    interpolationProjectorResponseP2I(&l_responseMapP2I);
    interpolateProjectorResponseP2IAtOutOfCameraArea(&l_responseMapP2I);
    l_procam->medianBlurForProjectorResponseP2I(&l_responseMapP2I, l_responseMapP2I);
    
    // set and save
    cout << "saving response function" << endl;
    // P2I
    l_procam->setProjectorResponseP2I(l_responseMapP2I);
    l_procam->saveProjectorResponseP2IForByte(PROJECTOR_RESPONSE_P2I_FILE_NAME_BYTE);
    // I2P
    l_procam->setProjectorResponse(l_responseMap);
    l_procam->saveProjectorResponseForByte(PROJECTOR_RESPONSE_I2P_FILE_NAME_BYTE);
    // all C images
    saveAllCImages();
    
    // deep copy
//    *_responseMap = l_responseMap.clone();          // I2P
//    *_responseMapP2I = l_responseMapP2I.clone();    // P2I
    cout << "finished saving response function" << endl;
    
    //test
    //    test_responseFunction();
    
    return true;
}

// シンプルな応答特性を計算する
// return                           : 成功したかどうか
/**
 * 流れ：投影・撮影・平均化・セッティング・補間
 */
bool LinearizerOfProjector::calcSimpleResponseFunction(void){
    // init
    ProCam* l_procam = getProCam();     // ProCamへのポインタ
    const Size* const l_cameraSize = l_procam->getCameraSize();         // カメラサイズ
    const Size* const l_projectorSize = l_procam->getProjectorSize();   // プロジェクタサイズ
    Vec3b l_captureImageColor(0,0,0);
//    Vec3b* l_responseMap = new Vec3b[256]; // _responseMapの一時的な置き場
//    Vec3b* l_responseMapP2I = new Vec3b[256];

    // projection RGB * luminance
    // scanning all luminance[0-255] of projector
    int prjLuminance = 0;
    Vec3b l_blackImageColor(0,0,0);
    l_procam->captureFromLightOnProjectorDomain(&l_blackImageColor, prjLuminance, true, SLEEP_TIME * 2);
    for (prjLuminance = 0; prjLuminance < 256; prjLuminance += PROJECTION_LUMINANCE_STEP) {
        _print(prjLuminance);
        
        // capture from projection image
        l_procam->captureFromLightOnProjectorDomain(&l_captureImageColor, prjLuminance, true);
        if (USE_LOOK_LIKE_ANOTHER_FLAG) {
            l_captureImageColor -= l_blackImageColor;
        }

        // set inverce response function(P2I and I2P)
        setSimpleResponseMap(l_captureImageColor, prjLuminance);
    }

    // interpolation projector response P to I
    interpolateSimpleProjectorResponseP2I();
//    interpolateSimpleProjectorResponseP2IAtOutOfCameraArea();
    
//    // set and save
//    cout << "saving response function" << endl;
//    // P2I
//    l_procam->setProjectorResponseP2I(l_responseMapP2I);
//    l_procam->saveProjectorResponseP2IForByte(PROJECTOR_RESPONSE_P2I_FILE_NAME_BYTE);
//    // I2P
//    l_procam->setProjectorResponse(l_responseMap);
//    l_procam->saveProjectorResponseForByte(PROJECTOR_RESPONSE_I2P_FILE_NAME_BYTE);
//    // all C images
//    saveAllCImages();
//    
//    // deep copy
//    *_responseMap = l_responseMap.clone();          // I2P
//    *_responseMapP2I = l_responseMapP2I.clone();    // P2I
//    cout << "finished saving response function" << endl;
//    
//    //test
//    //    test_responseFunction();
//    
    return true;
}

// 線形化出来てるかを線形化した投影光で０から２５５まで投影し，直線になるかを確認
void LinearizerOfProjector::test_responseFunction(void){
    // init
    ProCam* l_procam = getProCam();
    const Size* l_camSize = l_procam->getCameraSize();
    Mat l_captureImage(*l_camSize, CV_8UC3, CV_SCALAR_BLACK);
    
    ofstream ofs(LIN_TEST_DATA_PATH.c_str());
    for (int i = 0; i < 256; ++ i) {
        Vec3d l_mean(0,0,0), l_stddev(0,0,0);
        captureFromLightOnGeoP_ColP(&l_captureImage, Vec3b(i, 0, 0), l_procam);
        meanStdDev(l_captureImage, l_mean, l_stddev);
        MY_IMSHOW(l_captureImage);
        
        // output
        std::cout << i << "\t";
        _print_gnuplot2(std::cout, l_mean, l_stddev);
        
        // save
        ofs << i << "\t";
        _print_gnuplot_color2_l(ofs, l_mean, l_stddev);
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
    const Size l_camSize = l_procam->getCameraSize_();
    const Scalar l_color(_desiredColor);
    Mat l_image(l_camSize, CV_8UC3, l_color);
    return doRadiometricCompensation(l_image, _waitTimeNum);
}
bool LinearizerOfProjector::doRadiometricCompensation(const uchar& _desiredColorNumber, const int _waitTimeNum){
    return doRadiometricCompensation(Vec3b(_desiredColorNumber, _desiredColorNumber, _desiredColorNumber), _waitTimeNum);
}

bool LinearizerOfProjector::estimateC(cv::Mat* const _estC, const cv::Vec3b& _P){
    ProCam* l_procam = getProCam();
    const Size l_camSize = l_procam->getCameraSize_();
    const Mat l_PImage(l_camSize, CV_8UC3, cv::Scalar(_P));
    return estimateC(_estC, l_PImage);
}
bool LinearizerOfProjector::estimateC(cv::Mat* const _estC, const cv::Mat& _P){
    return convertColorSpaceUseV(_estC, _P);
}

// projector responseの補間を行う
// 要変更
// 入力と出力が同じファイルなので、別々にすべき
bool LinearizerOfProjector::interpolationProjectorResponseP2I(cv::Mat* const _prjRes){
    cout << "interpolation now ..." << endl;
    // initialize
    int rows = _prjRes->rows, cols = _prjRes->cols / 256, channels = _prjRes->channels();
    if (_prjRes->isContinuous()) {
        cols *= rows;
        rows = 1;
    }
    
    // scan all pixel
    for (int y = 0; y < rows; ++ y) {
        Vec3b* l_pPrjRes = _prjRes->ptr<Vec3b>(y);
        for (int x = 0; x < cols; ++ x) {
            
            // scan all color
            for (int c = 0; c < channels; ++ c) {
                // scan all luminance
                for (int p = 1; p < 255; ++ p) {    // 最初と最後は初期化の時に決定済みである為，除外
                    if (l_pPrjRes[x * 256 + p][c] == INIT_RES_NUM) {   // 抜けている箇所の特定
                        uchar p0 = p - 1, i0 = l_pPrjRes[x * 256 + p0][c];  // 下端の値
                        
                        // 上端の検索
                        uchar p1 = p + 1, i1 = 0;
                        for (; ; ++ p1) {
                            if (l_pPrjRes[x * 256 + p1][c] != INIT_RES_NUM || p1 == 255) {
                                i1 = l_pPrjRes[x * 256 + p1][c];
                                break;
                            }
                        }
                        //                        if (p == 1 && p1 == 255) {
                        //                            l_pPrjRes[x * 256 + p1][c] = INIT_RES_NUM;
                        //                            break;
                        //                        }
                        
                        // 抜けていた箇所全てを修復
                        for (int p_current = p; p_current < p1; ++ p_current) {
                            // set alpha
                            const double alpha = (double)(p_current - p0)/(double)(p1 - p0);
                            
                            // 抜けている箇所の修復
                            l_pPrjRes[x * 256 + p_current][c] = i0 + (uchar)(alpha * (double)(i1 - i0));
                        }
                    }
                }
            }
        }
    }
    
    cout << "finished interpolation" << endl;
    
    return true;
}
// test of front method
bool LinearizerOfProjector::test_interpolationProjectorResponseP2I(void){
    Mat m1(2, 256, CV_8UC3, Scalar(INIT_RES_NUM, INIT_RES_NUM, INIT_RES_NUM));
    //    m1.at<Vec3b>(0, 255) = Vec3b(127, 127, 127);
    //    m1.at<Vec3b>(1, 255) = Vec3b(0, 0, 0);
    for (int i = 0; i < 256; i += 5) {
        m1.at<Vec3b>(0, i) = Vec3b(i, i, i);
        m1.at<Vec3b>(1, i) = Vec3b(0, 0, 0);
        if (i > 128) {
            m1.at<Vec3b>(1, i) = CV_VEC3B_WHITE;
        }
    }
    //    m1.at<Vec3b>(1, 255) = Vec3b(255, 255, 255);
    _print(m1);
    interpolationProjectorResponseP2I(&m1);
    _print(m1);
    
    return true;
}

/**
 * @brief interpolate m_simpleProjectorResponseP2I
 * @output success or fail
 */
bool LinearizerOfProjector::interpolateSimpleProjectorResponseP2I(void){
    // init
    ProCam* l_procam = getProCam();
    const Vec3b* l_srcPrjRes = l_procam->getSimpleProjectorResponseP2I();
    Vec3b* l_dstPrjRes = new Vec3b[256];    // マジックコード（変えたい）
    
    // do
    interpolateSimpleProjectorResponseP2I(l_dstPrjRes, l_srcPrjRes);
    
    // output
    l_procam->setSimpleProjectorResponseP2I(l_dstPrjRes);
    return true;
}

bool LinearizerOfProjector::interpolateSimpleProjectorResponseP2I(cv::Vec3b* const _dst, const cv::Vec3b* const _src){
    cout << "interpolation now ..." << endl;
//    printVec3bArray(_src, 256);
    // initialize
    int channels = 3;
    
    // scan all color
    for (int c = 0; c < channels; ++ c) {
        // scan all luminance
        for (int p = 0; p < 256; ++ p) {
            // 最初と最後は初期化の時に決定済みである為，除外
            if (p == 0 || p == 255) {
                continue;
            }
            
            if (_src[p][c] == INIT_RES_NUM) {   // 抜けている箇所の特定
                // 下端の決定
                uchar p0 = (uchar)(p - 1), i0 = _src[p0][c];
                
                // 上端の検索
                uchar p1 = p + 1, i1 = 0;
                for (; ; ++ p1) {
                    if (_src[p1][c] != INIT_RES_NUM || p1 == 255) {
                        i1 = _src[p1][c];
                        break;
                    }
                }
                
                // 抜けていた箇所全てを修復
                int p_current;
                for (p_current = p; p_current < p1; ++ p_current) {
                    // set alpha
                    const double alpha = (double)(p_current - p0)/(double)(p1 - p0);
                    
                    // 抜けている箇所の修復
                    _dst[p_current][c] = i0 + (uchar)(alpha * (double)(i1 - i0));
//                    _print3((int)i0, (int)i1, alpha);
//                    _print3(p_current, c, _dst[p_current]);
//                    _print2(_dst[p_current], p_current);
                }
                
                // ポインタを修復箇所へ移動
                p = p_current;
                _dst[p][c] = _src[p][c];
            }
        }
    }
//    printVec3bArray(_dst, 256);
    _print_name(_dst);
    for (int i = 0; i < 256; ++ i) {
        _print3(i, _src[i], _dst[i]);
    }
    cout << "finished interpolation" << endl;
    
    return true;
}
// test of front method
bool LinearizerOfProjector::test_interpolateSimpleProjectorResponseP2I(void){
    // init
    Vec3b* l_src = new Vec3b[256];
    Vec3b* l_dst = new Vec3b[256];
    Vec3b* l_ans = new Vec3b[256];
    setVec3bArray(l_src, l_dst, l_ans, CV_VEC3B_BLACK, 256, 0, 256, 1);
    
    // test case 01
    std::cout << "test case 01 is";
    setVec3bArray(l_src, l_dst, l_ans, CV_VEC3B_BLACK, 256, 0, 256, 1);
    l_src[255] = CV_VEC3B_WHITE;
    // create
    for (int i = 0; i < 256; i += 5) {
        l_src[i] = CV_VEC3B_FLAT_GRAY(i);
        for (int j = i; j < i + 5 | j < 256; ++ j) {
            l_ans[j] = CV_VEC3B_FLAT_GRAY(j);
        }
    }
    interpolateSimpleProjectorResponseP2I(l_dst, l_src);
    if(isEqualVec3b(l_ans, l_dst, 256)){
        std::cout << "ok" << std::endl;
    } else {
        std::cout << "ng" << std::endl;
        for (int i = 0; i < 256; ++ i) {
            _print2(l_ans[i], l_dst[i]);
        }
    }
    
    // test case 02
    _print("test case 02");
    std::cout << "test case 01 is";
    setVec3bArray(l_src, l_dst, l_ans, CV_VEC3B_BLACK, 256, 0, 256, 1);
    l_src[255] = CV_VEC3B_WHITE;
    // create
    for (int i = 0; i < 256; i += 100) {
        l_src[i] = CV_VEC3B_FLAT_GRAY(i);
        for (int j = i; j < i + 100 | j < 256; ++ j) {
            l_ans[j] = CV_VEC3B_FLAT_GRAY(j);
        }
    }
    interpolateSimpleProjectorResponseP2I(l_dst, l_src);
    if(isEqualVec3b(l_ans, l_dst, 256)){
        std::cout << "l_answer = l_dst" << std::endl;
    } else {
        for (int i = 0; i < 256; ++ i) {
            _print2(l_ans[i], l_dst[i]);
        }
        std::cout << "l_answer != l_dst" << std::endl;
    }
    
    // test case 03
    std::cout << "test case 01 is";
    setVec3bArray(l_src, l_dst, l_ans, CV_VEC3B_BLACK, 256, 0, 256, 1);
    l_src[255] = CV_VEC3B_WHITE;
    // create
    for (int i = 0, j = 0; i < 256; i += 10, ++ j) {
        l_src[i] = CV_VEC3B_FLAT_GRAY(j);
        for (int k = i; k < i + 10 | k < 256; ++ k) {
            l_ans[k] = CV_VEC3B_FLAT_GRAY(j);
        }
    }
    interpolateSimpleProjectorResponseP2I(l_dst, l_src);
    if(isEqualVec3b(l_ans, l_dst, 256)){
        std::cout << "l_answer = l_dst" << std::endl;
    } else {
        for (int i = 0; i < 256; ++ i) {
            _print2(l_ans[i], l_dst[i]);
        }
        std::cout << "l_answer != l_dst" << std::endl;
    }
    
    // test case 04
    std::cout << "test case 01 is";
    setVec3bArray(l_src, l_dst, l_ans, CV_VEC3B_BLACK, 256, 0, 256, 1);
    l_src[255] = CV_VEC3B_WHITE;
    // create
    for (int i = 0, j = 0; i < 256; i += 10, ++ j) {
        l_src[j] = CV_VEC3B_FLAT_GRAY(i);
        l_ans[j] = CV_VEC3B_FLAT_GRAY(i);
    }
    for (int i = 26; i < 256; ++ i) {
        l_ans[i] = CV_VEC3B_WHITE;
    }
    interpolateSimpleProjectorResponseP2I(l_dst, l_src);
    if(isEqualVec3b(l_ans, l_dst, 256)){
        std::cout << "l_answer = l_dst" << std::endl;
    } else {
        for (int i = 0; i < 256; ++ i) {
            _print2(l_ans[i], l_dst[i]);
        }
        std::cout << "l_answer != l_dst" << std::endl;
    }
    
    return true;
}

// カメラ撮影場所以外の応答特性を設定
bool LinearizerOfProjector::interpolateProjectorResponseP2IAtOutOfCameraArea(cv::Mat* const _prjResP2I){
    // init
    ProCam* l_procam = getProCam();

    // error handle
    const Size* l_prjSize = l_procam->getProjectorSize();
    if (_prjResP2I->rows != l_prjSize->height || _prjResP2I->cols != l_prjSize->width * 256) {
        cerr << "different size from ["<<l_prjSize->height<<", "<<l_prjSize->height * 256<<"]" << endl;
        _print_mat_propaty(*_prjResP2I);
        exit(-1);
    }
    
    // create non camera area flag
    const Size* l_camSize = l_procam->getCameraSize();
    const Mat whiteImage(*l_camSize, CV_8UC3, CV_SCALAR_WHITE);
    Mat l_checkNonCameraAreaFlag(*l_prjSize, CV_8UC3, CV_SCALAR_BLACK);
    l_procam->convertProjectorDomainToCameraOne(&l_checkNonCameraAreaFlag, whiteImage);
    
    // set non camera area
    for (int P = 0; P < 256; ++ P) {
        Vec3b l_aveColor(0, 0, 0);
        Vec3d l_sum(0, 0, 0);
        int cnt = 0;
        
        // calc sum color
        for (int y = 0; y < l_prjSize->height; ++ y) {
            const Vec3b* l_pCheckNonCameraAreaFlag = l_checkNonCameraAreaFlag.ptr<Vec3b>(y);
            const Vec3b* l_pPrjResP2I = _prjResP2I->ptr<Vec3b>(y);
            
            for (int x = 0; x < l_prjSize->width; ++ x) {
                if (l_pCheckNonCameraAreaFlag[x] == CV_VEC3B_WHITE) {
                    l_sum += l_pPrjResP2I[x * 256 + P];
                    ++ cnt;
                }
            }
        }
        
        // get average
        //        l_aveColor = (Vec3b)(l_sum / (l_camSize->area()));
        l_aveColor = (Vec3b)(l_sum / (double)cnt);
        
        // set response at out of camera area
        for (int y = 0; y < l_prjSize->height; ++ y) {
            const Vec3b* l_pCheckNonCameraAreaFlag = l_checkNonCameraAreaFlag.ptr<Vec3b>(y);
            Vec3b* l_pPrjResP2I = _prjResP2I->ptr<Vec3b>(y);
            
            for (int x = 0; x < l_prjSize->width; ++ x) {
                if (l_pCheckNonCameraAreaFlag[x] == CV_VEC3B_BLACK) {
                    l_pPrjResP2I[x * 256 + P] = l_aveColor;
                }
            }
        }
    }
    
    return true;
}

