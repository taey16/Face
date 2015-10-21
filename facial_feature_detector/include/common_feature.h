/**@file
Function declaration for feature extraction.

Feature type:	0. 2D histogram
		1. 2D GIST global
		2. 2D SIFT local
		3. 2D HOG global
		4. 2D SURF global
		5. 3D histogram
		6. 3D GIST - not available
		7. 3D SIFT - no available
		8. 3D HOG
		9. 3D SURF
		10. 2D mean, var
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

#ifndef COMMON_FEATURE_
#define COMMON_FEATURE_

//#include <cv.h>

// include GIST library
//#include "../GIST_library/gist.h"

// include SIFT library
#ifdef __cplusplus
extern "C" {
#endif

//#include "../SIFT_library/sift.h"

#ifdef __cplusplus
}
#endif

// for histogram
#define HIST_BIN_NUM	32	/**< number of bin for histogram feature */

// for GIST
#define GIST_SCALE_NUM	3	/**< number of scale for GIST feature */
#define GIST_BLOCK_NUM	4	/**< number of block for GIST feature */

// for SIFT
#define SIFT_ORIENTATION_NUM	8	/**< number of orientation bin for SIFT feature */

// for HOG
#define HOG_CELL_SIZE			4	/**< cell size in pixel for HOG feature */
#define HOG_BLOCK_SIZE			2	/**< block size in cell for HOG feature */
#define HOG_ORIENTATION_NUM		4	/**< number of orientation bin for HOG feature */
#define AMP_THRESHOLD			5	/**< amplitude threshold to be 0 */

// for 3D HOG
#define HOG3D_CELL_SIZE		4	/**< cell size in pixel for HOG feature */
#define HOG3D_BLOCK_SIZE	2	/**< block size in cell for HOG feature */
#define HOG3D_BIN_NUM		20	/**< number of bin for 3D HOG feature */
#define PHI		1.618033988749895//((1+sqrt(5))/2)	/**< axes phi for 3D HOG feature */

/** @brief feature option */
struct FEATURE_OPTION
{
	// for all
	int type;					/**< feature type */
	int width;					/**< normalized image width */
	int height;					/**< normalized image height */
	int subregion_num;			/**< sub-region num */
	int characteristic_method;	/**< characteristic function: y=sqrt(x) (1), y=x^2 (2), y=x (the others) */
	int normalization_method;	/**< normalization method: none(0), L1_norm(1), L2_norm(2) */
	int clip_frame_num;			/**< # of frames for a feature */
	int step_frame_num;			/**< step # of frame */
	int feature_dim;

	// for 2D histogram
	int hist2d_bin_num;

	// for 2D HOG gloabl
	int hog2d_bin_num;
	int hog2d_block_size;
	int hog2d_cell_size;

	// for 2D SIFT gloabl
	int sift2d_bin_num;
	int sift2d_cell_size;

	// for 2D GIST
	int gist2d_scale_num;
	int gist2d_orientation_num[10];

	// for 2D SIFT
	int sift2d_feature_num[30];
	int sift2d_feature_num_temp;
	int sift2d_feature_dim;

	// for 2D SURF global
	int surf2d_bin_num;
	int surf2d_block_size;
	int surf2d_cell_size;

	// for 3D histogram
	int hist3d_bin_num;

	// for 3D HOG
	int hog3d_bin_num;
	int hog3d_block_size;
	int hog3d_cell_size;
	double normal_axis[20][3];	/**< 3d quantization directions */

	// for 3D SURF
	int surf3d_bin_num;
	int surf3d_block_size;
	int surf3d_cell_size;
};

int save_feature_option(FEATURE_OPTION *feature_option, char *folder_name);
int load_feature_option(FEATURE_OPTION *feature_option, char *folder_name);

struct COMMON_FEATURE
{
	// common variables
	double *feature;	/**< feature vector */
	int dim;		/**< dimension of feature vector */

	// variables for local features
	//	double x;                      	/**< x coord */
	//	double y;                      	/**< y coord */
	//	double a;                      	/**< Oxford-type affine region parameter */
	//	double b;                      	/**< Oxford-type affine region parameter */
	//	double c;                      	/**< Oxford-type affine region parameter */
	//	double scl;                    	/**< scale of a Lowe-style feature */
	//	double ori;                    	/**< orientation of a Lowe-style feature */
	//	int type;                      	/**< feature type, OXFD or LOWE */
	//	int category;                  	/**< all-purpose feature category */
	//	CvPoint2D64f img_pt;           	/**< location in image */
	//	CvPoint2D64f mdl_pt;           	/**< location in model */
};

void release_common_feature(COMMON_FEATURE *common_feature);

// convert image into image_t
/**
convert image into image_t.
@param img an image data
@param width image width
@param height image height
@return Returns image_t structured data.
 */
//template <typename T> image_t *Converte2image_t(T *img, int width, int height);

/**
 * set feature option
 */
void set_Feature_option(//
		FEATURE_OPTION *option,//
		int feature_type, int width, int height, int subregion_num,//
		int clip_frame_num, int step_frame_num,//
		int parameter0, int parameter1, int parameter2, int parameter3);

// initialize GIST
/**
Initialize GIST wavelete.
@param image_width image width
@param image_height image height
@param scale_num number of scales
@param orientation_num number of orientations in each scale
 */
void initialize_GIST_wavelete(int image_width, int image_height, int scale_num, int *orientation_num);

// general feature calculation function
/**
get feature from image or image set.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *getFeature(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

// feature functions
/**
calculate 2D histogram of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dhistogram(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate 2D GIST of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dgist(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate 2D SIFT of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dsift(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate 2D HOG of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dhog(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);
COMMON_FEATURE *calFeature_2Dhog_fast(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate 2D SIFT global of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dsift_global(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate 2D SURF of image
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dsurf(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate 2D mean, std of image
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_2Dmeanvar(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate 3D histogram of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_3Dhistogram(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate 3D GIST of image. (not implemented)
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_3Dgist(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate 3D SIFT of image. (not implemented)
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_3Dsift(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate 3D HOG of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_3Dhog(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate 3D SURF of image.
@param feature_num number of features
@param image image data
@param width image width
@param height image height
@param option feature option
@return Returns array of feature structure.
 */
COMMON_FEATURE *calFeature_3Dsurf(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/** feature function pointer */
typedef COMMON_FEATURE* (*FEATURE_FUNCTION)(int *feature_num, unsigned char **image_set, int width, int height, FEATURE_OPTION option);

/**
calculate histogram of image.
@param hist histogram vector
@param bin number of bins
@param image image data
@param width image width
@param height image height
@return Returns histogram length.
 */
template <typename T> int calRegionHistogram(double *hist, int bin_num, T *image, int width, int height);

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
void calGradient(S *grad, T *image, int width, int height, bool direction_flag);

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
void calBinAmplitude(T1 *og, T2 *am, S *grad_x, S *grad_y, int width, int height, int bin_num);

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
void IntegralImage_hog(DST_ ***integral_am, T1 *og, T2 *am, int bin_num, int width, int height);

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
void IntegralImage_surf(DST_ ***integral_am, T *dx, T *dy, int bin_num, int width, int height);

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
		int bin_num, int cell_size, int block_size, int pattern_num);
template <typename T> int calBLOCKfeature(//
		double *feature,//
		int *og, T *am, int width, int height,//
		int bin_num, int cell_size, int block_size, int pattern_num);

/**
calculate bin amplitude summations of cells.
@param cell_sum bin amplitude summations of cells
@param integral_am integral amplitude
@param cell_size cell size in pixel
@param cell_num_h horizontal number of cells
@param cell_num_v vertical number of cells
@param bin_num number of bin
 */
template <typename T> void calCellSum(T ***cell_sum, T ***integral_am, int cell_size, int cell_num_h, int cell_num_v, int bin_num);

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
void calGradient_t(T *grad, S *image1, S *image2, int width, int height, T denorm);

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
void calBinAmplitude3D(T1 **og, T2 **am, S **grad_x, S **grad_y, S **grad_t, int width, int height, int image_num, int bin_num, double normal_axes[20][3]);

/**
calculate 3D bin.
@param dx gradient dx
@param dy gradient dy
@param dt gradient dt
@param bin_num number of bins
@return Returns bin no.
 */
template <typename T> int getBin3D(T dx, T dy, T dt, int bin_num, double normal_axes[20][3]);

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
void IntegralImage3D_hog(DST_ ****integral_am, T1 **og, T2 **am, int bin_num, int width, int height, int image_num);

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
void IntegralImage3D_surf(DST_ ****integral_am, T **grad_x, T **grad_y, T **grad_t, int bin_num, int width, int height, int image_num);

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
		int bin_num, int cell_size, int block_size, int pattern_num);

// integral image
/**
calculate 2D integral image
@param integral_image integral image
@param image image data
@param width image width
@param height image height
 */
template <typename DST_, typename SRC_>
void IntegralImage2D(DST_ **integral_image, SRC_ *image, int width, int height);

/**
calculate 3D integral image
@param integral_image integral image
@param image_set sequential image data
@param image_num number of images
@param width image width
@param height image height
 */
template <typename DST_, typename SRC_>
void IntegralImage3D(DST_ ***integral_image, SRC_ **image_set, int image_num, int width, int height);

// distance measure
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
double calFeature_distance(double *X, double *Y, int dim, int type);

/**
calculate L1 distance.
L1 distance = sigma(abs(p-q)) (equal 0..limit)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double L1_distance(double *X, double *Y, int dim);

/**
calculate infinite norm distance.
infinite norm distance = max(abs(p-q)) (equal 0..limit)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double infinite_norm_distance(double *X, double *Y, int dim);

/**
calculate normalized infinite norm distance.
normalized infinite norm distance = max((p-q)^2/(abs(p)+abs(q))) (equal 0..limit)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double normalized_infinite_norm_distance(double *X, double *Y, int dim);

/**
calculate L2 distance.
L2 distance = sigma(p-q)^2 (equal 0..limit)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double L2_distance(double *X, double *Y, int dim);

/**
calculate Chi_square distance.
Chi-square distance = sigma((p-q)^2/(abs(p)+abs(q))) (equal 0..limit)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double Chi_square_distance(double *X, double *Y, int dim);

/**
calculate Hellinger distance.
Bhattacharrya coefficient = sigma(sqrt(pq)) (0..1 equal)
Hellinger distance = sqrt(1-Bc) (equal 0..1)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double Hellinger_distance(double *X, double *Y, int dim);

/**
calculate Hellinger distance by lookup.
Bhattacharrya coefficient = sigma(sqrt(pq)) (0..1 equal)
Hellinger distance = sqrt(1-Bc) (equal 0..1)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double Hellinger_distance_lookup(unsigned int *X, unsigned int *Y, int dim);

/**
calculate Kullback-Leibler divergence.
Kullback-Leibler divergence = sigma(p*log(p/q)), q=0 or p=0 -> 0 (equal 0..)
@param X a vector
@param Y a vector
@param dim dimension of vectors
@return Returns distance > 0. (0 when equal)
 */
double Kullback_Leibler_divergence(double *X, double *Y, int dim);

/** @brief distance function pointer */
typedef double (*FEATURE_DISTANCE)(double *X, double *Y, int dim);

double cal_L1norm(double *X, int num);
double cal_L2norm(double *X, int num);
double cal_L2norm_square(double *X, int num);
void normalize_vector(double *feature, int dim, int normalization_method);

double *vector_characteristic(double *feature, int dim, int characteristic_no);
void vector_characteristic(double *feature_new, double *feature, int dim, int characteristic_no);
//
//IplImage *image2IplImage(unsigned char *image, int width, int height, int channel);
//void image2IplImage(IplImage *iplimage, unsigned char *image, int width, int height, int channel);

COMMON_FEATURE *sift2common(struct feature feat);
void sift2common(COMMON_FEATURE *common, struct feature feat);
struct feature *common2sift(COMMON_FEATURE common);
void common2sift(struct feature *feat, COMMON_FEATURE common);

int match_sift(struct feature *feat1, int n1, struct feature *feat2, int n2);
int match_sift(struct kd_node *kd_root, struct feature *feat2, int n2);

#endif
