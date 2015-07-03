//
//  PhotometricModel.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2014/02/07.
//  Copyright (c) 2014年 志水 友輔. All rights reserved.
//

#ifndef __cameraBase03__PhotometricModel__
#define __cameraBase03__PhotometricModel__

#include <iostream>
#include "myOpenCV.h"

class PhotometricModelVector{
public:
    cv::Vec3d m_K;
    cv::Vec3d m_F;
    cv::Vec3d m_C;
    cv::Vec3d m_P;
    cv::Vec3d m_Cmax;
    cv::Vec3d m_Cmin;
    cv::Vec3d m_Pmax;
    cv::Vec3d m_Pmin;
    
    //
    PhotometricModelVector(void);
    
};

class PhotometricModel{
public:
    cv::Mat m_K;
    cv::Mat m_F;
    cv::Mat m_C;
    cv::Mat m_P;
    cv::Mat m_CMax;
    cv::Mat m_CMin;
    cv::Mat m_PMax;
    cv::Mat m_PMin;
    
    //
    PhotometricModel(const cv::Size& _size);
    PhotometricModel(const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _C, const cv::Mat& _P, const cv::Mat& _CMax, const cv::Mat& _CMin, const cv::Mat& _PMax, const cv::Mat& _PMin);
    bool getVector(PhotometricModelVector* _pm);
    
};


#endif /* defined(__cameraBase03__PhotometricModel__) */
