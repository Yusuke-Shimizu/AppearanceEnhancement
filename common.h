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
#define _print2(var1, var2) std::cout<<#var1<<" = ["<<(var1)<<","<<(var2)<<"] ("<<&(var1)<<"] ("<<__FUNCTION__<<")"<<std::endl
#define _print3(var1, var2, var3) std::cout<<#var1<<" = ["<<(var1)<<","<<(var2)<<","<<(var3)<<"] ("<<&(var1)<<") ("<<__FUNCTION__<<")"<<std::endl
#define _print4(var1, var2, var3, var4) std::cout<<#var1<<" = ["<<(var1)<<","<<(var2)<<","<<(var3)<<","<<(var4)<<"] ("<<&(var1)<<") ("<<__FUNCTION__<<")"<<std::endl
#define _print_name(var) std::cout<<#var<<" ("<<&(var)<<") ("<<__FUNCTION__<<")"
#define _print_name_ln(var) std::cout<<#var<<" ("<<&(var)<<") ("<<__FUNCTION__<<")"<<std::endl
#define ERROR_PRINT(var) std::cout<<"error is "<<#var<<" = "<<(var)<<" ("<<&(var)<<") ("<<__FUNCTION__<<")"<<std::endl


// 定義
//#define PRJ_SIZE_WIDTH 10
//#define PRJ_SIZE_HEIGHT 7
#define PRJ_SIZE_WIDTH 512
#define PRJ_SIZE_HEIGHT 512
//#define PRJ_SIZE_WIDTH 800
//#define PRJ_SIZE_HEIGHT 700
//#define PRJ_SIZE_WIDTH 1024
//#define PRJ_SIZE_HEIGHT 768

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

#define SLEEP_TIME 500    // スリープ時間(ms)
#define MAX_WINDOW_SIZE 10000   // ウィンドウの最大値
#define BINARY_THRESH 0     // ネガポジの二値化を行うときの閾値

// window name
#define W_NAME_GEO_CAMERA "geometric calibration camera"
#define W_NAME_GEO_PROJECTOR "geometric calibration projector"

enum stripeDirection {Vertical, Horizon};   // {縦，横}

#endif
