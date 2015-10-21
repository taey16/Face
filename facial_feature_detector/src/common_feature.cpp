/**@file
Function implementation for feature extraction.

Feature type:	0. 2D histogram
		1. 2D GIST global
		2. 2D SIFT
		3. 2D HOG global
		4. 2D SURF global
		5. 3D histogram
		6. 3D GIST - not available
		7. 3D SIFT - not available
		8. 3D HOG
		9. 3D SURF
		10. 2D mean, var
		11. 2D SIFT global 
Distance type:	L1 distance(0),
		L2 distance(1),
		infinite norm distance(2),
		normalized infinite norm distance(3),
		Chi-square(4),
		Hellinger(5),
		KL-divergence(6)

Copyright (c) 2011 Hyunchul Choi,
Multimedia Research Team, Daum corp.

@version 1.0
@date 2011.12.20
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <pthread.h>
#include "common_feature.h"

#include "fast_math.hpp"

#ifdef __cplusplus
extern "C" {
#endif

//#include "../SIFT_library/kdtree.h"

#ifdef __cplusplus
}
#endif


/** feature option */
FEATURE_OPTION feature_option;

// GIST gabor wavelete
/** Gabor wavelete for GIST calculation */
//image_list_t *GIST_gabor = NULL;
/** GIST orientation number for each scale */
int GIST_ORIENTATION_NUM[GIST_SCALE_NUM] = {4, 4, 8};

// HOG 3D normal axes
/** 4 bins tetrahedron: normal axes for 3D HOG calculation */
double tetrahedron_normal_axes[4][3] = {//
		{1,1,1}, {-1,-1,1}, {-1,1,-1}, {1,-1,-1}//
};
/** 6 bins hexahedron: normal axes for 3D HOG calculation */
double hexahedron_normal_axes[6][3] = {//
		{1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}//
};
/** 8 bins octahedron: normal axes for 3D HOG calculation */
double octahedron_normal_axes[8][3] = {//
		{1,1,1}, {1,1,-1}, {1,-1,1}, {1,-1,-1},//
		{-1,1,1}, {-1,1,-1}, {-1,-1,1}, {-1,-1,-1}//
};
/** 12 bins dodecahedron: normal axes for 3D HOG calculation */
double dodecahedron_normal_axes[12][3] = {//
		{0,1,PHI}, {0,1,-PHI}, {0,-1,PHI}, {0,-1,-PHI},//
		{1,PHI,0}, {1,-PHI,0}, {-1,PHI,0}, {-1,-PHI,0},//
		{PHI,0,1}, {PHI,0,-1}, {-PHI,0,1}, {-PHI,0,-1}//
};
/** 20 bins icosahedron: normal axes for 3D HOG calculation */
double icosahedron_normal_axes[20][3] = {//
		{1,1,1}, {1,1,-1}, {1,-1,1}, {1,-1,-1},//
		{-1,1,1}, {-1,1,-1}, {-1,-1,1}, {-1,-1,-1},//
		{0,1/PHI,PHI}, {0,1/PHI,-PHI}, {0,-1/PHI,PHI}, {0,-1/PHI,-PHI},//
		{1/PHI,PHI,0}, {1/PHI,-PHI,0}, {-1/PHI,PHI,0}, {-1/PHI,-PHI,0},//
		{PHI,0,1/PHI}, {PHI,0,-1/PHI}, {-PHI,0,1/PHI}, {-PHI,0,-1/PHI}//
};

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

FEATURE_FUNCTION feature_list[13] = {//
		calFeature_2Dhistogram,	// 0
		calFeature_2Dgist,		// 1
		calFeature_2Dsift,		// 2
		calFeature_2Dhog,		// 3
		calFeature_2Dsurf,		// 4
		calFeature_3Dhistogram,		// 5
		calFeature_3Dgist,		// 6
		calFeature_3Dsift,		// 7
		calFeature_3Dhog,		// 8
		calFeature_3Dsurf,		// 9
		calFeature_2Dmeanvar,		// 10
		calFeature_2Dsift_global,	// 11
		calFeature_2Dhog_fast		// 12
};

FEATURE_DISTANCE distance_measure_list[7] = {//
		L1_distance,				// 0
		L2_distance,				// 1
		infinite_norm_distance,			// 2
		normalized_infinite_norm_distance,	// 3
		Chi_square_distance,			// 4
		Hellinger_distance,			// 5
		Kullback_Leibler_divergence		// 6
};

/**
convert image into image_t.
@param img an image data
@param width image width
@param height image height
@return Returns image_t structured data.

template <typename T> image_t *Convert2image_t(T *img, int width, int height)
{
	image_t *src = image_new(width, height);

	double *src_ptr = src->data;
	T *img_ptr = img;
	for(int i=0;i<height; i++)
	{
		for(int j=0;j<width; j++)
		{
 *src_ptr++ = (double)(*img_ptr++);
		}
	}

	return src;
}
 */
void release_common_feature(COMMON_FEATURE *common_feature)
{
	delete [] common_feature->feature;
}

void set_Feature_option(//
		FEATURE_OPTION *option,//
		int feature_type, int width, int height, int subregion_num, int clip_frame_num, int step_frame_num,//
		int parameter0, int parameter1, int parameter2, int parameter3)
{
	int i, j;

	// use default options
	if(width==0 && height==0)
	{
		option->type = feature_type;
		option->width = 32;
		option->height = 32;
		option->subregion_num = 7;
		option->normalization_method = 1;	// 0: no norm, 1: L1 norm, 2: L2 norm
		option->characteristic_method = 0;	// 0: y=x, 1: y=sqrt(x), 2: y=x^2
		option->clip_frame_num = 1;
		option->step_frame_num = 1;
		option->feature_dim = 32;

		switch(feature_type)
		{
		case 0: // 2D histogram 0
			option->hist2d_bin_num = 32;
			option->feature_dim = 32;
			break;
/*			case 1: // 2D GIST 1
				option->gist2d_scale_num = GIST_SCALE_NUM;
				for(i=0; i<option->gist2d_scale_num; i++)
					option->gist2d_orientation_num[i] = GIST_ORIENTATION_NUM[i];
				initialize_GIST_wavelete(option->width, option->height, option->gist2d_scale_num, option->gist2d_orientation_num);
				option->feature_dim = 128;
				option->subregion_num = 1;
				break;
			case 2: // 2D SIFT 2
				option->width = 320;
				option->height = 240;
				option->subregion_num = 1;
				option->feature_dim = 128;
				break;
*/
			case 3: // 2D HOG 3
				 option->hog2d_bin_num = 4;
				 option->hog2d_block_size = 2;
				 option->hog2d_cell_size = 4;
				 option->feature_dim = option->hog2d_bin_num*option->hog2d_block_size*option->hog2d_block_size;
				 break;
			case 12: // 2D HOG 12
				 option->hog2d_bin_num = 4;
				 option->hog2d_block_size = 2;
				 option->hog2d_cell_size = 4;
				 option->feature_dim = option->hog2d_bin_num*option->hog2d_block_size*option->hog2d_block_size;
				 break;
			 case 4: // 2D SURF 4
				 option->surf2d_bin_num = 4;
				 option->surf2d_block_size = 2;
				 option->surf2d_cell_size = 4;
				 option->feature_dim = option->surf2d_bin_num*option->surf2d_block_size*option->surf2d_block_size;
				 break;
			 case 5: // 3D histogram 5
				 option->clip_frame_num = 2;
				 option->hist3d_bin_num = 32;
				 option->feature_dim = 32;
				 break;
			 case 6: // 3D GIST 6
				 option->clip_frame_num = 2;
				 break;
			 case 7: // 3D SIFT 7
				 option->clip_frame_num = 2;
				 break;
			 case 8: // 3D HOG 8
				 option->clip_frame_num = 2;
				 option->hog3d_bin_num = 8;
				 option->hog3d_block_size = 2;
				 option->hog3d_cell_size = 4;
				 for(i=0; i<option->hog3d_bin_num; i++)
					 for(j=0; j<3; j++)
						 option->normal_axis[i][j] = octahedron_normal_axes[i][j];
				 option->feature_dim = option->hog3d_bin_num*option->hog3d_block_size*option->hog3d_block_size;
				 break;
			 case 9: // 3D SURF 9
				 option->clip_frame_num = 2;
				 option->surf3d_bin_num = 6;
				 option->surf3d_block_size = 2;
				 option->surf3d_cell_size = 4;
				 option->feature_dim = option->surf3d_bin_num*option->surf3d_block_size*option->surf3d_block_size;
				 break;
			 case 11: // 2D SIFT global 11
				 option->sift2d_bin_num = 8;
				 option->sift2d_cell_size = 8;
				 option->feature_dim = option->sift2d_bin_num*(width/option->sift2d_cell_size)*(width/option->sift2d_cell_size);
				 break;
			 default: break;
		}
	}
	// set a specific options
	else
	{
		option->type = feature_type;
		option->width = width;
		option->height = height;
		option->subregion_num = subregion_num;
		option->characteristic_method = 0;
		option->normalization_method = parameter0;
		option->clip_frame_num = clip_frame_num;
		option->step_frame_num = step_frame_num;
		switch(feature_type)
		{
			case 0:	// 2D histogram
			{
				option->hist2d_bin_num = parameter1;
				break;
			}
	/*		case 1:	// 2D GIST
			{
				option->gist2d_scale_num = GIST_SCALE_NUM;
				for(i=0; i<option->gist2d_scale_num; i++)
					option->gist2d_orientation_num[i] = GIST_ORIENTATION_NUM[i];
				initialize_GIST_wavelete(option->width, option->height, option->gist2d_scale_num, option->gist2d_orientation_num);
				break;
			}
	*/
			case 2:	// 2D SIFT
			{
				break;
			}
			case 3: // 2D HOG
			{
				option->hog2d_bin_num = parameter1;
				option->hog2d_block_size = parameter2;
				option->hog2d_cell_size = parameter3;
				option->feature_dim = option->hog2d_bin_num*option->hog2d_block_size*option->hog2d_block_size;
				break;
			}
			case 12: // 2D HOG
			{
				option->hog2d_bin_num = parameter1;
				option->hog2d_block_size = parameter2;
				option->hog2d_cell_size = parameter3;
				option->feature_dim = option->hog2d_bin_num*option->hog2d_block_size*option->hog2d_block_size;
				break;
			}
			case 4: // 2D SURF
			{
				if(parameter1!=4)
					printf("bin num of 2D surf should be 4! 4 will be used automatically.\n");
				option->surf2d_bin_num = 4;
				option->surf2d_block_size = parameter2;
				option->surf2d_cell_size = parameter3;
				option->feature_dim = option->surf2d_bin_num*option->surf2d_block_size*option->surf2d_block_size;
				break;
			}
			case 5:	// 3D histogram
			{
				option->hist3d_bin_num = parameter1;
				break;
			}
			case 6:	// 3D GIST
			{
				break;
			}
			case 7:	// 3D SIFT
			{
				break;
			}
			case 8: // 3D HOG
			{
				option->hog3d_bin_num = parameter1;
				option->hog3d_block_size = parameter2;
				option->hog3d_cell_size = parameter3;
				switch(option->hog3d_bin_num)
				{
				case 4:
				{
					for(i=0; i<option->hog3d_bin_num; i++)
						for(j=0; j<3; j++)
							option->normal_axis[i][j] = tetrahedron_normal_axes[i][j];
					break;
				}
				case 6:
				{
					for(i=0; i<option->hog3d_bin_num; i++)
						for(j=0; j<3; j++)
							option->normal_axis[i][j] = hexahedron_normal_axes[i][j];
					break;
				}
				case 8:
				{
					for(i=0; i<option->hog3d_bin_num; i++)
						for(j=0; j<3; j++)
							option->normal_axis[i][j] = octahedron_normal_axes[i][j];
					break;
				}
				case 12:
				{
					for(i=0; i<option->hog3d_bin_num; i++)
						for(j=0; j<3; j++)
							option->normal_axis[i][j] = dodecahedron_normal_axes[i][j];
					break;
				}
				case 20:
				{
					for(i=0; i<option->hog3d_bin_num; i++)
						for(j=0; j<3; j++)
							option->normal_axis[i][j] = icosahedron_normal_axes[i][j];
					break;
				}
				default:
					break;
				} //switch(option->hog3d_bin_num)
				option->feature_dim = option->hog3d_bin_num*option->hog3d_block_size*option->hog3d_block_size;
				break;
			} //case 8: 3D HOG
			case 9: // 3D SURF
			{
				if(parameter1!=6)
					printf("bin num of 3D surf should be 6! 6 will be used automatically.\n");
				option->surf3d_bin_num = 6;
				option->surf3d_block_size = parameter2;
				option->surf3d_cell_size = parameter3;
				option->feature_dim = option->surf3d_bin_num*option->surf3d_block_size*option->surf3d_block_size;
				break;
			}
			case 11: // 2D SIFT global
			{
				option->sift2d_bin_num = parameter1;
				option->sift2d_cell_size = parameter3;
				option->feature_dim = option->sift2d_bin_num*(width/option->sift2d_cell_size)*(width/option->sift2d_cell_size);
				break;
			}
			default:
				break;
		} //switch(feature_type)
	}
}

int save_feature_option(FEATURE_OPTION *option, char *folder_name)
{
	char file_name[200];
	sprintf(file_name, "%s/feature_option.bin", folder_name);
	FILE *file = fopen(file_name, "wb");
	if(file==NULL)
		return -1;
	fwrite(option, sizeof(FEATURE_OPTION), 1, file);
	fclose(file);
	return 0;
}

int load_feature_option(FEATURE_OPTION *option, char *folder_name)
{
	char file_name[200];
	sprintf(file_name, "%s/feature_option.bin", folder_name);
	FILE *file = fopen(file_name, "rb");
	if(file==NULL)
		return -1;
	fread(option, sizeof(FEATURE_OPTION), 1, file);
	fclose(file);
	return 0;
}

/**
Initialize GIST wavelete.
@param image_width image width
@param image_height image height

void initialize_GIST_wavelete(int image_width, int image_height, int scale_num, int *orientation_num)
{
	if(GIST_gabor != NULL)
		image_list_delete(GIST_gabor);
	GIST_gabor = create_gabor(scale_num, orientation_num, image_width, image_height);
}
 */
/**
get feature from image or image set.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *getFeature(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	return feature_list[option.type](feature_num, image_set, width, height, option);
}

/**
calculate 2D histogram of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dhistogram(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	int i, x, y, v;

	int image_num = option.clip_frame_num;
	int x_size = width/option.subregion_num;
	int y_size = height/option.subregion_num;

	// get histogram
	double *feature = new double[option.hist2d_bin_num*option.subregion_num*option.subregion_num*image_num];
	double *feature_ptr = feature;
	unsigned char *image = new unsigned char[x_size*y_size];
	for(i=0; i<image_num; i++)
		for(y=0; y<option.subregion_num; y++)
			for(x=0; x<option.subregion_num; x++)
			{
				for(v=0; v<y_size; v++)
					memcpy(image+v*x_size, image_set[i]+(x*x_size+(y*y_size+v)*width), sizeof(unsigned char)*x_size);

				calRegionHistogram(feature_ptr, option.hist2d_bin_num, image, x_size, y_size);
				feature_ptr += option.hist2d_bin_num;
			}
	delete [] image;

	// normalize
	feature_ptr = feature;
	for(i=0; i<image_num; i++)
		for(y=0; y<option.subregion_num; y++)
			for(x=0; x<option.subregion_num; x++)
			{
				vector_characteristic(feature_ptr, feature_ptr, option.hist2d_bin_num, option.characteristic_method);
				normalize_vector(feature_ptr, option.hist2d_bin_num, option.normalization_method);
				feature_ptr += option.hist2d_bin_num;
			}

	// set to common_feature
	*feature_num = option.subregion_num*option.subregion_num;
	COMMON_FEATURE *common_feature = new COMMON_FEATURE[*feature_num];
	feature_ptr = feature;
	for(i=0; i<*feature_num; i++)
	{
		common_feature[i].feature = new double[option.hist2d_bin_num];
		memcpy(common_feature[i].feature, feature_ptr, sizeof(double)*option.hist2d_bin_num);
		common_feature[i].dim = option.hist2d_bin_num;
		feature_ptr += option.hist2d_bin_num;
	}

	delete [] feature;

	return common_feature;
}

/**
calculate histogram of image.
@param hist histogram vector
@param bin number of bins
@param image image data
@param width image width
@param height image height
@return Returns histogram length.
 */
template <typename T> int calRegionHistogram(double *hist, int bin_num, T *image, int width, int height)
{
	int i;

	//int shift_num = 8-(int)log2(bin_num);

	int shift_num = 0;
	int tmp = bin_num;
	while ( tmp >>= 1 ) ++shift_num;
	shift_num = 8 - shift_num;

	// count bin
	memset(hist, 0, sizeof(int)*bin_num);

	T *image_ptr = image;
	for(i=0; i<width*height; i++)
		hist[*image_ptr++ >> shift_num]++;

	return bin_num;
}

/**
calculate 2D GIST of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dgist(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	/*
	int image_num = option.clip_frame_num;

	// get gist vector
	double *feature = new double[option.subregion_num*option.subregion_num*GIST_gabor->size*image_num];
	int offset = 0;
	for(int i=0; i<image_num; i++)
	{
		image_t *src = Convert2image_t(image_set[i], width, height);
		prefilt(src, 4);
		int feature_dim = gist_gabor(feature+offset, src, option.subregion_num, GIST_gabor);
		image_delete(src);
		offset += feature_dim;
	}

	// normalize
	vector_characteristic(feature, feature, offset, option.characteristic_method);
	normalize_vector(feature, offset, option.normalization_method);

	// set to common_feature
	 *feature_num = 1;
	COMMON_FEATURE *common_feature = new COMMON_FEATURE[*feature_num];
	common_feature->feature = feature;
	common_feature->dim = offset;

	return common_feature;
	 */
	return NULL;
}

void normalize_vector(double *feature, int dim, int normalization_method)
{
	if(normalization_method==0)
		return;

	double norm = 0;
	if(normalization_method==1)
		// L1 norm
		norm = cal_L1norm(feature, dim);
	else if(normalization_method==2)
		// L2 norm
		norm = cal_L2norm(feature, dim);
	if(norm>1e-5)
	{
		for(int i=0; i<dim; i++)
			feature[i] /= norm;
	}
	else
	{
		for(int i=0; i<dim; i++)
			feature[i] = 0.0;
	}
}

double *vector_characteristic(double *feature, int dim, int characteristic_no)
{
	double *feature_new = new double[dim];
	vector_characteristic(feature_new, feature, dim, characteristic_no);

	return feature_new;
}

void vector_characteristic(double *feature_new, double *feature, int dim, int characteristic_no)
{
	if(characteristic_no == 1)
	{
		// characteristic function: y = sqrt(x)
		for(int i=0; i<dim; i++)
			if(feature[i]>0)
				feature_new[i] = fast_sqrt(feature[i]);
			else
				feature_new[i] = -fast_sqrt(-feature[i]);
	}
	else if(characteristic_no == 2)
	{
		// characteristic function: y = x^2
		for(int i=0; i<dim; i++)
			if(feature[i]>0)
				feature_new[i] = feature[i]*feature[i];	
			else
				feature_new[i] = -feature[i]*feature[i];	
	}
	else if(feature_new != feature)
	{
		// characteristic function: y = x
		memcpy(feature_new, feature, sizeof(double)*dim);
	}
}
/*
IplImage *image2IplImage(unsigned char *image, int width, int height, int channel)
{
	IplImage *iplimage = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, channel);
	image2IplImage(iplimage, image, width, height, channel);

	return iplimage;
}

void image2IplImage(IplImage *iplimage, unsigned char *image, int width, int height, int channel)
{
	unsigned char *image_ptr = (unsigned char*)iplimage->imageData;
	int widthStep = width*channel;
	for(int i=0; i<height; i++)
	{
		memcpy(image_ptr, image, sizeof(unsigned char)*widthStep);
		image_ptr += iplimage->widthStep;
		image += widthStep;
	}
}
*/
/*
COMMON_FEATURE *sift2common(struct feature feat)
{
	COMMON_FEATURE *common_feature = new COMMON_FEATURE[1];
	sift2common(common_feature, feat);

	return common_feature;
}

void sift2common(COMMON_FEATURE *common, struct feature feat)
{
	common->feature = new double[feat.d];
	memcpy(common->feature, feat.descr, sizeof(double)*feat.d);
	common->dim = feat.d;
}

struct feature *common2sift(COMMON_FEATURE common)
{
	struct feature *feat = new struct feature[1];
	common2sift(feat, common);

	return feat;
}

void common2sift(struct feature *feat, COMMON_FEATURE common)
{
	memcpy(feat->descr, common.feature, sizeof(double)*common.dim);
	feat->d = common.dim;

	feat->x = common.x;
	feat->y = common.y;
	feat->a = common.a;
	feat->b = common.b;
	feat->c = common.c;
	feat->scl = common.scl;
	feat->ori = common.ori;
	feat->type = common.type;
	feat->category = common.category;
	feat->img_pt.x = common.img_pt.x;
	feat->img_pt.y = common.img_pt.y;
	feat->mdl_pt.x = common.mdl_pt.x;
	feat->mdl_pt.y = common.mdl_pt.y;
}
 */
/**
calculate 2D SIFT of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dsift(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	/*
	int i, j;

	int image_num = option.clip_frame_num;

	// get sift features
	struct feature **feat = new struct feature*[image_num];
	int *num = new int[image_num];
	IplImage *img = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 1);
	for(i=0; i<image_num; i++)
	{
		image2IplImage(img, image_set[i], width, height, 1);
		num[i] = sift_features(img, feat+i);
	}
	cvReleaseImage(&img);
	int total_num = 0;
	for(i=0; i<image_num; i++)
		total_num += num[i];

	// normalize
	for(i=0; i<image_num; i++)
		for(j=0; j<num[i]; j++)
		{
			vector_characteristic(feat[i][j].descr, feat[i][j].descr, feat[i][j].d, option.characteristic_method);
			normalize_vector(feat[i][j].descr, feat[i][j].d, option.normalization_method);
		}

	// set to common_feature
	 *feature_num = total_num;
	COMMON_FEATURE *common_feature = new COMMON_FEATURE[*feature_num];
	int offset = 0;
	for(i=0; i<image_num; i++)
		for(j=0; j<num[i]; j++)
		{
			sift2common(common_feature+offset, feat[i][j]);
			offset++;
		}

	for(i=0; i<image_num; i++)
		free(feat[i]);
	delete [] feat;
	delete [] num;

	return common_feature;
	 */
	return NULL;
}

/**
calculate 2D HOG of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dhog(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	int i, j, k;

	int image_num = option.clip_frame_num;

	double *feature = new double[option.hog2d_bin_num*option.hog2d_block_size*option.hog2d_block_size*option.subregion_num*option.subregion_num*image_num];
	int feature_dim = 0;
	for(k=0; k<image_num; k++)
	{
		unsigned char *image_ptr = image_set[k];

		// gradient
		double *image_dx = new double[width*height];
		double *image_dy = new double[width*height];
		calGradient(image_dx, image_ptr, width, height, false);
		calGradient(image_dy, image_ptr, width, height, true);

		// calculate orientation and amplitude
		int *og = new int[width*height];
		double *am = new double[width*height];
		calBinAmplitude(og, am, image_dx, image_dy, width, height, option.hog2d_bin_num);
		delete [] image_dx;
		delete [] image_dy;

		// if step_size is a multiple of cell_size, skip calculating integral image
		// integral image
		double ***integral_am = new double**[height];
		for(i=0; i<height; i++)
		{
			integral_am[i] = new double*[width];
			for(j=0; j<width; j++)
				integral_am[i][j] = new double[option.hog2d_bin_num];
		}
		IntegralImage_hog(integral_am, og, am, option.hog2d_bin_num, width, height);
		delete [] og;
		delete [] am;

		// HOG feature
		double *feature_ptr = feature+feature_dim;
		feature_dim += calBLOCKfeature(\
				feature_ptr,\
				integral_am, width, height,\
				option.hog2d_bin_num, option.hog2d_cell_size, option.hog2d_block_size, option.subregion_num);
		for(i=0; i<height; i++)
		{
			for(j=0; j<width; j++)
				delete [] integral_am[i][j];
			delete [] integral_am[i];
		}
		delete [] integral_am;

		// feature normalization
		int sub_feature_dim = option.hog2d_bin_num*option.hog2d_block_size*option.hog2d_block_size;
		int sub_feature_num = option.subregion_num*option.subregion_num;
		for(i=0; i<sub_feature_num; i++)
		{
			vector_characteristic(feature_ptr, feature_ptr, sub_feature_dim, option.characteristic_method);
			normalize_vector(feature_ptr, sub_feature_dim, option.normalization_method);
			feature_ptr += sub_feature_dim;
		}
	}

	// set to common_feature
	int dim = option.hog2d_bin_num*option.hog2d_block_size*option.hog2d_block_size;
	*feature_num = option.subregion_num*option.subregion_num*image_num;
	COMMON_FEATURE *common_feature = new COMMON_FEATURE[*feature_num];
	double *feature_ptr = feature;
	for(i=0; i<*feature_num; i++)
	{
		common_feature[i].dim = dim;
		common_feature[i].feature = new double[dim];
		memcpy(common_feature[i].feature, feature_ptr, sizeof(double)*dim);
		feature_ptr += dim;
	}

	delete [] feature;

	return common_feature;
}

COMMON_FEATURE *calFeature_2Dhog_fast(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	int i, k;

	int image_num = option.clip_frame_num;

	double *feature = new double[option.hog2d_bin_num*option.hog2d_block_size*option.hog2d_block_size*option.subregion_num*option.subregion_num*image_num];
	int feature_dim = 0;
	for(k=0; k<image_num; k++)
	{
		unsigned char *image_ptr = image_set[k];

		// gradient
		double *image_dx = new double[width*height];
		double *image_dy = new double[width*height];
		calGradient(image_dx, image_ptr, width, height, false);
		calGradient(image_dy, image_ptr, width, height, true);

		// calculate orientation and amplitude
		int *og = new int[width*height];
		double *am = new double[width*height];
		calBinAmplitude(og, am, image_dx, image_dy, width, height, option.hog2d_bin_num);
		delete [] image_dx;
		delete [] image_dy;

		// HOG feature
		double *feature_ptr = feature+feature_dim;
		feature_dim += calBLOCKfeature(\
				feature_ptr,\
				og, am, width, height,\
				option.hog2d_bin_num, option.hog2d_cell_size, option.hog2d_block_size, option.subregion_num);
		delete [] og;
		delete [] am;

		// feature normalization
		int sub_feature_dim = option.hog2d_bin_num*option.hog2d_block_size*option.hog2d_block_size;
		int sub_feature_num = option.subregion_num*option.subregion_num;
		for(i=0; i<sub_feature_num; i++)
		{
			vector_characteristic(feature_ptr, feature_ptr, sub_feature_dim, option.characteristic_method);
			normalize_vector(feature_ptr, sub_feature_dim, option.normalization_method);
			feature_ptr += sub_feature_dim;
		}
	}

	// set to common_feature
	int dim = option.hog2d_bin_num*option.hog2d_block_size*option.hog2d_block_size;
	*feature_num = option.subregion_num*option.subregion_num*image_num;
	COMMON_FEATURE *common_feature = new COMMON_FEATURE[*feature_num];
	double *feature_ptr = feature;
	for(i=0; i<*feature_num; i++)
	{
		common_feature[i].dim = dim;
		common_feature[i].feature = new double[dim];
		memcpy(common_feature[i].feature, feature_ptr, sizeof(double)*dim);
		feature_ptr += dim;
	}

	delete [] feature;

	return common_feature;
}

/**
calculate 2D SIFT global of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dsift_global(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	option.hog2d_block_size = option.width/option.sift2d_cell_size;
	option.hog2d_cell_size = option.sift2d_cell_size;
	option.hog2d_bin_num = option.sift2d_bin_num;
	option.subregion_num = 1;

	return calFeature_2Dhog(feature_num, image_set, width, height, option);
}

/**
calculate 2D SURF of image
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dsurf(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	int i, j, k;

	int image_num = option.clip_frame_num;

	double *feature = new double[option.surf2d_bin_num*option.surf2d_block_size*option.surf2d_block_size*option.subregion_num*option.subregion_num*image_num];
	int feature_dim = 0;
	for(k=0; k<image_num; k++)
	{
		unsigned char *image_ptr = image_set[k];

		// gradient
		double *image_dx = new double[width*height];
		double *image_dy = new double[width*height];
		calGradient(image_dx, image_ptr, width, height, false);
		calGradient(image_dy, image_ptr, width, height, true);

		// if step_size is a multiple of cell_size, skip calculating integral image
		// integral image
		double ***integral_am = new double**[height];
		for(i=0; i<height; i++)
		{
			integral_am[i] = new double*[width];
			for(j=0; j<width; j++)
				integral_am[i][j] = new double[option.surf2d_bin_num];
		}
		IntegralImage_surf(integral_am, image_dx, image_dy, option.surf2d_bin_num, width, height);
		delete [] image_dx;
		delete [] image_dy;

		// SURF feature
		double *feature_ptr = feature+feature_dim;
		feature_dim += calBLOCKfeature(//
				feature_ptr,//
				integral_am, width, height,//
				option.surf2d_bin_num, option.surf2d_cell_size, option.surf2d_block_size, option.subregion_num);
		for(i=0; i<height; i++)
		{
			for(j=0; j<width; j++)
				delete [] integral_am[i][j];
			delete [] integral_am[i];
		}
		delete [] integral_am;

		// feature normalization
		int sub_feature_dim = option.surf2d_bin_num*option.surf2d_block_size*option.surf2d_block_size;
		int sub_feature_num = option.subregion_num*option.subregion_num;
		for(i=0; i<sub_feature_num; i++)
		{
			vector_characteristic(feature_ptr, feature_ptr, sub_feature_dim, option.characteristic_method);
			normalize_vector(feature_ptr, sub_feature_dim, option.normalization_method);
			feature_ptr += sub_feature_dim;
		}
	}

	// set to common_feature
	int dim = option.surf2d_bin_num*option.surf2d_block_size*option.surf2d_block_size;
	*feature_num = option.subregion_num*option.subregion_num*image_num;
	COMMON_FEATURE *common_feature = new COMMON_FEATURE[*feature_num];
	double *feature_ptr = feature;
	for(i=0; i<*feature_num; i++)
	{
		common_feature[i].dim = dim;
		common_feature[i].feature = new double[dim];
		memcpy(common_feature[i].feature, feature_ptr, sizeof(double)*dim);
		feature_ptr += dim;
	}

	// value check /////////////////////
	//	for(i=0; i<*feature_num; i++)
	//		for(j=0; j<dim; j++)
	//			printf("feature no. %d, feature[%d]=%f\n", i, j, common_feature[i].feature[j]);

	delete [] feature;

	return common_feature;
}

/**
calculate gradient of image.
for non-border pixels, gradient = (I(i+1)-I(i-1))/2.0
for border pixels, gradient = I(i+1)-I(i) or I(i)-I(i-1)
@param grad gradient vector
@param image image data
@param width image width
@param height image height
@param direction_flag gradient direction (0: dx, 1: dy)
 */
template <typename T, typename S>
void calGradient(S *grad, T *image, int width, int height, bool direction_flag)
{
	int i, j;

	T *image_ptr, *image_ptr1, *image_ptr2;
	S *grad_ptr, *grad_ptr1, *grad_ptr2;
	if(direction_flag)
	{
		// grad_y
		image_ptr = image + width;
		grad_ptr = grad + width;

		/*
		for(i=1; i<height-1; i++)
			for(j=0; j<width; j++)
			{
				*grad_ptr++ = (*(image_ptr+width)-*(image_ptr-width))/2.0;
				image_ptr++;
			}
		*/
		int sz = (height-2) * width;
		for ( i=0 ; i<sz ; i += 2 )
		{
			*grad_ptr++ = (*(image_ptr+width)-*(image_ptr-width)) * 0.5;
			image_ptr++;

			*grad_ptr++ = (*(image_ptr+width)-*(image_ptr-width)) * 0.5;
			image_ptr++;
		}
		if ( sz & 0x1 )
		{
			*grad_ptr++ = (*(image_ptr+width)-*(image_ptr-width)) * 0.5;
			image_ptr++;
		}

		int offset = width*(height-1);
		image_ptr1 = image;
		image_ptr2 = image + offset;
		grad_ptr1 = grad;
		grad_ptr2 = grad + offset;
		for(j=0; j<width; j++)
		{
			*grad_ptr1++ = *(image_ptr1+width)-*(image_ptr1);
			*grad_ptr2++ = *(image_ptr2)-*(image_ptr2-width);
			image_ptr1++;
			image_ptr2++;
		}
	}
	else
	{
		// grad_x
		for(j=1; j<width-1; j++)
		{
			image_ptr = image + j;
			grad_ptr = grad + j;
			for(i=0; i<height; i++)
			{
				*grad_ptr = (*(image_ptr+1)-*(image_ptr-1)) * 0.5;
				grad_ptr += width;
				image_ptr += width;
			}
		}
		// grad_x
		image_ptr1 = image;
		image_ptr2 = image + width-1;
		grad_ptr1 = grad;
		grad_ptr2 = grad + width-1;
		for(i=0; i<height; i++)
		{
			*grad_ptr1 = *(image_ptr1+1)-*(image_ptr1);
			*grad_ptr2 = *(image_ptr2)-*(image_ptr2-1);
			grad_ptr1 += width;
			grad_ptr2 += width;
			image_ptr1 += width;
			image_ptr2 += width;
		}
	}
}

/**
calculate bin no. and amplitude from gradient.
@param og vector of bin no.
@param am vector of gradient amplitude
@param grad_x gradient dx
@param grad_y gradient dy
@param width image width
@param height image height
@param bin_num number of bin
 */
template <typename T1, typename T2, typename S>
void calBinAmplitude(T1 *og, T2 *am, S *grad_x, S *grad_y, int width, int height, int bin_num)
{
	int i;//, j;
	double theta;
	double bin_angle = M_PI/bin_num;
	//bin_angle *= 2.0;
	S *grad_x_ptr = grad_x;
	S *grad_y_ptr = grad_y;
	T1 *og_ptr = og;
	T2 *am_ptr = am;

	int sz = height * width;
	for(i=0; i<sz; i++)
	{
		if((*grad_x_ptr)!=0)
		{
			theta = atan((double)(*grad_y_ptr)/(*grad_x_ptr));
			*og_ptr = (int)((theta+M_PI_2)/bin_angle);

			//*am_ptr = sqrt((*grad_x_ptr)*(*grad_x_ptr) + (*grad_y_ptr)*(*grad_y_ptr));
			*am_ptr = fast_sqrt((*grad_x_ptr)*(*grad_x_ptr) + (*grad_y_ptr)*(*grad_y_ptr));
		}
		else
		{
			*og_ptr = 0;
			if((*grad_y_ptr)>=0)
				*am_ptr = (*grad_y_ptr);
			else
				*am_ptr = -(*grad_y_ptr);
		}
		if(*am_ptr < AMP_THRESHOLD)
			*am_ptr = 0;
		og_ptr++;
		am_ptr++;
		grad_x_ptr++;
		grad_y_ptr++;

/*			theta = atan2(dy,dx);
			if(theta < 0) theta += M_PI;
			*og_ptr++ = (int)(theta/bin_angle);
			*am_ptr++ = sqrt((*grad_x_ptr)*(*grad_x_ptr) + (*grad_y_ptr)*(*grad_y_ptr));
			grad_x_ptr++;
			grad_y_ptr++;
*/
	}
}

/**
calculate integral amplitude for bin from orientation bin and amplitude.
@param integral_am integral amplitude
@param og vector of bin no.
@param am vector of gradient amplitude
@param bin_num number of bin
@param width image width
@param height image height
 */
template <typename DST_, typename T1, typename T2>
void IntegralImage_hog(DST_ ***integral_am, T1 *og, T2 *am, int bin_num, int width, int height)
{
	int i, j, k;

	DST_ *row_sum = new DST_[bin_num];

	int offset = 0;
	DST_ **integral1 = integral_am[0];
	memset(row_sum, 0, sizeof(DST_)*bin_num);
	for(j=0; j<width; j++)
	{
		DST_ *integral2 = integral1[j];
		row_sum[og[offset]] = row_sum[og[offset]] + am[offset];
		offset++;
		for(k=0; k<bin_num; k++)
			integral2[k] = row_sum[k];
	}
	for(i=1; i<height; i++)
	{
		DST_ **integral1 = integral_am[i];
		DST_ **integral2 = integral_am[i-1];
		memset(row_sum, 0, sizeof(DST_)*bin_num);
		for(j=0; j<width; j++)
		{
			DST_ *integral11 = integral1[j];
			DST_ *integral22 = integral2[j];
			row_sum[og[offset]] = row_sum[og[offset]] + am[offset];
			offset++;
			for(k=0; k<bin_num; k++)
				integral11[k] = integral22[k] + row_sum[k];
		}
	}

	delete [] row_sum;
}

/**
calculate integral amplitude for bin from gradients.
@param integral_am integral amplitude
@param dx gradient x
@param dy gradient y
@param bin_num number of bin
@param width image width
@param height image height
 */
template <typename DST_, typename T>
void IntegralImage_surf(DST_ ***integral_am, T *dx, T *dy, int bin_num, int width, int height)
{
	int i, j, k;

	DST_ *row_sum = new DST_[bin_num];

	int offset = 0;
	DST_ **integral1 = integral_am[0];
	memset(row_sum, 0, sizeof(DST_)*bin_num);
	for(j=0; j<width; j++)
	{
		DST_ *integral2 = integral1[j];
		if(dx[offset] > AMP_THRESHOLD)
		{
			row_sum[0] = row_sum[0] + dx[offset];
			row_sum[2] = row_sum[2] + dx[offset];
		}
		else if(dx[offset] < -AMP_THRESHOLD)
		{
			//row_sum[0] = row_sum[0] + dx[offset];
			row_sum[2] = row_sum[2] - dx[offset];
		}
		if(dy[offset] > AMP_THRESHOLD)
		{
			row_sum[1] = row_sum[1] + dy[offset];
			row_sum[3] = row_sum[3] + dy[offset];
		}
		else if(dy[offset] < -AMP_THRESHOLD)
		{
			//row_sum[1] = row_sum[1] + dy[offset];
			row_sum[3] = row_sum[3] - dy[offset];
		}
		offset++;
		for(k=0; k<bin_num; k++)
			integral2[k] = row_sum[k];
	}
	for(i=1; i<height; i++)
	{
		DST_ **integral1 = integral_am[i];
		DST_ **integral2 = integral_am[i-1];
		memset(row_sum, 0, sizeof(DST_)*bin_num);
		for(j=0; j<width; j++)
		{
			DST_ *integral11 = integral1[j];
			DST_ *integral22 = integral2[j];
			if(dx[offset] > AMP_THRESHOLD)
			{
				row_sum[0] = row_sum[0] + dx[offset];
				row_sum[2] = row_sum[2] + dx[offset];
			}
			else if(dx[offset] < -AMP_THRESHOLD)
			{
				//row_sum[0] = row_sum[0] + dx[offset];
				row_sum[2] = row_sum[2] - dx[offset];
			}
			if(dy[offset] > AMP_THRESHOLD)
			{
				row_sum[1] = row_sum[1] + dy[offset];
				row_sum[3] = row_sum[3] + dy[offset];
			}
			else if(dy[offset] < -AMP_THRESHOLD)
			{
				//row_sum[1] = row_sum[1] + dy[offset];
				row_sum[3] = row_sum[3] - dy[offset];
			}
			offset++;
			for(k=0; k<bin_num; k++)
				integral11[k] = integral22[k] + row_sum[k];
		}
	}

	delete [] row_sum;
}

/**
calculate bin amplitude summations of cells.
@param cell_sum bin amplitude summations of cells
@param integral_am integral amplitude
@param cell_size cell size in pixel
@param cell_num_h horizontal number of cells
@param cell_num_v vertical number of cells
@param bin_num number of bin
 */
template <typename T> void calCellSum(T ***cell_sum, T ***integral_am, int cell_size, int cell_num_h, int cell_num_v, int bin_num)
{
	int cv, ch, ori;
	int y = -1;
	for(cv=0; cv<cell_num_v; cv++)
	{
		int x = -1;
		for(ch=0; ch<cell_num_h; ch++)
		{
			T *cs1 = cell_sum[cv][ch];
			T *iam4 = integral_am[y+cell_size][x+cell_size];
			if(x>=0 && y>=0)
			{
				T *iam1 = integral_am[y][x];
				T *iam2 = integral_am[y+cell_size][x];
				T *iam3 = integral_am[y][x+cell_size];
				for(ori=0; ori<bin_num; ori++)
				{
					cs1[ori] = iam1[ori]-iam2[ori]-iam3[ori]+iam4[ori];
				}
			}
			else if(x<0 && y>=0)
			{
				T *iam3 = integral_am[y][x+cell_size];
				for(ori=0; ori<bin_num; ori++)
				{
					cs1[ori] =-iam3[ori]+iam4[ori];
				}
			}
			else if(x>=0 && y<0)
			{
				T *iam2 = integral_am[y+cell_size][x];
				for(ori=0; ori<bin_num; ori++)
				{
					cs1[ori] =-iam2[ori]+iam4[ori];
				}
			}
			else
			{
				for(ori=0; ori<bin_num; ori++)
				{
					cs1[ori] = iam4[ori];
				}
			}
			x += cell_size;
		}
		y += cell_size;
	}
}

/**
calculate BLOCK feature from integral amplitude.
@param feature BLOCK feature vector
@param integral_am integral amplitude
@param width image width
@param height image height
@param bin_num number of bin
@param cell_size cell size in pixel
@param block_Size block size in cell
@param pattern_num # of patterns in a row
@return Returns feature dimension.
 */
template <typename T> int calBLOCKfeature(//
		double *feature,//
		T ***integral_am, int width, int height,//
		int bin_num, int cell_size, int block_size, int pattern_num)
{
	int bh, bv, ch, cv, ori;

	// # of cells and blocks
	int cell_num_h = width/cell_size;
	int cell_num_v = height/cell_size;
	int block_num_h = cell_num_h - block_size + 1;
	int block_num_v = cell_num_v - block_size + 1;
	int step_num_h = block_num_h;
	int step_num_v = block_num_v;
	if(pattern_num>1)
	{
		step_num_h = (block_num_h-1)/(pattern_num-1);
		step_num_v = (block_num_v-1)/(pattern_num-1);
	}

	// cell summation
	T ***cell_sum = new T**[cell_num_v];
	for(cv=0; cv<cell_num_v; cv++)
	{
		cell_sum[cv] = new T*[cell_num_h];
		for(ch=0; ch<cell_num_h; ch++)
			cell_sum[cv][ch] = new T[bin_num];
	}
	calCellSum(cell_sum, integral_am, cell_size, cell_num_h, cell_num_v, bin_num);

	// concatenating block features
	int feature_dim = bin_num*block_size*block_size*pattern_num*pattern_num;
	double *feature_ptr = feature;
	for(bv=0; bv<block_num_v; bv+=step_num_v)
		for(bh=0; bh<block_num_h; bh+=step_num_h)
			// block features
			for(cv=0; cv<block_size; cv++)
				for(ch=0; ch<block_size; ch++)
				{
					T *val = cell_sum[cv+bv][ch+bh];
					for(ori=0; ori<bin_num; ori++)
						*feature_ptr++ = *val++;
				}

	for(cv=0; cv<cell_num_v; cv++)
	{
		for(ch=0; ch<cell_num_h; ch++)
			delete [] cell_sum[cv][ch];
		delete [] cell_sum[cv];
	}
	delete [] cell_sum;

	return feature_dim;
}

template <typename T> int calBLOCKfeature(//
		double *feature,//
		int *og, T *am, int width, int height,//
		int bin_num, int cell_size, int block_size, int pattern_num)
{
	int bh, bv, ch, cv, i, j;

	// # of cells and blocks
	int cell_num_h = width/cell_size;	// # of cells
	int cell_num_v = height/cell_size;	// # of cells
	int block_num_h = cell_num_h - block_size + 1;	// # of blocks
	int block_num_v = cell_num_v - block_size + 1;	// # of blocks
	int step_num_h = block_num_h;	// cell step
	int step_num_v = block_num_v;	// cell step
	if(pattern_num>1)
	{
		step_num_h = (block_num_h-1)/(pattern_num-1);
		step_num_v = (block_num_v-1)/(pattern_num-1);
	}

	// make og_ptr and am_ptr
	int **og_ptr = new int*[height];
	T **am_ptr = new T*[height];
	og_ptr[0] = og;
	am_ptr[0] = am;
	for(int i=1; i<height; i++)
	{
		og_ptr[i] = og_ptr[i-1] + width;
		am_ptr[i] = am_ptr[i-1] + width;
	}

	// concatenating block features
	int feature_dim = bin_num*block_size*block_size*pattern_num*pattern_num;
	memset(feature, 0, sizeof(double)*feature_dim);
	double *feature_ptr = feature;
	int step_num_v_pixel = step_num_v * cell_size;
	int step_num_h_pixel = step_num_h * cell_size;
	int offset_x, offset_y, offset_xx, offset_yy;
	offset_y = 0;
	for(bv=0; bv<block_num_v; bv+=step_num_v)
	{
		offset_x = 0;
		for(bh=0; bh<block_num_h; bh+=step_num_h)
		{
			// block features
			offset_yy = offset_y;
			for(cv=0; cv<block_size; cv++)
			{
				offset_xx = offset_x;
				for(ch=0; ch<block_size; ch++)
				{
					for(i=offset_yy; i<offset_yy+cell_size; i++)
						for(j=offset_xx; j<offset_xx+cell_size; j++)
							feature_ptr[og_ptr[i][j]] += am_ptr[i][j];

					feature_ptr = feature_ptr + bin_num;

					offset_xx += cell_size;
				}
				offset_yy += cell_size;
			}
			offset_x += step_num_h_pixel;
		}
		offset_y += step_num_v_pixel;
	}

	delete [] og_ptr;
	delete [] am_ptr;

	return feature_dim;
}

/**
calculate 2D mean, std of image
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dmeanvar(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	int i, j, x, y;
	int region_num = option.subregion_num*option.subregion_num;

	int *sum = new int[region_num];
	int *square_sum = new int[region_num];
	int region_index = 0;

	int subregion_width = width/option.subregion_num;
	int subregion_height = height/option.subregion_num;	
	int index_y = 0;
	for(i=0; i<option.subregion_num; i++)
	{
		int index_x = 0;
		for(j=0; j<option.subregion_num; j++)
		{
			for(y=index_y; y<index_y+subregion_height; y++)
				for(x=index_x; x<index_x+subregion_width; x++)
				{
					sum[region_index] = sum[region_index] + image_set[y][x];
					square_sum[region_index] = square_sum[region_index] + (int)image_set[y][x]*image_set[y][x];
				}
			region_index++;

			index_x += subregion_width;
		}
		index_y += subregion_height;
	}

	int denom = subregion_width*subregion_height;
	for(i=0; i<region_num; i++)
	{
		sum[i] = sum[i]/denom;
		square_sum[i] = square_sum[i]/denom - sum[i]*sum[i];
	}

	// set to common_feature
	*feature_num = region_num;
	COMMON_FEATURE *common_feature = new COMMON_FEATURE[*feature_num];
	for(i=0; i<*feature_num; i++)
	{
		common_feature[i].feature = new double[2];
		common_feature[i].feature[0] = sum[i];
		common_feature[i].feature[1] = square_sum[i];
		common_feature[i].dim = 2;
	}

	delete [] sum;
	delete [] square_sum;

	return common_feature;
}


/**
calculate 3D histogram of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_3Dhistogram(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	int i, k, x, y, v;

	int image_num = option.clip_frame_num;
	int x_size = width/option.subregion_num;
	int y_size = height/option.subregion_num;

	// count bin
	double *feature = new double[option.hist3d_bin_num*option.subregion_num*option.subregion_num];
	double *feature_ptr = feature;
	unsigned char *image = new unsigned char[x_size*y_size];
	for(y=0; y<option.subregion_num; y++)
		for(x=0; x<option.subregion_num; x++)
		{
			for(v=0; v<y_size; v++)
				memcpy(image+v*x_size, image_set[0]+(x*x_size+(y*y_size+v)*width), sizeof(unsigned char)*x_size);
			int feature_dim = calRegionHistogram(feature_ptr, option.hist3d_bin_num,//
					image, x_size, y_size);
			double *temp_feature = new double[feature_dim];
			for(k=1; k<image_num; k++)
			{
				for(v=0; v<y_size; v++)
					memcpy(image+v*x_size, image_set[k]+(x*x_size+(y*y_size+v)*width), sizeof(unsigned char)*x_size);
				calRegionHistogram(temp_feature, feature_dim,//
						image, x_size, y_size);
				for(i=0; i<feature_dim; i++)
					feature_ptr[i] += temp_feature[i];
			}
			delete [] temp_feature;

			feature_ptr += option.hist3d_bin_num;
		}
	delete [] image;

	// normalize
	feature_ptr = feature;
	for(y=0; y<option.subregion_num; y++)
		for(x=0; x<option.subregion_num; x++)
		{
			vector_characteristic(feature_ptr, feature_ptr, option.hist3d_bin_num, option.characteristic_method);
			normalize_vector(feature_ptr, option.hist3d_bin_num, option.normalization_method);
			feature_ptr += option.hist3d_bin_num;
		}

	// set to common_feature
	*feature_num = option.subregion_num*option.subregion_num;
	COMMON_FEATURE *common_feature = new COMMON_FEATURE[*feature_num];
	feature_ptr = feature;
	for(i=0; i<*feature_num; i++)
	{
		common_feature[i].feature = new double[option.hist3d_bin_num];
		memcpy(common_feature[i].feature, feature_ptr, sizeof(double)*option.hist3d_bin_num);
		common_feature[i].dim = option.hist3d_bin_num;
		feature_ptr += option.hist3d_bin_num;
	}

	delete [] feature;

	return common_feature;
}

/**
calculate 3D GIST of image. (not implemented)
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_3Dgist(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{


	return 0;
}

/**
calculate 3D SIFT of image. (not implemented)
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_3Dsift(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{


	return 0;
}

/**
calculate 3D HOG of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_3Dhog(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	int i, j, k;

	int image_num = option.clip_frame_num;

	// calculate gradient: x, y, t
	double **grad_x = new double*[image_num];
	double **grad_y = new double*[image_num];
	double **grad_t = new double*[image_num];
	for(k=0; k<image_num; k++)
	{
		grad_x[k] = new double[width*height];
		grad_y[k] = new double[width*height];
		grad_t[k] = new double[width*height];
	}

	if(image_num>1)
	{
		calGradient(grad_x[0], image_set[0], width, height, false);
		calGradient(grad_y[0], image_set[0], width, height, true);
		calGradient_t(grad_t[0], image_set[0], image_set[1], width, height, 1.0);
		for(k=1; k<image_num-1; k++)
		{
			calGradient(grad_x[k], image_set[k], width, height, false);
			calGradient(grad_y[k], image_set[k], width, height, true);
			calGradient_t(grad_t[k], image_set[k-1], image_set[k+1], width, height, 2.0);
		}
		calGradient(grad_x[image_num-1], image_set[image_num-1], width, height, false);
		calGradient(grad_y[image_num-1], image_set[image_num-1], width, height, true);
		calGradient_t(grad_t[image_num-1], image_set[image_num-2], image_set[image_num-1], width, height, 1.0);
	}
	else
	{
		calGradient(grad_x[0], image_set[0], width, height, false);
		calGradient(grad_y[0], image_set[0], width, height, true);
		memset(grad_t[0], 0, sizeof(double)*width*height);
	}

	// calculate orientation bin and amplitude
	int **og = new int*[image_num];
	double **am = new double*[image_num];
	for(k=0; k<image_num; k++)
	{
		og[k] = new int[width*height];
		am[k] = new double[width*height];
	}
	calBinAmplitude3D(og, am, grad_x, grad_y, grad_t, width, height, image_num, option.hog3d_bin_num, option.normal_axis);
	for(k=0; k<image_num; k++)
	{
		delete [] grad_x[k];
		delete [] grad_y[k];
		delete [] grad_t[k];
	}
	delete [] grad_x;
	delete [] grad_y;
	delete [] grad_t;

	// calculate integral amplitude 3D
	double ****integral_am = new double***[image_num];
	for(k=0; k<image_num; k++)
	{
		integral_am[k] = new double**[height];
		for(i=0; i<height; i++)
		{
			integral_am[k][i] = new double*[width];
			for(j=0; j<width; j++)
				integral_am[k][i][j] = new double[option.hog3d_bin_num];
		}
	}
	IntegralImage3D_hog(integral_am, og, am, option.hog3d_bin_num, width, height, image_num);
	for(k=0; k<image_num; k++)
	{
		delete og[k];
		delete am[k];
	}
	delete [] og;
	delete [] am;

	// HOG 3D feature
	double *feature = new double[option.hog3d_bin_num*option.hog3d_block_size*option.hog3d_block_size*option.subregion_num*option.subregion_num];
	calBLOCK3Dfeature(//
			feature,//
			integral_am, width, height, image_num,//
			option.hog3d_bin_num, option.hog3d_cell_size, option.hog3d_block_size, option.subregion_num);
	for(k=0; k<image_num; k++)
	{
		for(i=0; i<height; i++)
		{
			for(j=0; j<width; j++)
				delete [] integral_am[k][i][j];
			delete [] integral_am[k][i];
		}
		delete [] integral_am[k];
	}
	delete [] integral_am;

	// feature normalization
	int sub_feature_dim = option.hog3d_bin_num*option.hog3d_block_size*option.hog3d_block_size;
	int sub_feature_num = option.subregion_num*option.subregion_num;
	double *feature_ptr = feature;
	for(i=0; i<sub_feature_num; i++)
	{
		vector_characteristic(feature_ptr, feature_ptr, sub_feature_dim, option.characteristic_method);
		normalize_vector(feature_ptr, sub_feature_dim, option.normalization_method);
		feature_ptr += sub_feature_dim;
	}

	// set to common_feature
	int dim = sub_feature_dim;
	*feature_num = sub_feature_num;
	COMMON_FEATURE *common_feature = new COMMON_FEATURE[*feature_num];
	feature_ptr = feature;
	for(i=0; i<*feature_num; i++)
	{
		common_feature[i].dim = dim;
		common_feature[i].feature = new double[dim];
		memcpy(common_feature[i].feature, feature_ptr, sizeof(double)*dim);
		feature_ptr += dim;
	}

	delete [] feature;

	return common_feature;
}

/**
calculate 3D SURF of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_3Dsurf(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option)
{
	int i, j, k;

	int image_num = option.clip_frame_num;

	// calculate gradient: x, y, t
	double **grad_x = new double*[image_num];
	double **grad_y = new double*[image_num];
	double **grad_t = new double*[image_num];
	for(k=0; k<image_num; k++)
	{
		grad_x[k] = new double[width*height];
		grad_y[k] = new double[width*height];
		grad_t[k] = new double[width*height];
	}

	if(image_num>1)
	{
		calGradient(grad_x[0], image_set[0], width, height, false);
		calGradient(grad_y[0], image_set[0], width, height, true);
		calGradient_t(grad_t[0], image_set[0], image_set[1], width, height, 1.0);
		for(k=1; k<image_num-1; k++)
		{
			calGradient(grad_x[k], image_set[k], width, height, false);
			calGradient(grad_y[k], image_set[k], width, height, true);
			calGradient_t(grad_t[k], image_set[k-1], image_set[k+1], width, height, 2.0);
		}
		calGradient(grad_x[image_num-1], image_set[image_num-1], width, height, false);
		calGradient(grad_y[image_num-1], image_set[image_num-1], width, height, true);
		calGradient_t(grad_t[image_num-1], image_set[image_num-2], image_set[image_num-1], width, height, 1.0);
	}
	else
	{
		calGradient(grad_x[0], image_set[0], width, height, false);
		calGradient(grad_y[0], image_set[0], width, height, true);
		memset(grad_t[0], 0, sizeof(double)*width*height);
	}

	// calculate integral amplitude 3D
	double ****integral_am = new double***[image_num];
	for(k=0; k<image_num; k++)
	{
		integral_am[k] = new double**[height];
		for(i=0; i<height; i++)
		{
			integral_am[k][i] = new double*[width];
			for(j=0; j<width; j++)
				integral_am[k][i][j] = new double[option.surf3d_bin_num];
		}
	}
	IntegralImage3D_surf(integral_am, grad_x, grad_y, grad_t, option.surf3d_bin_num, width, height, image_num);
	for(k=0; k<image_num; k++)
	{
		delete [] grad_x[k];
		delete [] grad_y[k];
		delete [] grad_t[k];
	}
	delete [] grad_x;
	delete [] grad_y;
	delete [] grad_t;

	// SURF 3D feature
	double *feature = new double[option.surf3d_bin_num*option.surf3d_block_size*option.surf3d_block_size*option.subregion_num*option.subregion_num];
	calBLOCK3Dfeature(//
			feature,//
			integral_am, width, height, image_num,//
			option.surf3d_bin_num, option.surf3d_cell_size, option.surf3d_block_size, option.subregion_num);
	for(k=0; k<image_num; k++)
	{
		for(i=0; i<height; i++)
		{
			for(j=0; j<width; j++)
				delete [] integral_am[k][i][j];
			delete [] integral_am[k][i];
		}
		delete [] integral_am[k];
	}
	delete [] integral_am;

	// feature normalization
	int sub_feature_dim = option.surf3d_bin_num*option.surf3d_block_size*option.surf3d_block_size;
	int sub_feature_num = option.subregion_num*option.subregion_num;
	double *feature_ptr = feature;
	for(i=0; i<sub_feature_num; i++)
	{		
		vector_characteristic(feature_ptr, feature_ptr, sub_feature_dim, option.characteristic_method);
		normalize_vector(feature_ptr, sub_feature_dim, option.normalization_method);
		feature_ptr += sub_feature_dim;
	}

	// set to common_feature
	int dim = sub_feature_dim;
	*feature_num = sub_feature_num;
	COMMON_FEATURE *common_feature = new COMMON_FEATURE[*feature_num];
	feature_ptr = feature;
	for(i=0; i<*feature_num; i++)
	{
		common_feature[i].dim = dim;
		common_feature[i].feature = new double[dim];
		memcpy(common_feature[i].feature, feature_ptr, sizeof(double)*dim);
		feature_ptr += dim;
	}

	delete [] feature;

	return common_feature;
}

/**
calculate gradient of image along time direction. grad = (image2 - image1)/denorm.
@param grad gradient along time direction
@param image1 previous image data
@param image2 present image data
@param width image width
@param height image height
@param denorm unit time
 */
template <typename T, typename S>
void calGradient_t(T *grad, S *image1, S *image2, int width, int height, T denorm)
{
	int i, j;

	S *image1_ptr = image1;
	S *image2_ptr = image2;
	T *grad_ptr = grad;
	for(i=0; i<height; i++)
		for(j=0; j<width; j++)
			*grad_ptr++ = (*image2_ptr++ - *image1_ptr++)/denorm;
}

/**
calculate 3D bin and amplitude from gradients.
@param og bin no.
@param am amplitude
@param grad_x gradient dx
@param grad_y gradient dy
@param grad_t gradient dt
@param width image width
@param height image height
@param image_num number of images
@param bin_num number of bins
 */
template <typename T1, typename T2, typename S>
void calBinAmplitude3D(T1 **og, T2 **am, S **grad_x, S **grad_y, S **grad_t, int width, int height, int image_num, int bin_num, double normal_axes[20][3])
{
	int i, j;
	for(int k=0; k<image_num; k++)
	{
		T1 *og_ptr = og[k];
		T2 *am_ptr = am[k];
		S *grad_x_ptr = grad_x[k];
		S *grad_y_ptr = grad_y[k];
		S *grad_t_ptr = grad_t[k];
		for(i=0; i<height; i++)
			for(j=0; j<width; j++)
			{
				*am_ptr = sqrt(*grad_x_ptr * *grad_x_ptr + *grad_y_ptr * *grad_y_ptr + *grad_t_ptr * *grad_t_ptr);
				if(*am_ptr < AMP_THRESHOLD)
				{
					*am_ptr = 0;
					*og_ptr = 0;
				}
				else
					*og_ptr = getBin3D(*grad_x_ptr, *grad_y_ptr, *grad_t_ptr, bin_num, normal_axes);
				am_ptr++;
				og_ptr++;
				grad_x_ptr++;
				grad_y_ptr++;
				grad_t_ptr++;
			}
	}
}

/**
calculate 3D bin.
@param dx gradient dx
@param dy gradient dy
@param dt gradient dt
@param bin_num number of bins
@return Returns bin no.
 */
template <typename T> int getBin3D(T dx, T dy, T dt, int bin_num, double normal_axes[20][3])
{
	// (dx, dy, dt) vs. (a, b, c) of ax + by + ct + d = 0
	// how to define normal vector (a, b, c) ?
	// make the normals to have the same angle to each other. <ni, ni> = 1 for all i and <ni, nj> = constant for all i, j (!= i).
	// icosahedron (20 faces): n = (+-1, +-1, +-1), (0, +-1/PHI, +-PHI), (+-1/PHI, +-PHI, 0), (+-PHI, 0, +-1/PHI), PHI = (1+sqrt(5))/2
	// Bin index is i of ni which has the minimum angle to (dx, dy, dt).

	int i;
	double val;

	int bin_index = 0;
	double max_cos = dx*normal_axes[0][0] + dy*normal_axes[0][1] + dt*normal_axes[0][2];
	for(i=1; i<bin_num; i++)
	{
		val = dx*normal_axes[i][0] + dy*normal_axes[i][1] + dt*normal_axes[i][2];
		if(val > max_cos)
		{
			bin_index = i;
			max_cos = val;
		}
	}

	return bin_index;
}

/**
calculate 3D integral amplitude from 3D bin no. and amplitude.
@param integral_am 3D integral amplitude
@param og 3D bin no.
@param am 3D gradient amplitude
@param bin_num number of bins
@param width image width
@param height image height
@param image_num number of images
 */
template <typename DST_, typename T1, typename T2>
void IntegralImage3D_hog(DST_ ****integral_am, T1 **og, T2 **am, int bin_num, int width, int height, int image_num)
{
	// need to be optimized
	int i, j, k, t;

	// 2D integral images
	DST_ *row_sum = new DST_[bin_num];
	for(t=0; t<image_num; t++)
	{
		int offset = 0;
		DST_ ***integral_t = integral_am[t];
		DST_ **integral1 = integral_t[0];
		T1 *og_ptr = og[t];
		T2 *am_ptr = am[t];
		memset(row_sum, 0, sizeof(DST_)*bin_num);
		for(j=0; j<width; j++)
		{
			DST_ *integral2 = integral1[j];
			row_sum[og_ptr[offset]] = row_sum[og_ptr[offset]] + am_ptr[offset];
			offset++;
			for(k=0; k<bin_num; k++)
				integral2[k] = row_sum[k];
		}
		for(i=1; i<height; i++)
		{
			DST_ **integral1 = integral_t[i];
			DST_ **integral2 = integral_t[i-1];
			memset(row_sum, 0, sizeof(DST_)*bin_num);
			for(j=0; j<width; j++)
			{
				DST_ *integral11 = integral1[j];
				DST_ *integral22 = integral2[j];
				row_sum[og_ptr[offset]] = row_sum[og_ptr[offset]] + am_ptr[offset];
				offset++;
				for(k=0; k<bin_num; k++)
					integral11[k] = integral22[k] + row_sum[k];
			}
		}
	}

	// accumulated sum along to time line
	for(t=1; t<image_num; t++)
	{
		DST_ ***integral1 = integral_am[t];
		DST_ ***integral2 = integral_am[t-1];
		for(i=0; i<height; i++)
		{
			DST_ **integral11 = *integral1++;
			DST_ **integral22 = *integral2++;
			for(j=0; j<width; j++)
			{
				DST_ *integral111 = *integral11++;
				DST_ *integral222 = *integral22++;
				for(k=0; k<bin_num; k++)
					*integral111++ += *integral222++;
			}
		}
	}

	delete [] row_sum;
}

/**
calculate 3D integral amplitude from 3D gradients.
@param integral_am 3D integral amplitude
@param grad_x gradient_x
@param grad_y gradient_y
@param grad_t gradient_t
@param width image width
@param height image height
@param image_num number of images
 */
template <typename DST_, typename T>
void IntegralImage3D_surf(DST_ ****integral_am, T **grad_x, T **grad_y, T **grad_t, int bin_num, int width, int height, int image_num)
{
	// need to be optimized
	int i, j, k, t;

	// 2D integral images
	DST_ *row_sum = new DST_[bin_num];
	for(t=0; t<image_num; t++)
	{
		int offset = 0;
		DST_ ***integral_t = integral_am[t];
		DST_ **integral1 = integral_t[0];
		T *am_ptr_x = grad_x[t];
		T *am_ptr_y = grad_y[t];
		T *am_ptr_t = grad_t[t];
		memset(row_sum, 0, sizeof(DST_)*bin_num);
		for(j=0; j<width; j++)
		{
			DST_ *integral2 = integral1[j];
			if(am_ptr_x[offset]>0)
			{
				row_sum[0] = row_sum[0] + am_ptr_x[offset];
				row_sum[3] = row_sum[3] + am_ptr_x[offset];
			}
			else
			{
				//row_sum[0] = row_sum[0] + am_ptr_x[offset];
				row_sum[3] = row_sum[3] - am_ptr_x[offset];
			}
			if(am_ptr_y[offset]>0)
			{
				row_sum[1] = row_sum[1] + am_ptr_y[offset];
				row_sum[4] = row_sum[4] + am_ptr_y[offset];
			}
			else
			{
				//row_sum[1] = row_sum[1] + am_ptr_y[offset];
				row_sum[4] = row_sum[4] - am_ptr_y[offset];
			}
			if(am_ptr_t[offset]>0)
			{
				row_sum[2] = row_sum[2] + am_ptr_t[offset];
				row_sum[5] = row_sum[5] + am_ptr_t[offset];
			}
			else
			{
				//row_sum[2] = row_sum[2] + am_ptr_t[offset];
				row_sum[5] = row_sum[5] - am_ptr_t[offset];
			}
			offset++;
			for(k=0; k<bin_num; k++)
				integral2[k] = row_sum[k];
		}
		for(i=1; i<height; i++)
		{
			DST_ **integral1 = integral_t[i];
			DST_ **integral2 = integral_t[i-1];
			memset(row_sum, 0, sizeof(DST_)*bin_num);
			for(j=0; j<width; j++)
			{
				DST_ *integral11 = integral1[j];
				DST_ *integral22 = integral2[j];
				if(am_ptr_x[offset]>0)
				{
					row_sum[0] = row_sum[0] + am_ptr_x[offset];
					row_sum[3] = row_sum[3] + am_ptr_x[offset];
				}
				else
				{
					//row_sum[0] = row_sum[0] + am_ptr_x[offset];
					row_sum[3] = row_sum[3] - am_ptr_x[offset];
				}
				if(am_ptr_y[offset]>0)
				{
					row_sum[1] = row_sum[1] + am_ptr_y[offset];
					row_sum[4] = row_sum[4] + am_ptr_y[offset];
				}
				else
				{
					//row_sum[1] = row_sum[1] + am_ptr_y[offset];
					row_sum[4] = row_sum[4] - am_ptr_y[offset];
				}
				if(am_ptr_t[offset]>0)
				{
					row_sum[2] = row_sum[2] + am_ptr_t[offset];
					row_sum[5] = row_sum[5] + am_ptr_t[offset];
				}
				else
				{
					//row_sum[2] = row_sum[2] + am_ptr_t[offset];
					row_sum[5] = row_sum[5] - am_ptr_t[offset];
				}
				offset++;
				for(k=0; k<bin_num; k++)
					integral11[k] = integral22[k] + row_sum[k];
			}
		}
	}

	// accumulated sum along to time line
	for(t=1; t<image_num; t++)
	{
		DST_ ***integral1 = integral_am[t];
		DST_ ***integral2 = integral_am[t-1];
		for(i=0; i<height; i++)
		{
			DST_ **integral11 = *integral1++;
			DST_ **integral22 = *integral2++;
			for(j=0; j<width; j++)
			{
				DST_ *integral111 = *integral11++;
				DST_ *integral222 = *integral22++;
				for(k=0; k<bin_num; k++)
					*integral111++ += *integral222++;
			}
		}
	}

	delete [] row_sum;
}

// need to modify
/**
calculate 3D feature from 3D integral amplitude.
@param feature feature vector
@param integral_am 3D integral amplitude
@param width image width
@param height image height
@param image_num number of images
@param bin_num number of bins
@param cell_size cell size in pixel
@param block_size block size in cell
@param pattern_num # of patterns in a row
@return Returns feature dimension
 */
template <typename T> int calBLOCK3Dfeature(//
		double *feature,//
		T ****integral_am, int width, int height, int image_num,//
		int bin_num, int cell_size, int block_size, int pattern_num)
{
	int bh, bv, ch, cv, ori;

	// # of cells and blocks
	int cell_num_h = width/cell_size;
	int cell_num_v = height/cell_size;
	int block_num_h = cell_num_h - block_size + 1;
	int block_num_v = cell_num_v - block_size + 1;
	int step_num_h = cell_num_h/pattern_num;
	int step_num_v = cell_num_v/pattern_num;

	// cell summation
	T ***cell_sum = new T**[cell_num_v];
	for(cv=0; cv<cell_num_v; cv++)
	{
		cell_sum[cv] = new T*[cell_num_h];
		for(ch=0; ch<cell_num_h; ch++)
			cell_sum[cv][ch] = new T[bin_num];
	}
	T ***cell_sum_ptr = cell_sum;
	T ***integral_t = integral_am[image_num-1];
	int y = -1;
	for(cv=0; cv<cell_num_v; cv++)
	{
		T **cell_sum_ptr1 = *cell_sum_ptr++;
		int x = -1;
		for(ch=0; ch<cell_num_h; ch++)
		{
			T *cell_sum_ptr2 = *cell_sum_ptr1++;
			T *integral4 = integral_t[y+cell_size][x+cell_size];
			if(x>=0 && y>=0)
			{
				T *integral1 = integral_t[y][x];
				T *integral2 = integral_t[y+cell_size][x];
				T *integral3 = integral_t[y][x+cell_size];
				for(ori=0; ori<bin_num; ori++)
					*cell_sum_ptr2++ = *integral1++ - *integral2++ - *integral3++ + *integral4++;
			}
			else if(x<0 && y>=0)
			{
				T *integral3 = integral_t[y][x+cell_size];
				for(ori=0; ori<bin_num; ori++)
					*cell_sum_ptr2++ = - *integral3++ + *integral4++;
			}
			else if(x>=0 && y<0)
			{
				T *integral2 = integral_t[y+cell_size][x];
				for(ori=0; ori<bin_num; ori++)
					*cell_sum_ptr2++ = - *integral2++ + *integral4++;
			}
			else
			{
				for(ori=0; ori<bin_num; ori++)
					*cell_sum_ptr2++ = *integral4++;
			}
			x += cell_size;
		}
		y += cell_size;
	}

	// concatenating block features
	int feature_dim = bin_num*block_size*block_size*pattern_num*pattern_num;
	double *feature_ptr = feature;
	for(bv=0; bv<block_num_v; bv+=step_num_v)
		for(bh=0; bh<block_num_h; bh+=step_num_h)
			// block features
			for(cv=0; cv<block_size; cv++)
				for(ch=0; ch<block_size; ch++)
				{
					T *val = cell_sum[cv+bv][ch+bh];
					for(ori=0; ori<bin_num; ori++)
						*feature_ptr++ = *val++;
				}

	for(cv=0; cv<cell_num_v; cv++)
	{
		for(ch=0; ch<cell_num_h; ch++)
			delete [] cell_sum[cv][ch];
		delete [] cell_sum[cv];
	}
	delete [] cell_sum;

	return feature_dim;
}

/**
calculate 2D integral image
@param integral_image integral image
@param image image data
@param width image width
@param height image height
 */
template <typename DST_, typename SRC_>
void IntegralImage2D(DST_ **integral_image, SRC_ *image, int width, int height)
{
	int i, j;

	DST_ *integral1 = integral_image[0];
	SRC_ *image_ptr = image;
	DST_ row_sum = 0;
	for(j=0; j<width; j++)
	{
		row_sum = row_sum + *image_ptr++;
		*integral1++ = row_sum;
	}
	for(i=1; i<height; i++)
	{
		DST_ *integral1 = integral_image[i];
		DST_ *integral2 = integral_image[i-1];
		row_sum = 0;
		for(j=0; j<width; j++)
		{
			row_sum = row_sum + *image_ptr++;
			*integral1++ = *integral2++ + row_sum;
		}
	}
}

/**
calculate 3D integral image
@param integral_image integral image
@param image_set sequential image data
@param image_num number of images
@param width image width
@param height image height
 */
template <typename DST_, typename SRC_>
void IntegralImage3D(DST_ ***integral_image, SRC_ **image_set, int image_num, int width, int height)
{
	// need to be optimized
	int i, j, k;

	IntegralImage2D(integral_image[0], image_set[0], width, height);
	for(k=1; k<image_num; k++)
	{
		DST_ **integral1 = integral_image[k];
		DST_ **integral2 = integral_image[k-1];
		IntegralImage2D(integral1, image_set[k], width, height);
		for(i=0; i<height; i++)
		{
			DST_ *integral11 = *integral1++;
			DST_ *integral22 = *integral2++;
			for(j=0; j<width; j++)
				*integral11++ += *integral22++;
		}
	}
}

/**
calculate L1 norm distance.
L1 norm distance = sigma(abs(p-q)) (equal 0..limit)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double L1_distance(double *X, double *Y, int dim)
{
	int i;

	// L1 distance = sigma(abs(p-q)) (equal 0..limit)
	double distance = 0;
	for(i=0; i<dim; i++)
		distance += fabs(X[i]-Y[i]);

	return distance;
}

/**
calculate infinite norm distance.
infinite norm distance = max(abs(p-q)) (equal 0..limit)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double infinite_norm_distance(double *X, double *Y, int dim)
{
	// infinite norm = max(abs(p-q)) (equal 0..limit)
	int i;
	double diff;

	double distance = 0;
	for(i=0; i<dim; i++)
	{
		diff = fabs(X[i]-Y[i]);
		distance = MAX(distance, diff);
	}

	return distance;
}

/**
calculate normalized infinite norm distance.
normalized infinite norm distance = max((p-q)^2/(abs(p)+abs(q))) (equal 0..limit)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double normalized_infinite_norm_distance(double *X, double *Y, int dim)
{
	// normalized infinite norm distance = max((p-q)^2/(abs(p)+abs(q))) (equal 0..limit)
	int i;
	double norm, diff;

	double distance = 0;
	for(i=0; i<dim; i++)
	{
		norm = fabs(X[i])+fabs(Y[i]);
		if(norm>1e-10)
		{
			diff = X[i]-Y[i];
			diff = diff*diff/norm;
		}
		else if(X[i] == Y[i])
			diff = 0;
		else
			diff = 1e10;
		distance = MAX(distance, diff);
	}

	return distance;
}

/**
calculate L2 distance.
L2 distance = sigma(p-q)^2 (equal 0..limit)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double L2_distance(double *X, double *Y, int dim)
{
	int i;

	//	// normalize
	//	double sum_X = 0;
	//	double sum_Y = 0;
	//	for(i=0; i<dim; i++)
	//	{
	//		sum_X += X[i];
	//		sum_Y += Y[i];
	//	}
	//	if(sum_X != 0 && sum_Y !=0)
	{
		// L2 distance = sigma(p-q)^2 (equal 0..limit)
		double diff;
		double distance = 0;
		for(i=0; i<dim; i++)
		{
			diff = X[i]-Y[i];
			distance += diff*diff;
		}
		return distance;
	}
	//	else
	//		return 1e10;
}

/**
calculate Chi_square distance.
Chi-square distance = sigma(p-q)^2/(abs(p)+abs(q)) (equal 0..limit)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double Chi_square_distance(double *X, double *Y, int dim)
{
	// Chi-square distance = sigma(p-q)^2/(abs(p)+abs(q)) (equal 0..limit)
	int i;
	double norm, diff;

	double distance = 0;
	for(i=0; i<dim; i++)
	{
		norm = fabs(X[i])+fabs(Y[i]);
		if(norm>1e-10)
		{
			diff = X[i]-Y[i];
			distance += diff*diff/norm;
		}
		else if(X[i] != Y[i])
			distance += 1e10;
	}

	return distance;
}

/**
calculate Hellinger distance.
Bhattacharrya coefficient = sigma(sqrt(pq)) (0..1 equal)
Hellinger distance = sqrt(1-Bc) (equal 0..1)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double Hellinger_distance(double *X, double *Y, int dim)
{
	// Bhattacharrya coefficient = sigma(sqrt(pq)) (0..1 equal)
	// Hellinger distance = sqrt(1-Bc) (equal 0..1)
	int i;

	// normalize
	double sum_X = 0;
	double sum_Y = 0;
	for(i=0; i<dim; i++)
	{
		sum_X += X[i];
		sum_Y += Y[i];
	}
	if(fabs(sum_X)>1e-10 && fabs(sum_Y)>1e-10)
	{
		double scale = sqrt(sum_X*sum_Y);

		// Battacharrya coefficient
		double BC = 0;
		for(i=0; i<dim; i++)
			BC += sqrt(X[i]*Y[i]);
		BC /= scale;

		// Hellinger distance
		double distance = 1-BC;

		return distance;
	}
	else
		return 1e10;
}

/**
calculate Kullback-Leibler divergence.
Kullback-Leibler divergence = sigma(p*log(p/q)), q=0 or p=0 -> 0 (equal 0..)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double Kullback_Leibler_divergence(double *X, double *Y, int dim)
{
	// Kullback-Leibler divergence = sigma(p*log(p/q)), q=0 or p=0 -> 0 (equal 0..)
	// symmetric KL divergence = sigma(p*log(p/q)) + sigma(q*log(q/p)) = sigma((p-q)*log(p/q))
	int i;

	// normalize
	double sum_X = 0;
	double sum_Y = 0;
	for(i=0; i<dim; i++)
	{
		sum_X += X[i];
		sum_Y += Y[i];
	}
	if(fabs(sum_X)>1e-10 && fabs(sum_Y)>1e-10)
	{
		double offset = log(sum_Y/sum_X);

		// symmetric Kullback-Leibler divergence
		double distance = 0;
		for(i=0; i<dim; i++)
			if(fabs(Y[i])>1e-10 && fabs(X[i])>1e-10)
				distance += (X[i]/sum_X-Y[i]/sum_Y)*(log(X[i]/Y[i])+offset);
			else if(Y[i] != X[i])
				distance += (X[i]/sum_X-Y[i]/sum_Y)*(log((X[i]+1e-10)/(Y[i]+1e-10))+offset);

		return distance;
	}
	else
		return 1e10;
}

/**
calculate feature distance.
@param X a vector
@param Y a vector
@param dim dimension of vectors
@param type distance type:
		L1 distance(0),
		L2 distance(1),
		infinite norm distance(2),
		normalized infinite norm distance(3),
		Chi-square(4),
		Hellinger(5),
		KL-divergence(6)
@return Returns distance > 0. (0 when equal)
 */
double calFeature_distance(double *X, double *Y, int dim, int type)
{
	return distance_measure_list[type](X, Y, dim);
}
/*
int match_sift(struct feature *feat1, int n1, struct feature *feat2, int n2)
{
	struct kd_node* kd_root = kdtree_build( feat1, n1 );

	int m = match_sift(kd_root, feat2, n2);

	kdtree_release( kd_root );

	return m;
}

int match_sift(struct kd_node *kd_root, struct feature *feat2, int n2)
{
	struct feature* feat;
	struct feature** nbrs;
	double d0, d1;
	int k, i, m = 0;

	for( i = 0; i < n2; i++ )
	{
		feat = feat2 + i;
		k = kdtree_bbf_knn( kd_root, feat, 2, &nbrs, KDTREE_BBF_MAX_NN_CHKS );
		if( k == 2 )
		{
			d0 = descr_dist_sq( feat, nbrs[0] );
			d1 = descr_dist_sq( feat, nbrs[1] );
			if( d0 < d1 * NN_SQ_DIST_RATIO_THR )
			{
				m++;
				feat2[i].fwd_match = nbrs[0];
			}
		}
		free( nbrs );
	}

	return m;
}
*/
double cal_L1norm(double *X, int num)
{
	int i;
	double norm = 0;
	for(i=0; i<num; i++)
		norm += fabs(X[i]);

	return norm;
}

double cal_L2norm_square(double *X, int num)
{
	int i;
	double norm = 0;
	for(i=0; i<num; i++)
		norm += X[i]*X[i];

	return norm;
}

double cal_L2norm(double *X, int num)
{
	return fast_sqrt(cal_L2norm_square(X, num));
}
