/*
 * facial_feature_detector.cpp
 *
 *  Created on: Nov 11, 2013
 *      Author: hyunchul, minu
 */

#include "facial_feature_detector.h"
#include "feature.h"
#include "image_processing.h"
#include "stop_watch.h"
#include "fast_edge.h"

#include <cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include <cstdio>

namespace daum {

FacialFeatureDetector::FacialFeatureDetector() {
	model_seek_pos = 0;
	model_fp = NULL;
	num_models = 0;

	canonical_face_h = 224;//120;
	canonical_face_v = 224;//120;
	canonical_patch_h = 32;
	canonical_patch_v = 32;
}

FacialFeatureDetector::~FacialFeatureDetector() {
}

float FacialFeatureDetector::detect(Image& image, Rect& rect, int angle)
{
	facial_feature_points = regression_model.mean_shape;

	// modify rectangle to fix aspect ratio
	rect.width = rect.height * canonical_face_h / canonical_face_v;

	// prepare: crop, resize, transform from source image to canonical face
	Image image_prepare;
	Mat transform_matrix;
	preparation(image_prepare, transform_matrix, image, rect, canonical_face_h, canonical_face_v);
	if(angle != 0)
	{
		Point2f offset(image_prepare.cols>>1, image_prepare.rows>>1);
		Mat pre_transform = get_affine_transform_matrix(1.0f, 1.0f, -angle * M_PI/180, offset, offset);
		image_prepare = transform_fast(image_prepare, pre_transform);
		transform_matrix = pre_transform.inv() * transform_matrix;
	}

	// regression
	Mat motion_transform;	// canonical face to aligned image
	float confidence = converge_iteration(image_prepare, motion_transform);

	// motion matrix update
	face_motion_matrix_source_to_aligned = motion_transform * transform_matrix;	// source image to aligned image
	face_motion_matrix_aligned_to_source = face_motion_matrix_source_to_aligned.inv();	// aligned image to source_image
	{
	    float *ptr = (float*)face_motion_matrix_aligned_to_source.ptr();
	    float val = *(ptr+8);
	    for(int j=0; j<9; j++) {
	    	*ptr = *ptr / val;
	    	ptr++;
	    }
    }

	// return to image coordinate
	aligned_facial_feature_points = facial_feature_points;
	perturb_shape(facial_feature_points, facial_feature_points, face_motion_matrix_aligned_to_source);
	//Image wrapped_face = transform( image, face_motion_matrix_aligned_to_source );
	//aligned_face = wrapped_face;
	//similarity_matrix = face_motion_matrix_aligned_to_source;

	return confidence;
}

float FacialFeatureDetector::converge_iteration(Image& image_prepare, Mat& motion_transform)
{
	//Mat data(image_prepare.rows, image_prepare.cols, CV_8U, image_prepare.ptr());

	// find motion parameter: image_prepare to aligned image
	StopWatch motion_time;
	motion_time.start();
	motion_transform = inference_motion_whole(image_prepare);
	motion_time.stop();
	printf("InferenceMotion Time = %.0f ms\n", motion_time.elapsed_msecs());
	
	// motion compensation
	Mat motion_transform_inv = motion_transform.inv();
	{
	    float *ptr = (float*)motion_transform_inv.ptr();
	    float val = *(ptr+8);
	    for(int j=0; j<9; j++)
	    {
	    	*ptr = *ptr / val;
	    	ptr++;
	    }
    }
	Image image_temp = transform_fast(image_prepare, motion_transform_inv);
/*
	Mat data1(image_temp.rows, image_temp.cols, CV_8U, image_temp.ptr());
	Mat image_to_write = data1.clone();
	for(unsigned int i=0; i<facial_feature_points.size(); i++)
	{
		cv::circle(image_to_write, cv::Point2f(facial_feature_points[i].x, facial_feature_points[i].y), 1, cv::Scalar(255, 255, 0), 2);
	}
*/
	// find feature points
	StopWatch inference_time;
	inference_time.start();
	inference_whole(image_temp, facial_feature_points);
	inference_time.stop();
	
	printf("Inference Time = %.0f ms\n", inference_time.elapsed_msecs());

	// get confidence
	float confidence = cal_confidence(image_temp, facial_feature_points);

	return confidence;
}

void FacialFeatureDetector::postprocessing(Image& image)
{
	struct image img, img_scratch, img_scratch2;
	img.width = image.cols;
	img.height = image.rows;
	img.pixel_data = image.ptr();

	img_scratch = img;
	img_scratch.pixel_data = new unsigned char[img.width * img.height];
	img_scratch2 = img;
	img_scratch2.pixel_data = new unsigned char[img.width * img.height];

	gaussian_noise_reduce(&img, &img);

	//morph_close(&img, &img_scratch, &img_scratch2, &img);

	int* g = new int[img.height * img.width];
	int* dir = new int[img.height * img.width];
	int* g_x = new int[img.height * img.width];
	int* g_y = new int[img.height * img.width];

	calc_gradient_sobel(&img, g, dir);
	non_max_suppression(&img_scratch, g, dir);

	cv::Mat mat_dir(img.height, img.width, CV_8U);
	for ( int i=0 ; i<mat_dir.rows ; ++i ) {
		for ( int j=0 ; j<mat_dir.cols ; ++j ) {
			if ( dir[i*img.width + j] >=0 && dir[i*img.width + j] < 4 ) {
				mat_dir.at<uchar>(i, j) = dir[i*img.width + j] * 64;
			}
		}
	}
	//cv::imshow("", mat_dir); cv::waitKey();

	cv::Mat mat_out(img.height, img.width, CV_8U, img.pixel_data);
	cv::Mat rgb_out;
	cv::cvtColor(mat_out, rgb_out, CV_GRAY2BGR);
	for ( int j=0 ; j<(int)facial_feature_points.size() ; ++j ) {
		Point2f& pt = facial_feature_points[j];
		cv::Point2f center(pt.x, pt.y);
		cv::circle(rgb_out, center, 2, cv::Scalar(0, 255, 0), -2);
	}

	for ( int  k=0 ; k<10 ; ++k ) {
		Point2f& pt = facial_feature_points[k];
		int x = round(pt.x);
		int y = round(pt.y);
		float val = img_scratch.pixel_data[img_scratch.width * y + x];
		float maxv = val * val;
		int maxx = x;
		int maxy = y;

		const int M = 2.0;// / * img_scratch.width / (canonical_face_h*2);
		for ( int yy=max(0, y-M) ; yy<=min(img_scratch.height, y+M) ; ++yy ) {
			for ( int xx=max(0, x-M) ; xx<=min(img_scratch.width, x+M) ; ++xx ) {
				val = img_scratch.pixel_data[img_scratch.width * yy + xx];
				//if ( val < 50 )
				{
					val = val * val;// / sqrt((x-xx)*(x-xx) + (y-yy)*(y-yy));
					if ( val > maxv  )
					{
						maxv = val;
						maxx = xx;
						maxy = yy;
					}
				}
			}
		}
		pt.x = maxx;
		pt.y = maxy;
		cv::circle(rgb_out, cv::Point(maxx, maxy), 2, cv::Scalar(0, 0, 255), -2);
	}

	cv::imshow("", rgb_out);
	cv::waitKey();

	delete [] g;
	delete [] g_x;
	delete [] g_y;
	delete [] dir;

	delete [] img_scratch.pixel_data;
	delete [] img_scratch2.pixel_data;
}

float FacialFeatureDetector::cal_confidence(Image& img, vector<Point2f>& p)
{
	// get feature of shape
	vector<float> phi;
	for(unsigned int j=0; j<p.size(); j++)
	{
		Point p_round(round(p[j].x), round(p[j].y));
		// feature
		Image image_patch = get_patch(img, p_round, Size(canonical_patch_h, canonical_patch_v));
		Feature feature;
		feature.calc_HOG_descriptor_new(image_patch);
		phi.insert(phi.end(), feature.feature.begin(), feature.feature.end());
	}

	return cal_confidence(phi, regression_model.svm_weights);
}

float FacialFeatureDetector::cal_confidence(vector<float>& feature_vector, vector<float>& svm_weights)
{
	float confidence = *(svm_weights.rbegin());
	for(unsigned int i=0; i<feature_vector.size(); i++)
	{
		confidence += feature_vector[i] * svm_weights[i];
	}

	return confidence;
}

void FacialFeatureDetector::inference_whole(Image& img, vector<Point2f>& p)
{
	int layer_num = 0;
	if ( model_seek_pos > 0 )
	{
		fseek(model_fp, model_seek_pos, SEEK_SET);

		//__android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE", "model [%d] ", i);
		fread(&layer_num, sizeof(int), 1, model_fp);
	}
	else
	{
		layer_num = regression_model.B[0].size();
	}

	for(int j=0; j<layer_num ; j++)
	{
		//while(1)
		{
			vector<float> phi;

			StopWatch patch_time;
			StopWatch feature_time;
			if(j != 0)
			{
				for(unsigned int i=0; i<p.size(); i++)
				{
					// get image patch at p[i]
					patch_time.start();
					Image image_patch = get_patch(img, Point(round(p[i].x), round(p[i].y)), Size(canonical_patch_h, canonical_patch_v));
					patch_time.stop();

					// get phi (feature of the image patch)
					feature_time.start();
					Feature feature;
					feature.calc_HOG_descriptor_new(image_patch);
					phi.insert(phi.end(), feature.feature.begin(), feature.feature.end());
					feature_time.stop();
				}
			}
			else
			{
				// get image patch at p[i]
				patch_time.start();
				Image image_patch = get_patch(img, Point(img.cols>>1, img.rows>>1), Size(canonical_face_h, canonical_face_v));
				patch_time.stop();
				
				// get phi (feature of the image patch)
				feature_time.start();
				Feature feature;
				feature.calc_HOG_descriptor_for_motion(image_patch);
				phi.insert(phi.end(), feature.feature.begin(), feature.feature.end());
				feature_time.stop();
			}

		    StopWatch delta_time;

		    // get delta position
			vector<Point2f> delta_p;
			if ( model_seek_pos > 0 )
			{
				Mat B, b;
				StopWatch load_time;
				load_time.start();
				load_layer(model_fp, B, b);
				load_time.stop();
#ifdef __ANDROID__
				__android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE",	"ModelLoad Time = %.3f", load_time.elapsed_msecs());
#endif
				delta_time.start();
				inference_whole(delta_p, phi, B, b);
				delta_time.stop();
			}
			else
			{
				delta_time.start();
				inference_whole(delta_p, phi, regression_model.B[0][j], regression_model.b[0][j]);
				delta_time.stop();
			}

#ifdef __ANDROID__
			__android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE",	"GetPatch Time = %.3f",  patch_time.accumulated_msecs());
		    __android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE",	"FeatureCalc Time = %.3f", feature_time.accumulated_msecs());
		    __android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE",	"DeltaCalc Time = %.3f", delta_time.accumulated_msecs());
#else
		    printf("GetPatch Time = %.0f ms\n", patch_time.accumulated_msecs());
		    printf("FeatureCalc Time = %.0f ms\n", feature_time.accumulated_msecs());
		    printf("DeltaCalc Time = %.0f ms\n", delta_time.elapsed_msecs());
#endif

			// update position
			for(unsigned int i=0; i<p.size(); i++)
			{
				p[i] = Point2f(round(p[i].x), round(p[i].y)) + delta_p[i];
			}

			float max_delta = 0.0f;
			for(unsigned int i=0; i<p.size(); i++)
			{
				if(max_delta < fabs(delta_p[i].x))
					max_delta = fabs(delta_p[i].x);
				if(max_delta < fabs(delta_p[i].y))
					max_delta = fabs(delta_p[i].y);
			}

			printf("max. of delta = %f pixel\n", max_delta);
//			if(max_val < 0.1f)
//				break;
		}
	}
}

void FacialFeatureDetector::inference_whole(vector<Point2f>& delta, vector<float>& phi, Mat& B, Mat& b)
{
#ifdef __USE_DAUMCV__
	Mat phi_mat(phi.size(), 1, phi.data());
#else
	Mat phi_mat(phi.size(), 1, CV_32F, phi.data());
#endif


	// matrix algebra
	// Mat32 delta_p_mat = B * phi_mat + b;
	int m = B.rows;
	int n = B.cols;
	//Mat delta_p_mat(m, n);

	Point2f pt;
	delta.clear();
	delta.reserve(phi_mat.rows >> 1);

	for ( int i=0 ; i<m ; ++i )
	{
		float sum = 0;
		float* pB = B.ptr(i, 0);
		float* pp = phi_mat.ptr(0, 0);
		for ( int j=0 ; j<n ; j += 2 )
		{
			sum += (*pB++) * (*pp++);
			sum += (*pB++) * (*pp++);
		}
		if ( i & 1 )
		{
			pt.y = sum + b.at<float>(i, 0);
			delta.push_back(pt);
		}
		else
		{
			pt.x = sum + b.at<float>(i, 0);
		}
	}
}


Mat FacialFeatureDetector::inference_motion_whole(Image& img)
{
	// transform from img to aligned image
	Mat motion_transform = Mat::eye(3, 3);
	Image img_temp = img;
	for(unsigned int i=0; i<regression_model.MB[0].size(); i++)
	{
//		for(int k=0; k<10; k++)
		{
			if(i!=0)
			{
				// transform image
				Mat motion_transform_inv = motion_transform.inv();	// aligned image to img
				{
	                float *ptr = (float*)motion_transform_inv.ptr();
	                float val = *(ptr+8);
	                for(int j=0; j<9; j++)
	                {
	                	*ptr = *ptr / val;
	                	ptr++;
	                }
                }
				img_temp = transform_fast(img, motion_transform_inv);
			}

			vector<float> phi;
			// get image patch at p[i]
			Image image_patch = get_patch(img_temp, Point(img_temp.cols>>1, img_temp.rows>>1), Size(canonical_face_h, canonical_face_v));
			// get phi (feature of the image patch)
			Feature feature;
			feature.calc_HOG_descriptor_for_motion(image_patch);
			phi.insert(phi.end(), feature.feature.begin(), feature.feature.end());

			// get delta motion
			vector<float> delta_p;
			inference_motion_whole(delta_p, phi, regression_model.MB[0][i], regression_model.Mb[0][i]);
			printf("ds: %.2f %%, dtheta: %.2f degree, dtx: %.2f px, dty: %.2f px\n", delta_p[0], delta_p[1] * 180/M_PI, delta_p[2], delta_p[3]);

			// get transform matrix from img to image
			float scale = 1.0f + delta_p[0];
			float theta = delta_p[1];
			Point2f offset(img_temp.cols>>1, img_temp.rows>>1);
			Point2f trans(delta_p[2] + offset.x, delta_p[3] + offset.y);
			// aligned image to further aligned image
			Mat transform_matrix = get_affine_transform_matrix(scale, scale, theta, offset, trans);

			// update motion_transform: img to further aligned image
			motion_transform = transform_matrix * motion_transform; //////////// check it

			// stop condition
			if(fabs(delta_p[0]) < 0.01f\
			&& fabs(delta_p[1]) < 0.02f\
			&& fabs(delta_p[2]) < 1.0f\
			&& fabs(delta_p[3]) < 1.0f)
				break;
		}
	}

	return motion_transform;
}


void FacialFeatureDetector::inference_motion_whole(vector<float>& delta, vector<float>& phi, Mat& MB, Mat& Mb)
{
#ifdef __USE_DAUMCV__
	Mat phi_mat(phi.size(), 1, phi.data());
#else
	Mat phi_mat(phi.size(), 1, CV_32F, phi.data());
#endif
	// matrix algebra
	Mat delta_p_mat = MB * phi_mat + Mb;

	vector<float> delta_p(delta_p_mat.rows);
	memcpy(&(delta_p[0]), delta_p_mat.ptr(), sizeof(float)*delta_p.size());

	delta.swap(delta_p);
}



void FacialFeatureDetector::inference_part(Image& img, vector<Point2f>& p)
{
	for(unsigned int j=0; j<regression_model.B[0].size(); j++)
	{
//		while(1)
		{
			float max_delta = 0.0f;

			for(unsigned int i=0; i<p.size(); i++)
			{
				// get image patch at p[i]
				Image image_patch = get_patch(img, Point(round(p[i].x), round(p[i].y)), Size(canonical_patch_h, canonical_patch_v));

				// get phi (feature of the image patch)
				Feature feature;
//				feature.calc_HOG_descriptor(image_patch);
				feature.calc_HOG_descriptor_new(image_patch);

				// get delta position
				Point2f delta_p = inference_part(feature.feature, regression_model.B[i][j], regression_model.b[i][j]);

				// update position
				p[i] = Point2f(round(p[i].x), round(p[i].y)) + delta_p;
				//__android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE",	"udpate delta(%.3f,%.3f), to(%.3f, %.3f)", delta_p.x, delta_p.y, p[i].x, p[i].y);

				if(max_delta < fabs(delta_p.x))
					max_delta = fabs(delta_p.x);
				if(max_delta < fabs(delta_p.y))
					max_delta = fabs(delta_p.y);
			}

			printf("max. of delta = %f pixel\n", max_delta);
			//__android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE",	"max. of delta = %.3f pixel", max_delta);

//			if(max_val < 0.1f)
//				break;
		}
	}
}

Point2f FacialFeatureDetector::inference_part(vector<float>& phi, Mat& B, Mat& b)
{
#ifdef __USE_DAUMCV__
	Mat phi_mat(phi.size(), 1, phi.data());
#else
	Mat phi_mat(phi.size(), 1, CV_32F, phi.data());
#endif

	// matrix algebra
	Mat delta_p_mat = B * phi_mat + b;

	Point2f delta_p(delta_p_mat.at<float>(0,0), delta_p_mat.at<float>(1,0));
	return delta_p;
}

int FacialFeatureDetector::load_shape_data(string& file_name, vector<Point2f>& shape)
{
	FILE *file = fopen(file_name.c_str(), "rt");
	if(file == NULL)
	{
		printf("error! %s cannot be open!\n", file_name.c_str());
		return -1;
	}

	// for LFPW src3
	int shape_num;
	fscanf(file, "%d", &shape_num);
	if(shape_num <= 0 || shape_num > 100)
	{
		fclose(file);
		return -1;
	}
	int point_num;
	fscanf(file, "%d", &point_num);
	if(point_num <= 0 || point_num > 100)
	{
		fclose(file);
		return -1;
	}

	shape.resize(point_num);
	//float dummy;
	for(int i=0; i<point_num; i++)
	{
		//fscanf(file, "%f %f %f", &(shape[i].x), &(shape[i].y), &dummy);
		// for LFPW src3
		fscanf(file, "%f %f", &(shape[i].x), &(shape[i].y));
	}

	fclose(file);

	return 0;
}

int FacialFeatureDetector::save_shape_data(string& file_name, vector<Point2f>& shape)
{
	FILE *file = fopen(file_name.c_str(), "wt");
	if(file == NULL)
	{
		printf("error! %s cannot be written!\n", file_name.c_str());
		return -1;
	}

	fprintf(file, "%d\n", (int)shape.size());
	int dummy = 0;
	for(unsigned int i=0; i<shape.size(); i++)
	{
		fprintf(file, "%f %f %d\n", shape[i].x, shape[i].y, dummy);
	}

	fclose(file);

	return 0;
}

int FacialFeatureDetector::load_rect_data(string& file_name, vector<Rect>& rect)
{
	FILE *file = fopen(file_name.c_str(), "rt");
	if(file == NULL)
	{
		printf("error! %s cannot be open!\n", file_name.c_str());
		return -1;
	}

	int angle;
	int rect_num;
	fscanf(file, "%d", &rect_num);
	for(int i=0; i<rect_num; i++)
	{
		Rect rect_;
		fscanf(file, "%d %d %d %d %d", &(rect_.x), &(rect_.y), &(rect_.width), &(rect_.height), &angle);
		rect.push_back(rect_);
	}

	fclose(file);

	return 0;
}

int FacialFeatureDetector::load_model(string& file_name, string* date_string, bool load_all)
{
	FILE *file = fopen(file_name.c_str(), "rb");

	int return_val = load_model(file, date_string, load_all);

	if ( load_all && file != NULL ) fclose(file);

	return return_val;
}

int FacialFeatureDetector::load_layer(FILE* file, Mat& B, Mat& b)
{
	int width, height;
	fread(&width, sizeof(int), 1, file);
	fread(&height, sizeof(int), 1, file);
#ifdef __USE_DAUMCV__
	B = Mat(height, width);
	b = Mat(height, 1);
#else
	regression_model.B[i][j] = Mat(height, width, CV_32F);
	regression_model.b[i][j] = Mat(height, 1, CV_32F);
#endif
	fread(B.ptr(), sizeof(float), height*width, file);
	fread(b.ptr(), sizeof(float), height, file);

	return 0;
}

int FacialFeatureDetector::load_model(FILE* file, string* date_string, bool load_all)
{
	//FILE *file = fdopen(fd, "rb");
	if(file == NULL)
	{
		printf("error! cannot be open!\n");
		return -1;
	}
	//__android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE", "File Open success file=%d", file);

	model_fp = file;

	fread(&canonical_face_h, sizeof(int), 1, file);
	fread(&canonical_face_v, sizeof(int), 1, file);
	fread(&canonical_patch_h, sizeof(int), 1, file);
	fread(&canonical_patch_v, sizeof(int), 1, file);

	int weight_num;
	fread(&weight_num, sizeof(int), 1, file);
	//__android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE", "weight_num = %d", weight_num);
	regression_model.svm_weights.resize(weight_num);
	fread(&(regression_model.svm_weights[0]), sizeof(float), weight_num, file);

	int point_num = 0;
	fread(&point_num, sizeof(int), 1, file);
	//__android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE", "point_num = %d", point_num);
	regression_model.mean_shape.resize(point_num);
	fread(&(regression_model.mean_shape[0]), sizeof(Point2f), point_num, file);

	int motion_model_num;
	fread(&motion_model_num, sizeof(int), 1, file);
	regression_model.MB.resize(motion_model_num);
	regression_model.Mb.resize(motion_model_num);
	for(unsigned int i=0; i<regression_model.MB.size(); i++)
	{
		int layer_num;
		fread(&layer_num, sizeof(int), 1, file);
		regression_model.MB[i].resize(layer_num);
		regression_model.Mb[i].resize(layer_num);
		for(int j=0; j<layer_num; j++)
		{
			int width, height;
			fread(&width, sizeof(int), 1, file);
			fread(&height, sizeof(int), 1, file);
			regression_model.MB[i][j] = Mat(height, width);
			regression_model.Mb[i][j] = Mat(height, 1);
			fread(regression_model.MB[i][j].ptr(), sizeof(float), height*width, file);
			fread(regression_model.Mb[i][j].ptr(), sizeof(float), height, file);
		}
	}

	fread(&num_models, sizeof(int), 1, file);
	//__android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE", "model_num = %d", model_num);

	if ( load_all )
	{
		regression_model.B.resize(num_models);
		regression_model.b.resize(num_models);
		for(unsigned int i=0; i<regression_model.B.size(); i++)
		{
			int layer_num;
			//__android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE", "model [%d] ", i);
			fread(&layer_num, sizeof(int), 1, file);
			regression_model.B[i].resize(layer_num);
			regression_model.b[i].resize(layer_num);
			for(int j=0; j<layer_num; j++)
			{
				load_layer(file, regression_model.B[i][j], regression_model.b[i][j]);
			}
		}

		if(date_string != NULL)
		{
			char date_char[100];
			fread(date_char, sizeof(char), 100, file);
			*date_string = date_char;
		}
		model_seek_pos = -1;
	}
	else
	{
		model_seek_pos = ftell(file);
	}

	return 0;
}

void FacialFeatureDetector::print_model_info(string& model_file_name)
{
	string date_string;
	int state = load_model(model_file_name, &date_string, true);
	if(state != 0)
	{
		cout << "model file error!" << endl;
		return;
	}

	cout << endl;
	cout << "[regression model info.]" << endl;
	cout << "canonical_face_h = " << canonical_face_h << endl;
	cout << "canonical_face_v = " << canonical_face_v << endl;
	cout << "canonical_patch_h = " << canonical_patch_h << endl;
	cout << "canonical_patch_v = " << canonical_patch_v << endl;
	cout << "# of weights: " << regression_model.svm_weights.size() << endl;
	cout << "# of shape points: " << regression_model.mean_shape.size() << endl;
	cout << "# of parts: " << regression_model.B.size() << endl;
	cout << "# of layers: " << regression_model.B[0].size() << endl;
	cout << "size of B: " << regression_model.B[0][0].rows << " x " << regression_model.B[0][0].cols << endl;
	cout << "size of b: " << regression_model.b[0][0].rows << " x " << regression_model.b[0][0].cols << endl;
	cout << "date of saving: " << date_string << endl;
}

};
