#ifndef _IMAGE_PROCESS_H_
#define _IMAGE_PROCESS_H_
#include "Define_op.h"

void Resize_BI(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);
void Resize_NN(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);
void Resize(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h);

void image2LBP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data);
void image2MCT9_1(unsigned char* src_data, int src_w, int src_h, unsigned short* lbp_data);
void image2MCT(unsigned char *src_data, int src_w, int src_h, unsigned short *mct);

void Position_25L2Org(struct SMyRect *src_rc, int cx, int cy);
void Position_25R2Org(struct SMyRect *src_rc, int cx, int cy);

int  Region(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data, struct SMyRect rcRegion);
void FlipX(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data);

void he_small(unsigned char *src_data, int src_w, int src_h);
int  he(unsigned char *image, int w, int h, unsigned char *amhe_data);

void Rotate(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data);
void InvRotate(SMyRect src, int cpx, int cpy, int angle, SMyRect* dest);

void Rotate_25(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data);

int rgb2gray(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data);
void DrawRoiForFace(int iFaceNum, unsigned char *pData, int Wd, int Ht, struct SMyRect *pFaces);
void DrawRoiForFaceGray(int iFaceNum, unsigned char *pData, int Wd, int Ht, struct SMyRect *pFaces);
int AddBorder(unsigned char *raw_data, int src_w, int src_h, unsigned char *des_data, int border_size);

int Rotate_90(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data);

#endif 
