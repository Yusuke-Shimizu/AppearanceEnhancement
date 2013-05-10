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

void mat2char(unsigned char c[], const cv::Mat *m);
void setColor(const cv::MatIterator_<cv::Vec3b> it, const unsigned char red, const unsigned char green, const unsigned char blue);
void setColor(const cv::MatIterator_<cv::Vec3b> it, const unsigned char luminance);

#endif
