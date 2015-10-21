/*
 * feature.cpp
 *
 *  Created on: Nov 11, 2013
 *      Author: hyunchul
 */

#include "feature.h"

namespace daum {

Feature::Feature() {
	// TODO Auto-generated constructor stub
	set_configuration(8,4,8);
}

Feature::~Feature() {
	// TODO Auto-generated destructor stub
}

void Feature::set_configuration(int cell_size, int block_num, int bin_num)
{
	feature_configure.cell_size = cell_size;
	feature_configure.block_num = block_num;
	feature_configure.bin_num = bin_num;
}

void Feature::calc_HOG_descriptor(Image& image)
{
	// change image to array
	int width = image.size().width;
	int height = image.size().height;
	vector<unsigned char> image_array(width * height);
	vector<unsigned char>::iterator image_array_iter = image_array.begin();
	for(int i=0; i<height; i++)
	{
		for(int j=0; j<width; j++)
		{
			*image_array_iter++ = image.at(i,j);
		}
	}
	unsigned char *image_ptr = &(image_array[0]);

	// set common feature option
	FEATURE_OPTION feature_option;
	set_Feature_option(&feature_option,\
			3, 32, 32, 1,\
			1, 1,\
			2, feature_configure.bin_num, feature_configure.block_num, feature_configure.cell_size);
	// get HOG feature
	int feature_num;
	COMMON_FEATURE *common_feature = getFeature(&feature_num, &image_ptr, width, height, feature_option);

	// set to feature vector
	feature.clear();
	for(int i=0; i<feature_num; i++)
	{
		feature.insert(feature.end(), common_feature[i].feature, common_feature[i].feature + common_feature[i].dim);
	}

	// release common feature
	for(int i=0; i<feature_num; i++)
	{
		release_common_feature(&(common_feature[i]));
	}
	delete [] common_feature;
}

void Feature::calc_HOG_descriptor_new(Image& image)
{
	// change image to array
	int width = image.size().width;
	int height = image.size().height;
	unsigned char *image_ptr = image.ptr();

	// set common feature option
	FEATURE_OPTION feature_option;
	set_Feature_option(&feature_option,\
			12, 32, 32, 1,\
			1, 1,\
			2, feature_configure.bin_num, feature_configure.block_num, feature_configure.cell_size);
	// get HOG feature
	int feature_num;
	COMMON_FEATURE *common_feature = getFeature(&feature_num, &image_ptr, width, height, feature_option);

	// set to feature vector
	feature.clear();
	for(int i=0; i<feature_num; i++)
	{
		feature.insert(feature.end(), common_feature[i].feature, common_feature[i].feature + common_feature[i].dim);
	}

	// release common feature
	for(int i=0; i<feature_num; i++)
	{
		release_common_feature(&(common_feature[i]));
	}
	delete [] common_feature;
}


void Feature::calc_HOG_descriptor_for_motion(Image& image)
{

	// change image to array
	int width = image.size().width;
	int height = image.size().height;
	unsigned char *image_ptr = image.ptr();

	// set common feature option
	set_configuration(8,4,8);
	FEATURE_OPTION feature_option;
	set_Feature_option(&feature_option,\
			12, 128, 128, 7,\
			1, 1,\
			2, feature_configure.bin_num, feature_configure.block_num, feature_configure.cell_size);
	// get HOG feature
	int feature_num;
	COMMON_FEATURE *common_feature = getFeature(&feature_num, &image_ptr, width, height, feature_option);

	// set to feature vector
	feature.clear();
	for(int i=0; i<feature_num; i++)
	{
		feature.insert(feature.end(), common_feature[i].feature, common_feature[i].feature + common_feature[i].dim);
	}

	// release common feature
	for(int i=0; i<feature_num; i++)
	{
		release_common_feature(&(common_feature[i]));
	}
	delete [] common_feature;
}

} /* namespace daum */
