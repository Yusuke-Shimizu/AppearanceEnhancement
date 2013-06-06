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
void setColor(const cv::MatIterator_<cv::Vec3b> it, const unsigned char red, const unsigned char green, const unsigned char blue);
void setColor(const cv::MatIterator_<cv::Vec3b> it, const unsigned char luminance);
void setPoint(cv::Point* const p, const int _x, const int _y);
// print method
void printMatPropaty(const cv::Mat* const m1);
void printOpenCVVersion(void);
// init method
void initPoint(cv::Point* const p, const int size);
// other method
void mat2char(unsigned char c[], const cv::Mat *m);
uchar convertNumber16sTo8u(const short src, const cv::Size* charRange, const cv::Size* intRange);
void test_convertNumber16sTo8u(void);
void convertMatDepth16sTo8u(cv::Mat* const dst8u, const cv::Mat* const src16s);
void test_convertMatDepth16sTo8u(void);
void imshow16s(const char* const windowName, const cv::Mat* const mat16s);
void test_imwrite(void);
int areaSize(const cv::Size* const _size);
void videoCapture_test(void);
void test_sizeArea(void);

#endif
