/*
 * regression_model.h
 *
 *  Created on: Dec 6, 2013
 *      Author: minu
 */

#ifndef REGRESSION_MODEL_H_
#define REGRESSION_MODEL_H_

#include <cstdio>

#include "typedef.h"

class CanonicalSize
{
public:
	CanonicalSize(int _face_h = 120, int _face_v = 120, int _patch_h = 32, int _patch_v = 32) :
		face_h(_face_h), face_v(_face_v), patch_h(_patch_h), patch_v(_patch_v)
	{
		face_h = _face_h;
		face_v = _face_v;
		patch_h = _patch_h;
		patch_v = _patch_v;
	}

	bool ReadFromFile(FILE* fp, int offset = 0)
	{
		if ( fp == NULL ) return false;
		if ( offset > 0 ) fseek(fp, offset, SEEK_SET);

		fread(&face_h,  sizeof(face_h), 1, fp);
		fread(&face_v,  sizeof(face_v), 1, fp);
		fread(&patch_h, sizeof(patch_h), 1, fp);
		fread(&patch_v, sizeof(patch_v), 1, fp);

		return true;
	}

	bool WriteToFile(FILE* fp, int offset = 0)
	{
		if ( fp == NULL ) return false;
		if ( offset > 0 ) fseek(fp, offset, SEEK_SET);

		fwrite(&face_h,  sizeof(face_h), 1, fp);
		fwrite(&face_v,  sizeof(face_v), 1, fp);
		fwrite(&patch_h, sizeof(patch_h), 1, fp);
		fwrite(&patch_v, sizeof(patch_v), 1, fp);

		return true;
	}

	int face_h;
	int face_v;
	int patch_h;
	int patch_v;
};

class RegressionLayer {
public:
	RegressionLayer()
	{
		loaded = false;
	}

	bool ReadFromFile(FILE* fp, int offset = 0)
	{
		if ( fp == NULL ) return false;
		if ( offset > 0 ) fseek(fp, offset, SEEK_SET);

		int width, height;
		fread(&width, sizeof(int), 1, fp);
		fread(&height, sizeof(int), 1, fp);

		B = Mat(height, width);
		b = Mat(height, 1);

		fread(B.ptr(), sizeof(B[0]), B.size().area(), fp);
		fread(b.ptr(), sizeof(b[0]), b.size().area(), fp);

		loaded = true;
		return true;
	}

	bool WriteToFile(FILE* fp, int offset = 0)
	{
		if ( fp == NULL ) return false;
		if ( offset > 0 ) fseek(fp, offset, SEEK_SET);

		fwrite(&B.cols, sizeof(B.cols), 1, fp);
		fwrite(&B.rows, sizeof(B.rows), 1, fp);
		fwrite(B.ptr(), sizeof(float), B.size().area(), fp);
		fwrite(b.ptr(), sizeof(float), b.size().area(), fp);

		return true;
	}

	Mat B, b;
	bool loaded;
};

class RegressionModel {
public:
	RegressionModel()
	{
		fp_model = NULL;
	}

	bool ReadFromFile(FILE* fp, int offset = 0)
	{
		if ( fp == NULL ) return false;
		if ( offset > 0 ) fseek(fp, offset, SEEK_SET);
		fp_model = fp;

		int model_num;
		fread(&model_num, sizeof(int), 1, fp);
		layers_.resize(model_num);
		for ( int i=0 ; i<model_num ; ++i )
		{
			layers_[i].ReadFromFile(fp);
		}

		return true;
	}

	bool WriteToFile(FILE* fp, int offset = 0)
	{
		if ( fp == NULL ) return false;
		if ( offset > 0 ) fseek(fp, offset, SEEK_SET);



		return true;
	}

	RegressionLayer& GetRegressionLayer(int layer_index)
	{
		if ( !layers_.empty() )
		{
			return layers_[layer_index];
		}

		temp_layer.ReadFromFile(fp_model);
		return temp_layer;
	}

	vector<RegressionLayer> layers_;

private:
	FILE* fp_model;
	RegressionLayer temp_layer;
};

/*
class RegressionModel {
public:
	//RegressionModel();
	RegressionModel(const string& filename);
	RegressionModel(const FILE* fp);
	RegressionLayer& GetRegressionLayer(int layer_num);
	virtual ~RegressionModel();

	vector<Point2f> mean_shape;
	vector<float> svm_weights;

	int canonical_face_h = 120;
	int canonical_face_v = 120;
	int canonical_patch_h = 32;
	int canonical_patch_v = 32;

	int num_models = 0;
	int num_layers = 0;
	int num_points = 0;

private:
	vector<RegressionLayer> layers_;
	vector<int> layer_begin_pos;

	bool LoadHeaders();
};

class RegressionModelLoader
{
public:
	RegressionModelLoader(const string& filename, bool in_memory=false);
	RegressionModelLoader(const FILE* fp, bool in_memory=false);

	RegressionModel& GetRegressionModel(int model_num)
	{
		return models_[model_num];
	}

private:
	bool in_memory_ = false;
	vector<RegressionModel> models_;
	vector<int> model_begin_pos_;

	FILE* fp_model_ = NULL;
};


RegressionModel::RegressionModel(const string& filename)
{
	FILE *fp = fopen(filename.c_str(), "rb");
	RegressionModel(fp);
}


RegressionModel::RegressionModel(const FILE* fp)
{
	fp_model_ = fp;
}


bool RegressionModel::LoadHeaders()
{
	if ( fp_model_ == NULL ) return false;
	fseek(fp_model_, 0, SEEK_SET);

	// data, version

	// canonical size
	fread(&canonical_face_h,  sizeof(int), 1, fp_model_);
	fread(&canonical_face_v,  sizeof(int), 1, fp_model_);
	fread(&canonical_patch_h, sizeof(int), 1, fp_model_);
	fread(&canonical_patch_v, sizeof(int), 1, fp_model_);

	// svm weights for confidence
	int weight_num;
	fread(&weight_num, sizeof(int), 1, fp_model_);
	fread(&(svm_weights[0]), sizeof(float), svm_weights.size(), fp_model_);

	// number of feature points;
	fread(&num_points, sizeof(int), 1, fp_model_);

	// mean shapes
	mean_shape.resize(num_points);
	fread(&(mean_shape[0]), sizeof(Point2f), num_points, fp_model_);

	// number of models;
	fread(&num_models, sizeof(int), 1, fp_model_);
	model_pos_ = ftell(fp_model_);

	return true;
}


bool RegressionModel::LoadNextLayerMatrix(Mat& B, Mat& b)
{
	if ( fp_model_ == NULL ) return false;

	if ( num_layers == 0 || layer_counter_ == num_layers )
	{
		layer_counter_ = 0;
		fseek(fp_model_, model_pos_, SEEK_SET);
		if ( fread(&num_layers, sizeof(int), 1, fp_model_) == 0 ) return false;
	}

	int width, height;
	fread(&width, sizeof(int), 1, fp_model_);
	fread(&height, sizeof(int), 1, fp_model_);

	B = Mat(height, width);
	b = Mat(height, 1);

	fread(B.ptr(), sizeof(B[0]), height*width, fp_model_);
	fread(b.ptr(), sizeof(b[0]), height, fp_model_);

	return true;
}


RegressionModel::~RegressionModel()
{
	if ( fp_close_flag_ && fp_model_ != NULL ) fclose(fp_model_);
}
*/

#endif /* REGRESSION_MODEL_H_ */

