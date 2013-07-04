//
//  GeometricCalibration.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/05/23.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#ifndef __cameraBase03__GeometricCalibration__
#define __cameraBase03__GeometricCalibration__

#include <iostream>
#include "myOpenCV.h"
#include "ProCam.h"

// 型による色の定義
#define BOOL_BLACK 0
#define BOOL_WHITE 1
#define CHAR_BLACK UCHAR_MIN
#define CHAR_WHITE UCHAR_MAX

// その他
#define MAX_WINDOW_SIZE 10000   // ウィンドウの最大値
#define BINARY_THRESH 0     // ネガポジの二値化を行うときの閾値
#define CAPTURE_NUM 10

// window name
#define W_NAME_GEO_CAMERA "geometric calibration camera"
#define W_NAME_GEO_PROJECTOR "geometric calibration projector"

// 先攻宣言
class ProCam;

class GeometricCalibration{
private:
    GeometricCalibration(const GeometricCalibration& gc);   // コピーコンストラクタ隠し（プログラムで１つしか存在しない為）
public:
    enum stripeDirection {Vertical, Horizon};   // {縦，横}

    GeometricCalibration(void);
    GeometricCalibration(const cv::Size* const _size);
    void printCurrentPattern(const bool* const pattern, const int patternSize);
    void printPatternMap(const bool* const map, const cv::Size* const mapSize);
    void printAccessMap(const bool* const accessMap, const cv::Size* const mapSize, const int mapDepth);
    int calcBitCodeNumber(const int length);
    void test_calcBitCodeNumber(void);
    void makePureBinaryCode(bool *pattern, const unsigned int patternSize, const unsigned int layerNum);
    bool getNthBit(const int num, const int Nth);
    void makeGrayCodePattern(bool *pattern, const unsigned int patternSize, const unsigned int layerNum);
    void makeGrayCodePattern2(bool *pattern, const unsigned int patternSize, const unsigned int layerNum);
    void makeNegaCodePattern(bool *pattern, const bool *posiPattern, const unsigned int patternSize);
    void pattern2map(bool *map, const bool* const pattern, const cv::Size* const mapSize, const stripeDirection direction);
    void map2image(cv::Mat *image, const bool* const map, const cv::Size* const mapSize);
    void image2map(bool* const map, cv::Mat* const image, const cv::Size* const mapSize);
    void accessMap2image(cv::Mat *image, const cv::Point* const accessMap, const cv::Size& mapSize, const cv::Size& maxSize);
    void createBinaryMap(bool *binaryMap, const cv::Size* const mapSize, const unsigned int layerNum, const stripeDirection direction);
    void insertAccessMap(bool* accessMap, const cv::Size* const mapSize, const int bitDepth, const bool* const patternMap, const int offsetBit);
    void test_insertAccessMap(void);
    void captureProjectionImage(cv::Mat* const captureImage, const cv::Mat* const projectionImage, cv::VideoCapture *videoStream);
    void addSpatialCodeOfProCam(bool* const spatialCodeProjector, bool* const spatialCodeCamera, const cv::Size* const projectorSize, const cv::Size* const cameraSize, const int patternLayerNum, const int offset, const stripeDirection direction, cv::VideoCapture *videoStream);
    void divideCode(bool* const codeX, bool* const codeY, const bool* const originalCode, const int sizeX, const int sizeY);
    void grayCode2binaryCode(bool* const binaryCode, const bool* const grayCode, const int bitNum);
    void test_grayCode2binaryCode(void);
    int binary2decimal(const bool* const binaryCode, const int depth);
    void test_grayCode2binaryCode_binary2decimal(void);
    int getPositionFromGrayCode(const bool* const grayCode, const int depth);
    void test_getPositionFromGrayCode(void);
    void setAccessMap(cv::Point* const c2pMap, const bool* codeMapCamera, const bool* codeMapProjector, const cv::Size* cameraSize, const cv::Size* projectorSize, const cv::Size* const depthSize);
    bool test_setAccessMap(void);
    void getProjectorPoint(cv::Point* const projector, const cv::Point* const camera, const cv::Point* const accessMapC2P, const int cameraWidth);
    void test_accessMap(const cv::Point* const accessMapCam2Pro, const cv::Size& cameraSize, const cv::Size& projectorSize, const char* _fileName);
    void test_geometricCalibration(cv::Point* const accessMapC2P, cv::VideoCapture *video, const cv::Size* const cameraSize, const cv::Size* const projectorSize);
    bool doCalibration(cv::Point* const _accessMapCam2Pro, cv::VideoCapture* video);
};

#endif /* defined(__cameraBase03__GeometricCalibration__) */
