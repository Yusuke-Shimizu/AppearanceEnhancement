//
//  common.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/03/26.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#ifndef cameraBase03_common_h
#define cameraBase03_common_h

// プリント系
#define _print(var) std::cout<<#var<<" = "<<(var)<<" ("<<&(var)<<") ("<<__FUNCTION__<<")"<<std::endl
#define _print2(var1, var2) std::cout<<"["<<#var1<<", "<<#var2<<"]"<<" = ["<<(var1)<<", "<<(var2)<<"] ("<<&(var1)<<") ("<<__FUNCTION__<<")"<<std::endl
#define _print3(var1, var2, var3) std::cout<<"["<<#var1<<", "<<#var2<<", "<<#var3<<"]"<<" = ["<<(var1)<<", "<<(var2)<<", "<<(var3)<<"] ("<<&(var1)<<") ("<<__FUNCTION__<<")"<<std::endl
#define _print4(var1, var2, var3, var4) std::cout<<"["<<#var1<<", "<<#var2<<", "<<#var3<<", "<<#var4<<"]"<<" = ["<<(var1)<<", "<<(var2)<<", "<<(var3)<<", "<<(var4)<<"] ("<<&(var1)<<") ("<<__FUNCTION__<<")"<<std::endl
// name print
#define _print_name(var) std::cout<<#var<<" ("<<&(var)<<") ("<<__FUNCTION__<<")"
#define _print_name_ln(var) _print_name(var)<<std::endl
// error print
#define ERROR_PRINT(var) std::cerr<<"error is "<<#var<<" = "<<(var)<<" ("<<&(var)<<") ("<<__FUNCTION__<<")"<<std::endl
#define ERROR_PRINT2(var1, var2) std::cerr<<"error is ["<<#var1<<", "<<#var2<<"]"<<" = ["<<(var1)<<", "<<(var2)<<"] ("<<&(var1)<<") ("<<__FUNCTION__<<")"<<std::endl
#define ERROR_PRINT3(var1, var2, var3) std::cerr<<"error is ["<<#var1<<", "<<#var2<<", "<<#var3<<"]"<<" = ["<<(var1)<<", "<<(var2)<<", "<<(var3)<<"] ("<<&(var1)<<") ("<<__FUNCTION__<<")"<<std::endl
#define ERROR_PRINT4(var1, var2, var3, var4) std::cerr<<"error is ["<<#var1<<", "<<#var2<<", "<<#var3<<", "<<#var4<<"]"<<" = ["<<(var1)<<", "<<(var2)<<", "<<(var3)<<", "<<(var4)<<"] ("<<&(var1)<<") ("<<__FUNCTION__<<")"<<std::endl
// bar
#define _print_bar std::cout << "----------------------------" << std::endl
#define _print_bar2 std::cout << "--------------------------------------------------------" << std::endl
// create excel format
#define _print_excel2(var1, var2) std::cout<<(var1)<<"\t"<<(var2)<<std::endl
#define _print_excel3(var1, var2, var3) std::cout<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<std::endl
#define _print_excel4(var1, var2, var3, var4) std::cout<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<std::endl
#define _print_excel5(var1, var2, var3, var4, var5) std::cout<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<"\t"<<(var5)<<std::endl
#define _print_excel6(var1, var2, var3, var4, var5, var6) std::cout<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<"\t"<<(var5)<<"\t"<<(var6)<<std::endl
#define _print_excel7(var1, var2, var3, var4, var5, var6, var7) std::cout<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<"\t"<<(var5)<<"\t"<<(var6)<<"\t"<<(var7)<<std::endl
#define _print_excel8(var1, var2, var3, var4, var5, var6, var7, var8) std::cout<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<"\t"<<(var5)<<"\t"<<(var6)<<"\t"<<(var7)<<"\t"<<(var8)<<std::endl
#define _print_excel9(var1, var2, var3, var4, var5, var6, var7, var8, var9) std::cout<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<"\t"<<(var5)<<"\t"<<(var6)<<"\t"<<(var7)<<"\t"<<(var8)<<"\t"<<(var9)<<std::endl
// output
#define _print_gnuplot2(output, var1, var2) output<<(var1)<<"\t"<<(var2)<<std::endl
#define _print_gnuplot3(output, var1, var2, var3) output<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<std::endl
#define _print_gnuplot4(output, var1, var2, var3, var4) output<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<std::endl
#define _print_gnuplot5(output, var1, var2, var3, var4, var5) output<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<"\t"<<(var5)<<std::endl
#define _print_gnuplot6(output, var1, var2, var3, var4, var5, var6) output<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<"\t"<<(var5)<<"\t"<<(var6)<<std::endl
#define _print_gnuplot7(output, var1, var2, var3, var4, var5, var6, var7) output<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<"\t"<<(var5)<<"\t"<<(var6)<<"\t"<<(var7)<<std::endl
#define _print_gnuplot8(output, var1, var2, var3, var4, var5, var6, var7, var8) output<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<"\t"<<(var5)<<"\t"<<(var6)<<"\t"<<(var7)<<"\t"<<(var8)<<std::endl
#define _print_gnuplot9(output, var1, var2, var3, var4, var5, var6, var7, var8, var9) output<<(var1)<<"\t"<<(var2)<<"\t"<<(var3)<<"\t"<<(var4)<<"\t"<<(var5)<<"\t"<<(var6)<<"\t"<<(var7)<<"\t"<<(var8)<<"\t"<<(var9)<<std::endl


// 定義
//#define PRJ_SIZE_WIDTH 10
//#define PRJ_SIZE_HEIGHT 7
#define PRJ_SIZE_WIDTH 512
#define PRJ_SIZE_HEIGHT 512
//#define PRJ_SIZE_WIDTH 800
//#define PRJ_SIZE_HEIGHT 700
//#define PRJ_SIZE_WIDTH 768
//#define PRJ_SIZE_HEIGHT 768
//#define PRJ_SIZE_WIDTH 1024
//#define PRJ_SIZE_HEIGHT 768
//#define PRJ_SIZE_WIDTH 1024
//#define PRJ_SIZE_HEIGHT 1024

// 符号有無Nビットの範囲
#define UCHAR_MIN 0
#define DEPTH_U8BIT_MIN 0
#define DEPTH_U8BIT_MAX 255
#define DEPTH_S16BIT_MIN -32768
#define DEPTH_S16BIT_MAX 32767

// 型による色の定義
#define BOOL_BLACK 0
#define BOOL_WHITE 1
#define CHAR_BLACK UCHAR_MIN
#define CHAR_WHITE UCHAR_MAX

// その他
#define SLEEP_TIME 300    // スリープ時間(ms)
#define MAX_WINDOW_SIZE 10000   // ウィンドウの最大値
#define BINARY_THRESH 0     // ネガポジの二値化を行うときの閾値
#define CAPTURE_NUM 10
#define NOISE_RANGE 0.01

// window name
#define W_NAME_GEO_CAMERA "geometric calibration camera"
#define W_NAME_GEO_PROJECTOR "geometric calibration projector"

#endif
