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
#include "common.h"

#define OPENCV_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define OPENCV_VERSION_CODE OPENCV_VERSION(CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION)

// 符号有無Nビットの範囲
#define UCHAR_MIN 0
#define DEPTH_U8BIT_MIN 0
#define DEPTH_U8BIT_MAX 255
#define DEPTH_S16BIT_MIN -32768
#define DEPTH_S16BIT_MAX 32767

#define _print_vector(vec) std::cout<<#vec<<"[b, g, r] = "<<(vec)<<" ("<<&(vec)<<") ("<<__FUNCTION__<<")"<<std::endl
#define MY_IMSHOW(img) cv::imshow(#img, img)

typedef cv::Vec<double, 9> Vec9d;

enum ColorName{CV_RED = 2, CV_GREEN = 1, CV_BLUE = 0};

///////////////////////////////  check method ///////////////////////////////
bool isEqualSize(const cv::Mat& m1, const cv::Mat& m2);
bool isEqualSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3);
bool isEqualSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4);
bool isEqualSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5);
bool isEqualSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6);
bool isDifferentSize(const cv::Mat& m1, const cv::Mat& m2);
bool isDifferentSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3);
bool isDifferentSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4);
bool isDifferentSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5);
bool isDifferentSize(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6);

bool isContinuous(const cv::Mat& m1, const cv::Mat& m2);
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3);
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4);
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5);
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6);

bool isEqualDepth(const cv::Mat& m1, const cv::Mat& m2);

bool isEqualChannel(const cv::Mat& m1, const cv::Mat& m2);

bool isEqualType(const cv::Mat& m1, const cv::Mat& m2);
bool isEqualType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3);
bool isEqualType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4);
bool isEqualType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5);
bool isEqualType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6);

bool isEqualSizeAndType(const cv::Mat& m1, const cv::Mat& m2);
bool isEqualSizeAndType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3);
bool isEqualSizeAndType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4);
bool isEqualSizeAndType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5);
bool isEqualSizeAndType(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6);

bool isPosiNum(const cv::Mat& m1);
bool isPosiNum(const cv::Mat& m1, const cv::Mat& m2);
bool isPosiNum(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3);
bool isPosiNum(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4);

bool isEqualPoint(const cv::Point* const p1, const cv::Point* const p2, const int length);
///////////////////////////////  set method ///////////////////////////////
void setColor(const cv::MatIterator_<cv::Vec3b> it, const uchar red, const uchar green, const uchar blue);
void setColor(const cv::MatIterator_<cv::Vec3b> it, const uchar luminance);
void setColor(cv::Mat* const _mat, const double& _red, const double& _green, const double& _blue);
void setColor(cv::Mat* const _mat, const double& _luminance);
bool setColor(cv::Mat* const _dst, const cv::Mat& _src);
bool setVecColor(cv::Vec3b* const _color, const char _luminance, const ColorName _cName);
//bool setFlatImage(cv::Mat* const _image, );
void setPoint(cv::Point* const _p, const int _x, const int _y);
///////////////////////////////  get method ///////////////////////////////
bool getPoint(int* const _x, int* const _y, const cv::Point& _p);
uchar getPixelNumuc(const cv::Mat& _image, const cv::Point& _point, const ColorName _cName);
double getPixelNumd(const cv::Mat& _image, const cv::Point& _point, const ColorName _cName);
//const cv::Scalar_<double>* getPixelNumd(const cv::Mat& _image, const cv::Point& _point);
const cv::Vec3d* getPixelNumd(const cv::Mat& _image, const cv::Point& _point);
///////////////////////////////  print method ///////////////////////////////
void printMatPropaty(const cv::Mat& m1);
void printOpenCVVersion(void);
///////////////////////////////  init method ///////////////////////////////
void initPoint(cv::Point* const p, const int size);
void initMat(cv::Mat* const _aMat, const int _size);
void initVec3b(cv::Vec3b* const _vector);
///////////////////////////////  convert method ///////////////////////////////
void mat2char(unsigned char c[], const cv::Mat *m);
uchar convertNumber16sTo8u(const short src, const cv::Size* charRange, const cv::Size* intRange);
void test_convertNumber16sTo8u(void);
void convertMatDepth16sTo8u(cv::Mat* const dst8u, const cv::Mat* const src16s);
void test_convertMatDepth16sTo8u(void);
bool convertBGRtoRGB(cv::Vec3b* const _rgb, const cv::Vec3b& _bgr);
bool convertVecToMat(cv::Mat_<double>* const _dst, const Vec9d& _src);
bool convertMatToVec(Vec9d* const _dst, const cv::Mat_<double>& _src);
///////////////////////////////  round method ///////////////////////////////
bool roundXtoY(double* const _num, const double& X, const double& Y);
bool round0to1(double* const _num);
bool roundXtoYForMat(cv::Mat* const _mat, const cv::Mat& _X, const cv::Mat& _Y);
bool roundXtoYForMat(cv::Mat* const _mat, const double& _X, const double& _Y);
bool round0to1ForMat(cv::Mat* const _mat);
void test_round0to1ForMat(void);
///////////////////////////////  other method ///////////////////////////////
void imshow16s(const char* const windowName, const cv::Mat* const mat16s);
void videoCapture_test(void);
void test_sizeArea(void);
bool calcMultiplyEachElement(cv::Mat* om, const cv::Mat& im1, const cv::Mat& im2);
bool calcDivideEachElement(cv::Mat* om, const cv::Mat& im1, const cv::Mat& im2);
bool getDiffNumOfMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2);
bool compareMat(const cv::Mat& m1, const cv::Mat& m2, const double& thresh);
void test_compareMat(void);
bool getAvgOfDiffMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2);
void test_getAvgOfDiffMat(void);
bool getRateOfDiffMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2);
void test_sizeCompare(void);
void doAnyMethodForAllPixelOfMat(cv::Mat* const m1);
bool showData(const cv::Mat& data);
bool mulElmByElm(cv::Mat* const dst, const cv::Mat& src1, const cv::Mat& src2);
bool divElmByElm(cv::Mat* const dst, const cv::Mat& src1, const cv::Mat& src2);
bool normalizeByAnyColorChannel(cv::Mat* const image, const ColorName _cName);
bool calcAverageOfImage(cv::Vec3b* const _aveColor, const cv::Mat& image);
///////////////////////////////  inline method ///////////////////////////////
inline void MY_WAIT_KEY(void){
    std::cout << "push any key" << std::endl;
    cv::waitKey(-1);
}

#endif
