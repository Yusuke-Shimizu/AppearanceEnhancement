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

class GeometricCalibration{
private:
    //cv::Point* accessMap;

public:
    enum stripeDirection {Vertical, Horizon};   // {縦，横}

    GeometricCalibration(void);
    GeometricCalibration(const cv::Size* const _size);
    bool doCalibration(void);
    void printCurrentPattern(const bool* const pattern, const int patternSize);
    void printPatternMap(const bool* const map, const cv::Size* const mapSize);
    void printAccessMap(const bool* const accessMap, const cv::Size* const mapSize, const int mapDepth);
    int calcBitCodeNumber(const int length);
    void makePureBinaryCode(bool *pattern, const unsigned int patternSize, const unsigned int layerNum);
    void makeGrayCodePattern(bool *pattern, const unsigned int patternSize, const unsigned int layerNum);
    void makeNegaCodePattern(bool *pattern, const bool *posiPattern, const unsigned int patternSize);
    void pattern2map(bool *map, const bool* const pattern, const cv::Size* const mapSize, const stripeDirection direction);
    void map2image(cv::Mat *image, const bool* const map, const cv::Size* const mapSize);
    void image2map(bool* const map, cv::Mat* const image, const cv::Size* const mapSize);
    void accessMap2image(cv::Mat *image, const cv::Point* const accessMap, const cv::Size* const mapSize, const cv::Size* const maxSize);
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
    void getProjectorPoint(cv::Point* const projector, const cv::Point* const camera, const cv::Point* const accessMapC2P, const int cameraWidth);
    void test_geometricCalibration(cv::Point* const accessMapC2P, cv::VideoCapture *video, const cv::Size* const cameraSize, const cv::Size* const projectorSize);

};

#endif /* defined(__cameraBase03__GeometricCalibration__) */
