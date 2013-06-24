//
//  myOpenCV.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/05/10.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#ifndef cameraBase03_myOpenCV_h
#define cameraBase03_myOpenCV_h

#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>

#define OPENCV_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define OPENCV_VERSION_CODE OPENCV_VERSION(CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION)

//const double M_PI = 3.14159265358979;

// set method
void setColor(const cv::MatIterator_<cv::Vec3b> it, const uchar red, const uchar green, const uchar blue);
void setColor(const cv::MatIterator_<cv::Vec3b> it, const uchar luminance);
void setColor(cv::Mat* const mat, const double& red, const double& green, const double& blue);
void setColor(cv::Mat* const mat, const double& luminance);
bool setColor(cv::Mat* const dst, const cv::Mat& src);
void setPoint(cv::Point* const p, const int _x, const int _y);
// print method
void printMatPropaty(const cv::Mat* const m1);
void printOpenCVVersion(void);
// init method
void initPoint(cv::Point* const p, const int size);
// check method
bool checkMatSize(const cv::Mat& m1, const cv::Mat& m2);
bool checkMatSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3);
bool checkMatSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4);
bool checkMatSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5);
bool checkMatSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6);
bool checkContinuous(const cv::Mat& m1, const cv::Mat& m2);
bool checkContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3);
bool checkContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4);
bool checkContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5);
bool checkContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6);
// other method
void mat2char(unsigned char c[], const cv::Mat *m);
uchar convertNumber16sTo8u(const short src, const cv::Size* charRange, const cv::Size* intRange);
void test_convertNumber16sTo8u(void);
void convertMatDepth16sTo8u(cv::Mat* const dst8u, const cv::Mat* const src16s);
void test_convertMatDepth16sTo8u(void);
void imshow16s(const char* const windowName, const cv::Mat* const mat16s);
void test_imwrite(void);
void videoCapture_test(void);
void test_sizeArea(void);
bool calcMultiplyEachElement(cv::Mat* om, const cv::Mat& im1, const cv::Mat& im2);
bool calcDivideEachElement(cv::Mat* om, const cv::Mat& im1, const cv::Mat& im2);
bool compareMat(const cv::Mat& m1, const cv::Mat& m2, const double& thresh);
void test_compareMat(void);
bool getDiffNumOfMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2);
bool getAvgOfDiffMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2);
void test_getAvgOfDiffMat(void);
bool getRateOfDiffMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2);
void test_sizeCompare(void);
void doAnyMethodForAllPixelOfMat(cv::Mat* const m1);
bool showData(const cv::Mat& data);
bool mulElmByElm(cv::Mat* const dst, const cv::Mat& src1, const cv::Mat& src2);
bool divElmByElm(cv::Mat* const dst, const cv::Mat& src1, const cv::Mat& src2);

#endif
