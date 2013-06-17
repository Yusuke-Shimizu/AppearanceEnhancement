//
//  LinearizerOfProjector.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/07.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include "LinearizerOfProjector.h"
#include "ProCam.h"
#include "common.h"

///////////////////////////////  constructor ///////////////////////////////
LinearizerOfProjector::LinearizerOfProjector(void){
    initColorMixingMatrix();
}

LinearizerOfProjector::LinearizerOfProjector(const ProCam* const procam){
    LinearizerOfProjector();
    setProCam(procam);
}

///////////////////////////////  set method ///////////////////////////////
// m_ColorMixingMatrix（色変換行列）の設定
// input / colMix   : 設定する行列
// return           : 成功したかどうか
bool LinearizerOfProjector::setColorMixingMatrix(const cv::Mat* const colMix){
    m_ColorMixingMatrix = *colMix;
    return true;
}

// プロカムのセッティング
bool LinearizerOfProjector::setProCam(const ProCam* const procam){
    *m_procam = *procam;
    return true;
}

///////////////////////////////  get method ///////////////////////////////
// m_ColorMixingMatrix（色変換行列）の取得
// output / colMix  : 色変換行列を入れる行列
// return           : 成功したかどうか
bool LinearizerOfProjector::getColorMixingMatrix(cv::Mat* const colMix){
    *colMix = m_ColorMixingMatrix;
    return true;
}

///////////////////////////////  init method ///////////////////////////////
// init method
bool LinearizerOfProjector::initColorMixingMatrix(void){
    // 3x3で対角成分が１、残りが０の行列を生成
    cv::Mat initMat = cv::Mat::eye(3, 3, CV_64FC1);
    if( !setColorMixingMatrix(&initMat) ) return false;
    _print(initMat);
    return true;
}

///////////////////////////////  other method ///////////////////////////////
// プロジェクタの線形化を行うメソッド
// input / responseOfProjector  : 線形化のルックアップテーブルを入れる配列
// return   : 成功したかどうか
bool LinearizerOfProjector::linearlize(double* const responseOfProjector){
    // 色変換行列の生成
    if( !calcColorMixingMatrix() ) return false;
    
    return true;
}

// 色変換行列の算出
bool LinearizerOfProjector::calcColorMixingMatrix(void){
    
    return true;
}
