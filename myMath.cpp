//
//  myMath.cpp
//  cameraBase03
//
//  Created by 志水 友輔 on 2013/06/15.
//  Copyright (c) 2013年 志水 友輔. All rights reserved.
//

#include "myMath.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <list>
#include <numeric>

// http://d.hatena.ne.jp/teramonagi/20120627/1340805908から拝借
// 平均を取得
//template <template<class T, class Allocator = std::allocator<T> > class Container>
//double mean(Container<double> & x) {
//	return std::accumulate(x.begin(), x.end(), 0.0) / x.size();
//}
//
//// 分散を取得
//template <template<class T, class Allocator = std::allocator<T> > class Container>
//double var(Container<double> & x) {
//	double size = x.size();
//	double x_mean = mean(x);
//	return (std::inner_product(x.begin(), x.end(), x.begin(), 0.0) - x_mean * x_mean * size)/ (size - 1.0);
//}
//
//// 標準偏差を取得
//template <template<class T, class Allocator = std::allocator<T> > class Container>
//double sd(Container<double> & x) {
//	return std::sqrt(var(x));
//}
