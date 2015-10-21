#include "ImageProcessing.h"
#include <math.h>

CImageProcessing::CImageProcessing()
{
}

CImageProcessing::~CImageProcessing()
{
}

bool CImageProcessing::size(IplImage *img, int *w, int *h, int *c)
{
	if(!img) return false;

	*w = img->width;
	*h = img->height;
	*c = img->nChannels;

	return true;
}

IplImage* CImageProcessing::create(int w, int h, int c)
{
	IplImage *img = NULL;

	img = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, c);
	
	return img;
}

void CImageProcessing::destroy(IplImage **img)
{
	if( !(*img) )
		return;

	cvReleaseImage(img);
	*img = NULL;

	return;
}

IplImage* CImageProcessing::load(const char *file_name)
{
	IplImage *img = NULL;

	img = cvLoadImage(file_name);

	return img;
}

bool CImageProcessing::save(const char *file_name, IplImage *img)
{
	if(!img || !file_name) 
		return false;

	return cvSaveImage(file_name, img);
}

bool CImageProcessing::clone(IplImage *src_img, IplImage *des_img)
{
	if(!src_img || !des_img) return false;

	cvCopy(src_img, des_img);

	return true;
}

bool CImageProcessing::getraw_notnull(IplImage *img, unsigned char* pdata)
{
	if(!img) return false;

	int i, w=0, h=0, channel=0;
	int step = 0, dumy = 0;

	w = img->width;
	h = img->height;
	channel = img->nChannels; 
	step = w*channel;
	dumy = step % 4;

	if(dumy) {
	    dumy = 4 - dumy;
	}
	if(!dumy) {
	    memcpy(pdata, img->imageData, img->imageSize);
	} else {
		for(i=0; i<h; i++) {
		    memcpy(pdata+i*step, img->imageData+i*(step+dumy), step);
		}
	}	
	return true;
}

bool CImageProcessing::setraw(IplImage *img, int w, int h, int channel, unsigned char* pdata)
{
	int i=0;
	int step;
	step = w * channel;
	int dumy;
	dumy = step % 4;

	if(!img) return false;

	if(dumy) {
	    dumy = 4 - dumy;
	}
	if(!dumy) {
	    memcpy(img->imageData, pdata, img->imageSize);
	} else {
		for(i=0; i<h; i++) {
		    memcpy(img->imageData+i*(step+dumy), pdata+i*step, step);
		}
	}
	return true;
}

bool CImageProcessing::bgr2gray(IplImage *src_img, IplImage *des_img)
{
	if(!src_img || src_img->nChannels != 3) return false;

	cvCvtColor(src_img, des_img, CV_BGR2GRAY);

	return true;
}

bool CImageProcessing::region(IplImage *src_img, IplImage *des_img, int roi_x, int roi_y, int roi_w, int roi_h)
{
	if( !src_img ) return false;
	if(roi_x < 0 || roi_y < 0) return false;
	if(roi_w > src_img->width || roi_h > src_img->height) return false;
	if(roi_x + roi_w > src_img->width || roi_y + roi_h > src_img->height) return false;
	
	cvSetImageROI(src_img, cvRect(roi_x, roi_y, roi_w, roi_h));
	clone(src_img, des_img);
	cvResetImageROI(src_img);

	return true;
}
