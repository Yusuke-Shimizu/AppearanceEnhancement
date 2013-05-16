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

//const double M_PI = 3.14159265358979;

// set method
void setColor(const cv::MatIterator_<cv::Vec3b> it, const unsigned char red, const unsigned char green, const unsigned char blue);
void setColor(const cv::MatIterator_<cv::Vec3b> it, const unsigned char luminance);
void setPoint(cv::Point* const p, const int _x, const int _y);
// print method
void printMatPropaty(const cv::Mat* const m1);
// init method
void initPoint(cv::Point* const p, const int size);
// other method
void mat2char(unsigned char c[], const cv::Mat *m);
void subMat(cv::Mat *dst, const cv::Mat* const src1, const cv::Mat* const src2);
#endif
