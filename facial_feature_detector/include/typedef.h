/*
 * typedef.h
 *
 *  Created on: Nov 12, 2013
 *      Author: minu
 */

#ifndef TYPEDEF_H_
#define TYPEDEF_H_

#include "Matrix.hpp"
#include "Core.hpp"

#include <stdint.h>

#define __USE_DAUMCV__

#ifdef __USE_DAUMCV__

typedef unsigned char uchar;

// TODO: NDK로 포팅할 때 잘(?) 변경 할 예정
typedef daum::Matrix_<float>	Mat;
typedef daum::Matrix_<uchar>	Image;
typedef daum::Matrix_<int>	Mat32;
typedef daum::Matrix_<int64_t>	Mat64;
typedef daum::Matrix_<float>	Matrix;
//typedef daum::Point_<int>		Point;
//typedef daum::Rect_<int>		Rect;
//typedef daum::Size_<int>		Size;
//typedef daum::Point_<float>		Point2f;

#else

#include <cv.h>
#include <opencv2/highgui/highgui.hpp>

typedef cv::Mat					Mat;
typedef daum::Matrix_<uchar> 	Image;
typedef daum::Matrix_<float>	Matrix;
typedef cv::Point				Point;
typedef cv::Rect				Rect;
typedef cv::Size				Size;
typedef cv::Point2f				Point2f;

#endif

#endif /* TYPEDEF_H_ */
