//
//  PhotometricModel.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2014/02/07.
//  Copyright (c) 2014年 志水 友輔. All rights reserved.
//

#include "PhotometricModel.h"

PhotometricModel::PhotometricModel(const cv::Size& _size)
:m_K(_size, CV_64FC3, 0.0), m_F(_size, CV_64FC3, 0.0), m_C(_size, CV_64FC3, 0.0), m_P(_size, CV_64FC3, 0.0), m_CMax(_size, CV_64FC3, 0.0), m_CMin(_size, CV_64FC3, 0.0), m_PMax(_size, CV_64FC3, 0.0), m_PMin(_size, CV_64FC3, 0.0)
{
    
}

PhotometricModel::PhotometricModel(const cv::Mat& _K, const cv::Mat& _F, const cv::Mat& _C, const cv::Mat& _P, const cv::Mat& _CMax, const cv::Mat& _CMin, const cv::Mat& _PMax, const cv::Mat& _PMin){
    m_K = _K;
    m_F = _F;
    m_C = _C;
    m_P = _P;
    m_CMax = _CMax;
    m_CMin = _CMin;
    m_PMax = _PMax;
    m_PMin = _PMin;
}

bool getVector(PhotometricModelVector* _pmv){
    
    return true;
}
