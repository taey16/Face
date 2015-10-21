/*
 * image_processing.h
 *
 *  Created on: 2013. 11. 10.
 *      Author: minu, hyunchul
 */

#ifndef IMAGE_PROCESSING_H_
#define IMAGE_PROCESSING_H_

#include "typedef.h"
#include <vector>
using namespace std;

namespace daum {

Mat get_affine_transform_matrix(float scale_x, float scale_y, float theta_radian, Point2f& offset, Point2f& t);
Mat get_transform_matrix(vector<Point2f>& x, vector<Point2f>& y);
Image transform(const Image& src, Mat& M);
Image transform_fast(const Image& src, Mat& M, int width = 0, int height = 0);

void preparation(Image& processed_image, Mat& transform_matrix,\
		Image& image, Rect& rect, int cannonical_face_h, int cannonical_face_v);

void perturb_shape(vector<Point2f>& new_shape, vector<Point2f>& shape, Mat& M);
Point2f perturb_point(Point2f& p, Mat& M);

Image resize(const Image& src, Size dsize);
Image resize_fast(const Image& src, Size dsize);
Image convert_gray(const uchar* src, int rows, int cols, bool BGR=true);
Image flip(const Image& src, bool flag);

Image get_patch(const Image& image, const Point& center, Size size);
Image get_patch(const Image& image, const Point& center, Size size, float theta);

};



#endif /* IMAGE_PROCESSING_H_ */
