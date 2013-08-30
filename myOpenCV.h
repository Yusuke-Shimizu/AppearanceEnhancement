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
#define MY_IMSHOW(img) cv::imshow(#img, img);cv::waitKey(30)
#define _print_mat_propaty(mat) _print_name(mat);printMatPropaty(mat)

#ifdef MAC
const char CV_BUTTON_ESC    = 27;
const char CV_BUTTON_UP     = 0;
const char CV_BUTTON_DOWN   = 1;
const char CV_BUTTON_LEFT   = 2;
const char CV_BUTTON_RIGHT  = 3;
const char CV_BUTTON_c      = 99;
const char CV_BUTTON_C      = 67;
const char CV_BUTTON_g      = 103;
const char CV_BUTTON_G      = 71;
const char CV_BUTTON_f      = 102;
const char CV_BUTTON_F      = 70;
const char CV_BUTTON_DELETE = 127;
#endif
#ifdef MAC_PROJECTOR
const int CV_BUTTON_ESC     = 27;
const int CV_BUTTON_UP      = 0;
const int CV_BUTTON_DOWN    = 1;
const int CV_BUTTON_LEFT    = 2;
const int CV_BUTTON_RIGHT   = 3;
const int CV_BUTTON_c       = 99;
const int CV_BUTTON_C       = 67;
const int CV_BUTTON_g       = 103;
const int CV_BUTTON_G       = 71;
#endif
#ifdef LINUX
const char CV_BUTTON_ESC    = 27;
const char CV_BUTTON_UP     = 82;
const char CV_BUTTON_DOWN   = 84;
const char CV_BUTTON_LEFT   = 81;
const char CV_BUTTON_RIGHT  = 83;
const char CV_BUTTON_c      = 99;
const char CV_BUTTON_C      = 67;
const char CV_BUTTON_g      = 103;
const char CV_BUTTON_G      = 71;
const char CV_BUTTON_f      = 102;
const char CV_BUTTON_F      = 70;
const char CV_BUTTON_DELETE = 8;
#endif

// define based Color
const cv::Vec3b CV_VEC3B_RED(0, 0, 255);
const cv::Vec3b CV_VEC3B_GREEN(0, 255, 0);
const cv::Vec3b CV_VEC3B_BLUE(255, 0, 0);
const cv::Vec3b CV_VEC3B_BLACK(0, 0, 0);
const cv::Vec3b CV_VEC3B_WHITE(255, 255, 255);
const cv::Scalar CV_SCALAR_RED(CV_VEC3B_RED);
const cv::Scalar CV_SCALAR_GREEN(CV_VEC3B_GREEN);
const cv::Scalar CV_SCALAR_BLUE(CV_VEC3B_BLUE);
const cv::Scalar CV_SCALAR_BLACK(CV_VEC3B_BLACK);
const cv::Scalar CV_SCALAR_WHITE(CV_VEC3B_WHITE);

typedef cv::Vec<double, 9> Vec9d;
typedef cv::Vec<double, 12> Vec12d;

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
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6, const cv::Mat& m7);
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6, const cv::Mat& m7, const cv::Mat& m8);

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
void setPoint(cv::Point* const _p, const int _x, const int _y);
///////////////////////////////  get method ///////////////////////////////
bool getPoint(int* const _x, int* const _y, const cv::Point& _p);
uchar getPixelNumuc(const cv::Mat& _image, const cv::Point& _point, const ColorName _cName);
double getPixelNumd(const cv::Mat& _image, const cv::Point& _point, const ColorName _cName);
//const cv::Scalar_<double>* getPixelNumd(const cv::Mat& _image, const cv::Point& _point);
const cv::Vec3d* getPixelNumd(const cv::Mat& _image, const cv::Point& _point);
bool getDiagonalImage(cv::Mat* _diagImage, const cv::Mat_<Vec9d>& _vec9dImage);
///////////////////////////////  print method ///////////////////////////////
void printMatPropaty(const cv::Mat& m1);
void printOpenCVVersion(void);
void printVideoPropaty(cv::VideoCapture& _video);
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
bool convertRGBtoBGR(cv::Vec3b* const _bgr, const cv::Vec3b& _rgb);
bool convertVecToMat(cv::Mat* const _dst, const Vec9d& _src);
bool convertMatToVec(Vec9d* const _dst, const cv::Mat_<double>& _src);
///////////////////////////////  round method ///////////////////////////////
bool roundXtoY(double* const _num, const double& X, const double& Y);
bool round0to1(double* const _num);
bool roundXtoYForMat(cv::Mat* const _mat, const cv::Mat& _X, const cv::Mat& _Y);
bool roundXtoYForMat(cv::Mat* const _mat, const double& _X, const double& _Y);
bool round0to1ForMat(cv::Mat* const _mat);
void test_round0to1ForMat(void);
///////////////////////////////  other method ///////////////////////////////
void imshow16s(const char* const windowName, const cv::Mat& mat16s);
void videoCapture_test(void);
void test_sizeArea(void);
bool calcMultiplyEachElement(cv::Mat* om, const cv::Mat& im1, const cv::Mat& im2);
bool calcDivideEachElement(cv::Mat* om, const cv::Mat& im1, const cv::Mat& im2);
bool getDiffNumOfMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2);
bool compareMat(const cv::Mat& m1, const cv::Mat& m2, const double& thresh);
void test_compareMat(void);
bool getAvgOfDiffMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2);
void test_getAvgOfDiffMat(void);
bool getAvgOfDiffMat2(cv::Vec3d* const diff, const cv::Mat& m1, const cv::Mat& m2);
void test_getAvgOfDiffMat2(void);
bool getRateOfDiffMat(double* const diff, const cv::Mat& m1, const cv::Mat& m2);
void test_sizeCompare(void);
void doAnyMethodForAllPixelOfMat(cv::Mat* const m1);
bool showData(const cv::Mat& data);
bool mulElmByElm(cv::Mat* const dst, const cv::Mat& src1, const cv::Mat& src2);
bool divElmByElm(cv::Mat* const dst, const cv::Mat& src1, const cv::Mat& src2);
bool normalizeByAnyColorChannel(cv::Mat* const image, const ColorName _cName);
bool calcAverageOfImage(cv::Vec3b* const _aveColor, const cv::Mat& image);
bool insertMatForDeepDepthMat(cv::Mat* const _deepDepthMat, const cv::Mat& _oneLayerMat, const int _depth, const int _maxDepth = 256);
void checkButton(void);
///////////////////////////////  inline method ///////////////////////////////
inline void MY_WAIT_KEY(void){
    std::cout << "push any key" << std::endl;
    cv::waitKey(-1);
}
inline void MY_WAIT_KEY(const int key){
    while(true){
        std::cout << "push " << key << " key" << std::endl;
        char pushKey = cv::waitKey(0);
        std::cout << (int)pushKey << std::endl;
        if (pushKey == key) break;
    }
}

#endif
