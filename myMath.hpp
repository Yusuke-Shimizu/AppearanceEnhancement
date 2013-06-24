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

template <template<class T, class Allocator = std::allocator<T> > class Container>
//double mean(Container<double> & x);
double mean(Container<double> & x) {
	return std::accumulate(x.begin(), x.end(), 0.0) / x.size();
}
template <template<class T, class Allocator = std::allocator<T> > class Container>
//double var(Container<double> & x);
double var(Container<double> & x) {
	double size = x.size();
	double x_mean = mean(x);
	return (std::inner_product(x.begin(), x.end(), x.begin(), 0.0) - x_mean * x_mean * size)/ (size - 1.0);
}

template <template<class T, class Allocator = std::allocator<T> > class Container>
//double sd(Container<double> & x);
double sd(Container<double> & x) {
	return std::sqrt(var(x));
}
#endif /* defined(__cameraBase03__myMath__) */
