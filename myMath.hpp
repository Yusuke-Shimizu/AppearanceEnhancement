//
//  myMath.h
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/15.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#ifndef __cameraBase03__myMath__
#define __cameraBase03__myMath__

#include <iostream>
#include <cmath>
#include <vector>
#include <list>
#include <numeric>
#include <algorithm>    // std::max, std::min
#include "common.h"

// http://d.hatena.ne.jp/teramonagi/20120627/1340805908から拝借
// 平均値を取得
template <template<class T, class Allocator = std::allocator<T> > class Container>
double mean(Container<double> & x) {
	return std::accumulate(x.begin(), x.end(), 0.0) / x.size();
}

// 分散値を取得
template <template<class T, class Allocator = std::allocator<T> > class Container>
double var(Container<double> & x) {
	double size = x.size();
	double x_mean = mean(x);
	return (std::inner_product(x.begin(), x.end(), x.begin(), 0.0) - x_mean * x_mean * size)/ (size - 1.0);
}

// 標準偏差を取得
template <template<class T, class Allocator = std::allocator<T> > class Container>
double sd(Container<double> & x) {
	return std::sqrt(var(x));
}

// 引数の値を０から１に丸める
// ex) round0to1(5) = 1, round0to1(0.1) = 0.1, round0to1(-0.2) = 0
void test(void);// {
//    std::max(*_num, 0.0);
//    std::min(*_num, 1.0);
//    return true;
//}
// 上のテスト
//void test_round0to1(void){
//    double a = 5, b = 0.1, c = -0.2;
//    _print3(a, b, c);
//    round0to1(&a);
//    round0to1(&b);
//    round0to1(&c);
//    _print3(a, b, c);
//}

#endif /* defined(__cameraBase03__myMath__) */
