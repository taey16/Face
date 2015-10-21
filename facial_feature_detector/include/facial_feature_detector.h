/*
 * facial_feature_detector.h
 *
 *  Created on: Nov 11, 2013
 *      Author: hyunchul, minu
 */

#ifndef FACIAL_FEATURE_DETECTOR_H_
#define FACIAL_FEATURE_DETECTOR_H_

#include <string>
#include <vector>
using namespace std;

#include "typedef.h"

namespace daum {

class FacialFeatureDetector {
public:
	FacialFeatureDetector();
	virtual ~FacialFeatureDetector();

	float detect(Image& image, Rect& rect, int angel);

	int load_model(string& file_name, string* date_string = NULL, bool load_all = false);
	int load_model(FILE* file, string* date_string = NULL, bool load_all = false);
	void print_model_info(string& model_file_name);

	int load_shape_data(string& file_name, vector<Point2f>& shape);
	int save_shape_data(string& file_name, vector<Point2f>& shape);

	int load_rect_data(string& file_name, vector<Rect>& rect);

	vector<Point2f> facial_feature_points;
	vector<Point2f> aligned_facial_feature_points;
	Mat face_motion_matrix;
	Mat face_motion_matrix_source_to_aligned;
	Mat face_motion_matrix_aligned_to_source;

private:
	void inference_part(Image& image, vector<Point2f>& p);
	Point2f inference_part(vector<float>& phi, Mat& B, Mat& b);

	void inference_motion_whole(vector<float>& delta, vector<float>& phi, Mat& MB, Mat& Mb);
	Mat inference_motion_whole(Image& img);
	float converge_iteration(Image& image_prepare, Mat& motion_transform);


	void inference_whole(Image& image, vector<Point2f>& p);
	void inference_whole(vector<Point2f>& delta, vector<float>& phi, Mat& B, Mat& b);
	float cal_confidence(Image& img, vector<Point2f>& p);
	float cal_confidence(vector<float>& feature_vector, vector<float>& svm_weights);
	void postprocessing(Image& image);
	int load_layer(FILE* file, Mat& B, Mat& b);

	struct REGRESSION_MODEL{
		vector<Point2f> mean_shape;
		vector<vector<Mat> > B;
		vector<vector<Mat> > b;
		vector<float> svm_weights;

		// regression for motion
		vector<vector<Mat> > MB;
		vector<vector<Mat> > Mb;
	} regression_model;

	int canonical_face_h;
	int canonical_face_v;
	int canonical_patch_h;
	int canonical_patch_v;

	int num_models;
	FILE* model_fp;
	int model_seek_pos;
};

} /* namespace daum */

#endif /* FACIAL_FEATURE_DETECTOR_H_ */
