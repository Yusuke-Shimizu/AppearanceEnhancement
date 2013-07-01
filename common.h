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
#define _print_name(var) std::cout<<#var<<" ("<<&(var)<<") ("<<__FUNCTION__<<")"<<std::endl
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
// output mat
#define _print_gnuplot_mat2(output, index, mat1, mat2) _print_gnuplot4(output, index, (mat1).at<double>(0, 0), (mat2).at<double>(0, 0))
#define _print_gnuplot_mat3(output, index, mat1, mat2, mat3) _print_gnuplot4(output, index, (mat1).at<double>(0, 0), (mat2).at<double>(0, 0), (mat3).at<double>(0, 0))
#define _print_gnuplot_mat4(output, index, mat1, mat2, mat3, mat4) _print_gnuplot5(output, index, (mat1).at<double>(0, 0), (mat2).at<double>(0, 0), (mat3).at<double>(0, 0), (mat4).at<double>(0, 0))
#define _print_gnuplot_mat5(output, index, mat1, mat2, mat3, mat4, mat5) _print_gnuplot6(output, index, (mat1).at<double>(0, 0), (mat2).at<double>(0, 0), (mat3).at<double>(0, 0), (mat4).at<double>(0, 0), (mat5).at<double>(0, 0))
#define _print_gnuplot_mat6(output, index, mat1, mat2, mat3, mat4, mat5, mat6) _print_gnuplot7(output, index, (mat1).at<double>(0, 0), (mat2).at<double>(0, 0), (mat3).at<double>(0, 0), (mat4).at<double>(0, 0), (mat5).at<double>(0, 0), (mat6).at<double>(0, 0))
#define _print_gnuplot_mat7(output, index, mat1, mat2, mat3, mat4, mat5, mat6, mat7) _print_gnuplot8(output, index, (mat1).at<double>(0, 0), (mat2).at<double>(0, 0), (mat3).at<double>(0, 0), (mat4).at<double>(0, 0), (mat5).at<double>(0, 0), (mat6).at<double>(0, 0), (mat7).at<double>(0, 0))
#define _print_gnuplot_mat8(output, index, mat1, mat2, mat3, mat4, mat5, mat6, mat7, mat8) _print_gnuplot9(output, index, (mat1).at<double>(0, 0), (mat2).at<double>(0, 0), (mat3).at<double>(0, 0), (mat4).at<double>(0, 0), (mat5).at<double>(0, 0), (mat6).at<double>(0, 0), (mat7).at<double>(0, 0), (mat8).at<double>(0, 0))

// 定義
#define VGA_WIDTH 640
#define VGA_HEIGHT 480

//#define PRJ_SIZE_WIDTH 10
//#define PRJ_SIZE_HEIGHT 7
//#define PRJ_SIZE_WIDTH 512
//#define PRJ_SIZE_HEIGHT 512
//#define PRJ_SIZE_WIDTH 800
//#define PRJ_SIZE_HEIGHT 700
//#define PRJ_SIZE_WIDTH 768
//#define PRJ_SIZE_HEIGHT 768
//#define PRJ_SIZE_WIDTH 1024
//#define PRJ_SIZE_HEIGHT 768
//#define PRJ_SIZE_WIDTH 1024
//#define PRJ_SIZE_HEIGHT 1024
#define PRJ_SIZE_WIDTH VGA_WIDTH
#define PRJ_SIZE_HEIGHT VGA_HEIGHT

#define NOISE_RANGE 0.01
//#define NOISE_RANGE 0.05

#endif
