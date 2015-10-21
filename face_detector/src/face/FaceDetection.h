#ifndef _FACE_DETECTION_H_
#define _FACE_DETECTION_H_

#include "Define_op.h"

int fd_find(unsigned char* srcData, int nSrcW, int nSrcH, bool bRIP, bool bROP);
bool fd_where(struct SMyRect* rcFace, int* nFaceRIP, int* nFaceROP);	

#endif 
