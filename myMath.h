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
template <template<class T, class Allocator = std::allocator<T> > class Container>
double mean(Container<double> & x);
template <template<class T, class Allocator = std::allocator<T> > class Container>
double var(Container<double> & x);
template <template<class T, class Allocator = std::allocator<T> > class Container>
double sd(Container<double> & x);
#endif /* defined(__cameraBase03__myMath__) */
