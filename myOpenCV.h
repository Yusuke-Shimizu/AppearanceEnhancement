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
const uchar UCHAR_MIN = 0;
const int DEPTH_U8BIT_MIN = 0;
const int DEPTH_U8BIT_MAX = 255;
const float DEPTH_S16BIT_MIN = -32768;
const float DEPTH_S16BIT_MAX = 32767;

#define _print_vector(vec) std::cout<<#vec<<"[b, g, r] = "<<(vec)<<" ("<<&(vec)<<") ("<<__FUNCTION__<<")"<<std::endl
#define _print_mat_propaty(mat) _print_name(mat);printMatPropaty(mat)

// show
#define MY_IMSHOW(img) cv::imshow(#img, img);cv::waitKey(30)
#define MY_IMSHOW_(img) cv::imshow(#img, img)
#define MY_IMSHOW2(img1, img2) MY_IMSHOW_(img1);MY_IMSHOW(img2)
#define MY_IMSHOW2_(img1, img2) MY_IMSHOW_(img1);MY_IMSHOW_(img2)
#define MY_IMSHOW3(img1, img2, img3) MY_IMSHOW2_(img1, img2);MY_IMSHOW(img3)
#define MY_IMSHOW3_(img1, img2, img3) MY_IMSHOW2_(img1, img2);MY_IMSHOW_(img3)
#define MY_IMSHOW4(img1, img2, img3, img4) MY_IMSHOW3_(img1, img2, img3);MY_IMSHOW(img4)
#define MY_IMSHOW4_(img1, img2, img3, img4) MY_IMSHOW3_(img1, img2, img3);MY_IMSHOW_(img4)
#define MY_IMSHOW5(img1, img2, img3, img4, img5) MY_IMSHOW4_(img1, img2, img3, img4);MY_IMSHOW(img5)
#define MY_IMSHOW5_(img1, img2, img3, img4, img5) MY_IMSHOW4_(img1, img2, img3, img4);MY_IMSHOW_(img5)
#define MY_IMSHOW6(img1, img2, img3, img4, img5, img6) MY_IMSHOW5_(img1, img2, img3, img4, img5);MY_IMSHOW(img6)
#define MY_IMSHOW6_(img1, img2, img3, img4, img5, img6) MY_IMSHOW5_(img1, img2, img3, img4, img5);MY_IMSHOW_(img6)
#define MY_IMSHOW7(img1, img2, img3, img4, img5, img6, img7) MY_IMSHOW6_(img1, img2, img3, img4, img5, img6);MY_IMSHOW(img7)
#define MY_IMSHOW7_(img1, img2, img3, img4, img5, img6, img7) MY_IMSHOW6_(img1, img2, img3, img4, img5, img6);MY_IMSHOW_(img7)
#define MY_IMSHOW8(img1, img2, img3, img4, img5, img6, img7, img8) MY_IMSHOW7_(img1, img2, img3, img4, img5, img6, img7);MY_IMSHOW(img8)
#define MY_IMSHOW8_(img1, img2, img3, img4, img5, img6, img7, img8) MY_IMSHOW7_(img1, img2, img3, img4, img5, img6, img7);MY_IMSHOW_(img8)
#define MY_IMSHOW9(img1, img2, img3, img4, img5, img6, img7, img8, img9) MY_IMSHOW8_(img1, img2, img3, img4, img5, img6, img7, img8);MY_IMSHOW(img9)
#define MY_IMSHOW9_(img1, img2, img3, img4, img5, img6, img7, img8, img9) MY_IMSHOW8_(img1, img2, img3, img4, img5, img6, img7, img8);MY_IMSHOW_(img9)
#define MY_IMSHOW10(img1, img2, img3, img4, img5, img6, img7, img8, img9, img10) MY_IMSHOW9_(img1, img2, img3, img4, img5, img6, img7, img8, img9);MY_IMSHOW(img10)
#define MY_IMSHOW10_(img1, img2, img3, img4, img5, img6, img7, img8, img9, img10) MY_IMSHOW9_(img1, img2, img3, img4, img5, img6, img7, img8, img9);MY_IMSHOW_(img10)

#define MY_IMWRITE(path, num, oss, tmp) oss.str("");oss.clear(std::stringstream::goodbit);oss<<path<<#tmp<<num<<".png";cv::imwrite(oss.str().c_str(), tmp)
#define MY_IMWRITE2(path, num, oss, tmp1, tmp2) MY_IMWRITE(path, num, oss, tmp1);MY_IMWRITE(path, num, oss, tmp2)
#define MY_IMWRITE3(path, num, oss, tmp1, tmp2, tmp3) MY_IMWRITE2(path, num, oss, tmp1, tmp2);MY_IMWRITE(path, num, oss, tmp3)
#define MY_IMWRITE4(path, num, oss, tmp1, tmp2, tmp3, tmp4) MY_IMWRITE3(path, num, oss, tmp1, tmp2, tmp3);MY_IMWRITE(path, num, oss, tmp4)
#define MY_IMWRITE5(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5) MY_IMWRITE4(path, num, oss, tmp1, tmp2, tmp3, tmp4);MY_IMWRITE(path, num, oss, tmp5)
#define MY_IMWRITE6(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6) MY_IMWRITE5(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5);MY_IMWRITE(path, num, oss, tmp6)
#define MY_IMWRITE7(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7) MY_IMWRITE6(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6);MY_IMWRITE(path, num, oss, tmp7)
#define MY_IMWRITE8(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8) MY_IMWRITE7(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7);MY_IMWRITE(path, num, oss, tmp8)
#define MY_IMWRITE9(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9) MY_IMWRITE8(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8);MY_IMWRITE(path, num, oss, tmp9)
#define MY_IMWRITE10(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10) MY_IMWRITE9(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9);MY_IMWRITE(path, num, oss, tmp10)

#define MY_IMWRITE_D(path, num, oss, tmp) oss.str("");oss.clear(std::stringstream::goodbit);oss<<path<<#tmp<<num<<".png";tmp.convertTo(tmp,CV_8UC3, 255);cv::imwrite(oss.str().c_str(), tmp)
#define MY_IMWRITE_D2(path, num, oss, tmp1, tmp2) MY_IMWRITE_D(path, num, oss, tmp1);MY_IMWRITE_D(path, num, oss, tmp2)
#define MY_IMWRITE_D3(path, num, oss, tmp1, tmp2, tmp3) MY_IMWRITE_D2(path, num, oss, tmp1, tmp2);MY_IMWRITE_D(path, num, oss, tmp3)
#define MY_IMWRITE_D4(path, num, oss, tmp1, tmp2, tmp3, tmp4) MY_IMWRITE_D3(path, num, oss, tmp1, tmp2, tmp3);MY_IMWRITE_D(path, num, oss, tmp4)
#define MY_IMWRITE_D5(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5) MY_IMWRITE_D4(path, num, oss, tmp1, tmp2, tmp3, tmp4);MY_IMWRITE_D(path, num, oss, tmp5)
#define MY_IMWRITE_D6(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6) MY_IMWRITE_D5(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5);MY_IMWRITE_D(path, num, oss, tmp6)
#define MY_IMWRITE_D7(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7) MY_IMWRITE_D6(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6);MY_IMWRITE_D(path, num, oss, tmp7)
#define MY_IMWRITE_D8(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8) MY_IMWRITE_D7(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7);MY_IMWRITE_D(path, num, oss, tmp8)
#define MY_IMWRITE_D9(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9) MY_IMWRITE_D8(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8);MY_IMWRITE_D(path, num, oss, tmp9)
#define MY_IMWRITE_D10(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10) MY_IMWRITE_D9(path, num, oss, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9);MY_IMWRITE_D(path, num, oss, tmp10)

// print color
#define _print_gnuplot_color(output, var) output<<(var)[0]<<"\t"<<(var)[1]<<"\t"<<(var)[2]<<"\t"
#define _print_gnuplot_color_l(output, var) _print_gnuplot_color(output, var);output<<std::endl
#define _print_gnuplot_color2(output, var1, var2) _print_gnuplot_color(output,var1);_print_gnuplot_color(output,var2)
#define _print_gnuplot_color2_l(output, var1, var2) _print_gnuplot_color2(output,var1, var2);output<<std::endl
#define _print_gnuplot_color3(output, var1, var2, var3) _print_gnuplot_color2(output, var1, var2);_print_gnuplot_color(output, var3)
#define _print_gnuplot_color3_l(output, var1, var2, var3) _print_gnuplot_color3(output, var1, var2, var3);output<<std::endl
#define _print_gnuplot_color4(output, var1, var2, var3, var4) _print_gnuplot_color3(output, var1, var2, var3);_print_gnuplot_color(output, var4)
#define _print_gnuplot_color4_l(output, var1, var2, var3, var4) _print_gnuplot_color4(output, var1, var2, var3, var4);output<<std::endl
#define _print_gnuplot_color5(output, var1, var2, var3, var4, var5) _print_gnuplot_color4(output, var1, var2, var3, var4);_print_gnuplot_color(output, var5)
#define _print_gnuplot_color5_l(output, var1, var2, var3, var4, var5) _print_gnuplot_color5(output, var1, var2, var3, var4, var5);output<<std::endl
#define _print_gnuplot_color6(output, var1, var2, var3, var4, var5, var6) _print_gnuplot_color5(output, var1, var2, var3, var4, var5);_print_gnuplot_color(output, var6)
#define _print_gnuplot_color6_l(output, var1, var2, var3, var4, var5, var6) _print_gnuplot_color6(output, var1, var2, var3, var4, var5, var6);output<<std::endl
#define _print_gnuplot_color7(output, var1, var2, var3, var4, var5, var6, var7) _print_gnuplot_color6(output, var1, var2, var3, var4, var5, var6);_print_gnuplot_color(output, var7)
#define _print_gnuplot_color7_l(output, var1, var2, var3, var4, var5, var6, var7) _print_gnuplot_color7(output, var1, var2, var3, var4, var5, var6, var7);output<<std::endl
#define _print_gnuplot_color8(output, var1, var2, var3, var4, var5, var6, var7, var8) _print_gnuplot_color7(output, var1, var2, var3, var4, var5, var6, var7);_print_gnuplot_color(output, var8)
#define _print_gnuplot_color8_l(output, var1, var2, var3, var4, var5, var6, var7, var8) _print_gnuplot_color8(output, var1, var2, var3, var4, var5, var6, var7, var8);output<<std::endl
#define _print_gnuplot_color9(output, var1, var2, var3, var4, var5, var6, var7, var8, var9) _print_gnuplot_color8(output, var1, var2, var3, var4, var5, var6, var7, var8);_print_gnuplot_color(output, var9)
#define _print_gnuplot_color9_l(output, var1, var2, var3, var4, var5, var6, var7, var8, var9) _print_gnuplot_color9(output, var1, var2, var3, var4, var5, var6, var7, var8, var9);output<<std::endl


#ifdef MAC
const int CV_BUTTON_SMALL_TO_CAPITAL = -32;
const int CV_BUTTON_ESC    = 27;
const int CV_BUTTON_UP     = 63232;
const int CV_BUTTON_DOWN   = CV_BUTTON_UP + 1;
const int CV_BUTTON_LEFT   = CV_BUTTON_UP + 2;
const int CV_BUTTON_RIGHT  = CV_BUTTON_UP + 3;
const int CV_BUTTON_DELETE = 127;
const int CV_BUTTON_a      = 97;
const int CV_BUTTON_0      = 48;
#endif
#ifdef LINUX
const int CV_BUTTON_ESC     = 1048603;
const int CV_BUTTON_LEFT    = 1113937;
const int CV_BUTTON_UP      = CV_BUTTON_LEFT + 1;
const int CV_BUTTON_RIGHT   = CV_BUTTON_LEFT + 2;
const int CV_BUTTON_DOWN    = CV_BUTTON_LEFT + 3;
const int CV_BUTTON_DELETE  = 1113864;
const int CV_BUTTON_SMALL_TO_CAPITAL = 65504;
const int CV_BUTTON_a       = 1048673;
const int CV_BUTTON_0       = 1048624;
#endif
const int CV_BUTTON_b       = CV_BUTTON_a + 1;
const int CV_BUTTON_B       = CV_BUTTON_b + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_c       = CV_BUTTON_a + 2;  // 1048675
const int CV_BUTTON_C       = CV_BUTTON_c + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_d       = CV_BUTTON_a + 3;
const int CV_BUTTON_e       = CV_BUTTON_a + 4;  // 1048677
const int CV_BUTTON_E       = CV_BUTTON_e + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_f       = CV_BUTTON_a + 5;  // 1048678;
const int CV_BUTTON_F       = CV_BUTTON_f + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_g       = CV_BUTTON_a + 6;  // 1048679;
const int CV_BUTTON_G       = CV_BUTTON_g + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_k       = CV_BUTTON_a + 10;
const int CV_BUTTON_K       = CV_BUTTON_k + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_l       = CV_BUTTON_a + 11; //1048684;
const int CV_BUTTON_L       = CV_BUTTON_l + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_m       = CV_BUTTON_a + 12; //1048684;
const int CV_BUTTON_M       = CV_BUTTON_m + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_n       = CV_BUTTON_a + 13;
const int CV_BUTTON_N       = CV_BUTTON_n + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_p       = CV_BUTTON_a + 15;
const int CV_BUTTON_P       = CV_BUTTON_p + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_q       = CV_BUTTON_a + 16; //1048690;
const int CV_BUTTON_Q       = CV_BUTTON_q + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_r       = CV_BUTTON_a + 17; //1048690;
const int CV_BUTTON_R       = CV_BUTTON_r + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_s       = CV_BUTTON_a + 18;
const int CV_BUTTON_S       = CV_BUTTON_s + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_t       = CV_BUTTON_a + 19;
const int CV_BUTTON_T       = CV_BUTTON_t + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_v       = CV_BUTTON_a + 21;
const int CV_BUTTON_V       = CV_BUTTON_v + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_w       = CV_BUTTON_a + 22;
const int CV_BUTTON_W       = CV_BUTTON_w + CV_BUTTON_SMALL_TO_CAPITAL;
const int CV_BUTTON_y       = CV_BUTTON_a + 24;
const int CV_BUTTON_Y       = CV_BUTTON_y + CV_BUTTON_SMALL_TO_CAPITAL;

// number button
const int CV_BUTTON_1       = CV_BUTTON_0 + 1;
const int CV_BUTTON_2       = CV_BUTTON_0 + 2;
const int CV_BUTTON_3       = CV_BUTTON_0 + 3;
const int CV_BUTTON_4       = CV_BUTTON_0 + 4;
const int CV_BUTTON_5       = CV_BUTTON_0 + 5;
const int CV_BUTTON_6       = CV_BUTTON_0 + 6;
const int CV_BUTTON_7       = CV_BUTTON_0 + 7;
const int CV_BUTTON_8       = CV_BUTTON_0 + 8;
const int CV_BUTTON_9       = CV_BUTTON_0 + 9;

// define based Color
const cv::Vec3b CV_VEC3B_RED(0, 0, 255);
const cv::Vec3b CV_VEC3B_GREEN(0, 255, 0);
const cv::Vec3b CV_VEC3B_BLUE(255, 0, 0);
const cv::Vec3b CV_VEC3B_YELLOW(0, 255, 255);
const cv::Vec3b CV_VEC3B_CYAN(255, 255, 0);
const cv::Vec3b CV_VEC3B_PURPLE(255, 0, 255);
const cv::Vec3b CV_VEC3B_BLACK(0, 0, 0);
const cv::Vec3b CV_VEC3B_WHITE(255, 255, 255);

const cv::Vec3d CV_VEC3D_RED(CV_VEC3B_RED / 255.0);
const cv::Vec3d CV_VEC3D_GREEN(CV_VEC3B_GREEN / 255.0);
const cv::Vec3d CV_VEC3D_BLUE(CV_VEC3B_BLUE / 255.0);
const cv::Vec3d CV_VEC3D_YELLOW(CV_VEC3B_YELLOW / 255.0);
const cv::Vec3d CV_VEC3D_CYAN(CV_VEC3B_CYAN / 255.0);
const cv::Vec3d CV_VEC3D_PURPLE(CV_VEC3B_PURPLE / 255.0);
const cv::Vec3d CV_VEC3D_BLACK(CV_VEC3B_BLACK);
const cv::Vec3d CV_VEC3D_WHITE(CV_VEC3B_WHITE / 255.0);

const cv::Scalar CV_SCALAR_RED(CV_VEC3B_RED);
const cv::Scalar CV_SCALAR_GREEN(CV_VEC3B_GREEN);
const cv::Scalar CV_SCALAR_BLUE(CV_VEC3B_BLUE);
const cv::Scalar CV_SCALAR_YELLOW(CV_VEC3B_YELLOW);
const cv::Scalar CV_SCALAR_CYAN(CV_VEC3B_CYAN);
const cv::Scalar CV_SCALAR_PURPLE(CV_VEC3B_PURPLE);
const cv::Scalar CV_SCALAR_BLACK(CV_VEC3B_BLACK);
const cv::Scalar CV_SCALAR_WHITE(CV_VEC3B_WHITE);

const cv::Scalar CV_SCALAR_D_RED(CV_VEC3D_RED);
const cv::Scalar CV_SCALAR_D_GREEN(CV_VEC3D_GREEN);
const cv::Scalar CV_SCALAR_D_BLUE(CV_VEC3D_BLUE);
const cv::Scalar CV_SCALAR_D_YELLOW(CV_VEC3D_YELLOW);
const cv::Scalar CV_SCALAR_D_CYAN(CV_VEC3D_CYAN);
const cv::Scalar CV_SCALAR_D_PURPLE(CV_VEC3D_PURPLE);
const cv::Scalar CV_SCALAR_D_BLACK(CV_VEC3D_BLACK);
const cv::Scalar CV_SCALAR_D_WHITE(CV_VEC3D_WHITE);

typedef cv::Vec<double, 9> Vec9d;
typedef cv::Vec<double, 12> Vec12d;

enum ColorName{CV_RED = 2, CV_GREEN = 1, CV_BLUE = 0};

// size
const cv::Size VGA_SIZE(640, 480);
const cv::Size XGA_SIZE(1024, 768);
const cv::Size HDTV_720P_SIZE(1280, 720);
const cv::Size WSXGA_PLUS_SIZE(1680, 1050);

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
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6, const cv::Mat& m7, const cv::Mat& m8, const cv::Mat& m9);
bool isContinuous(const cv::Mat& m1, const cv::Mat& m2, const cv::Mat& m3, const cv::Mat& m4, const cv::Mat& m5, const cv::Mat& m6, const cv::Mat& m7, const cv::Mat& m8, const cv::Mat& m9, const cv::Mat& m10);

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
bool yes_no(void);
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
bool getBinaryExposureImage(cv::Mat* const _dst, const cv::Mat& _src, const int thresh);
bool getOverExposureImage(cv::Mat* const _dst, const cv::Mat& _src);
bool getUnderExposureImage(cv::Mat* const _dst, const cv::Mat& _src);
bool getThresholdColorImage(cv::Mat* const _dst, const cv::Mat& _src, const cv::Vec3b _thresh);
bool getFps(double* const _start, double* const _end, double* const _fps);
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
bool convertVector2Mat(cv::Mat* const _dst, const std::vector<cv::Vec3d>& _src);
bool test_convertVector2Mat(void);
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
bool calcAverageOfImage_d(cv::Vec3d* const _aveColor, const cv::Mat& image);
bool insertMatForDeepDepthMat(cv::Mat* const _deepDepthMat, const cv::Mat& _oneLayerMat, const int _depth, const int _maxDepth = 256);
void checkButton(void);
void meanMat(cv::Mat* const _meanMat, const std::vector<cv::Mat>& _matVec);
void test_meanMat(void);
bool calcMeanStddevOfDiffImage(cv::Scalar* const _mean, cv::Scalar* const _stddev, const cv::Mat& _mat1, const cv::Mat& _mat2);
bool test_calcMeanStddevOfDiffImage(void);
bool margeImage(cv::Mat* const _dst, const cv::Mat& _src1, const cv::Mat& _src2);
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
