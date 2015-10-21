#ifndef _IMAGEPROCESSING_H_
#define _IMAGEPROCESSING_H_

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

class CImageProcessing
{
public:
	CImageProcessing();
	virtual ~CImageProcessing();
		
	bool size(IplImage *img, int *w, int *h, int *c);
	IplImage* create(int w, int h, int c);
	void destroy(IplImage **img);

	//file
	IplImage* load(const char *file_name);
	bool save(const char *file_name, IplImage *img);
	bool clone(IplImage *src_img, IplImage *des_img);

	//raw	
	bool getraw_notnull(IplImage *img, unsigned char* pdata);
	bool setraw(IplImage *img, int w, int h, int channel, unsigned char* pdata);

	//color conversion
	bool bgr2gray(IplImage *src_img, IplImage *des_img);

	//region image
	bool region(IplImage *src_img, IplImage *des_img, int roi_x, int roi_y, int roi_w, int roi_h);
};

#endif

