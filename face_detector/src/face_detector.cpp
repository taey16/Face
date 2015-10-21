/**
Optimized for Thumbnail
to detailed experimental result :
1. http://play.daumcorp.com/display/mmd/Face+Detector+Optimize+%28zealota%29
2. http://play.daumcorp.com/pages/viewpage.action?pageId=140368643
3. http://play.daumcorp.com/pages/viewpage.action?pageId=140233025
4. http://play.daumcorp.com/pages/viewpage.action?pageId=140366315

*/

#include "face_detector.h"

#include "face/Define_op.h"
#include "face/FaceDetection.h"

using namespace Daum;

#define _P_NSIZE1 12
static const int _P_SIZE1[_P_NSIZE1] = {33, 44, 54, 65, 76, 86, 97, 107, 118, 129, 139, 150}; // uniform, minimum side < 150, 5 sec

#define _P_NSIZE2 12
static const int _P_SIZE2[_P_NSIZE2] = {33, 36, 39, 44, 49, 55, 64, 76, 94, 121, 173, 300}; //from hw kim, final parameter for thumbnail, 7 sec

#define _P_NSIZE3 15
static const int _P_SIZE3[_P_NSIZE3] = {33, 35, 38, 41, 44, 48, 53, 59, 67, 77, 91, 110, 139, 190, 300}; // 400x300 add layer from hw kim, sec 8 (rip/rop: off/on: 12 sec)

#define _P_NSIZE4 12
static const int _P_SIZE4[_P_NSIZE4] = {33, 48, 63, 79, 94, 109, 124, 139, 154, 170, 185, 200}; // uniform, minimum side < 200, 9 sec

#define _P_NSIZE5 16
static const int _P_SIZE5[_P_NSIZE5] = {33, 35, 37, 40, 43, 47, 51, 56, 63, 71, 81, 95, 115, 144, 195, 300}; // 400x300 add layer from hw kim, sec 9 (rip/rop: off/on: 15 sec)

#define _P_NSIZE6 12
static const int _P_SIZE6[_P_NSIZE6] = {72, 77, 83, 89, 97, 106, 116, 130, 147, 169, 198, 240}; // from hw kim, linear face size, minumum size < 240, linear2, 10 sec

#define _P_NSIZE7 17
static const int _P_SIZE7[_P_NSIZE7] = {33, 35, 37, 40, 42, 46, 50, 54, 59, 66, 74, 85, 99, 119, 149, 199, 300}; // 400x300 add layer from hw kim, sec 10 (rip/rop: off/on: 16 sec)

#define _P_NSIZE8 56
static const int _P_SIZE8[_P_NSIZE8] = {33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94, 96, 99, 102, 105, 108, 111, 114, 117, 120}; // minimum side < 120, 11 sec

#define _P_NSIZE9 18
static const int _P_SIZE9[_P_NSIZE9] = {33, 35, 37, 39, 42, 45, 48, 52, 57, 62, 69, 78, 89, 103, 124, 154, 203, 300}; // 400x300 add layer from hw kim, sec 11 (rip/rop: off/on: 17 sec)

#define _P_NSIZE10 19
static const int _P_SIZE10[_P_NSIZE10] = {33, 35, 37, 39, 41, 44, 47, 50, 55, 59, 65, 72, 81, 92, 107, 128, 158, 207, 300}; // 400x300 add layer from hw kim, sec 12 (rip/rop: off/on: 17 sec)

#define _P_NSIZE11 12
static const int _P_SIZE11[_P_NSIZE11] = {33, 53, 72, 92, 112, 132, 151, 171, 191, 211, 230, 250}; // uniform, minumm side < 250, 13 sec

#define _P_NSIZE12 20
static const int _P_SIZE12[_P_NSIZE12] = {33, 35, 36, 38, 41, 43, 46, 49, 53, 57, 62, 68, 75, 84, 96, 111, 132, 162, 210, 300}; // 400x300 add layer from hw kim, sec 13 (rip/rop: off/on: 18 sec)

#define _P_NSIZE13 12
static const int _P_SIZE13[_P_NSIZE13] = {72, 81, 91, 102, 114, 128, 144, 162, 182, 204, 229, 240}; // tune from jy, minimum side < 240, 14 sec

#define _P_NSIZE14 12
static const int _P_SIZE14[_P_NSIZE14] = {72, 80, 90, 102, 114, 129, 145, 161, 181, 202, 226, 240}; // from hw kim, linear face size, minimum size < 240, linear1, 14 sec

#define _P_NSIZE15 12
static const int _P_SIZE15[_P_NSIZE15] = {33, 57, 82, 106, 130, 154, 179, 203, 227, 251, 276, 300}; // uniform, minimum side < 300, 19 sec

#define _P_NSIZE16 12
static const int _P_SIZE16[_P_NSIZE16] = {33, 62, 91, 119, 148, 177, 206, 235, 264, 292, 321, 350}; // uniform, minimum side < 350, 26 sec

#define _P_NSIZE17 14
static const int _P_SIZE17[_P_NSIZE17] = {72, 81, 91, 102, 114, 128, 144, 162, 182, 204, 229, 257, 289, 300}; // add layer to 240 tune version, minimum side < 300, 42 sec

#define _P_NSIZE18 49
static const int _P_SIZE18[_P_NSIZE18] = {66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94, 96, 99, 102, 105, 108, 111, 114, 117, 120, 123, 126, 129, 133, 137, 141, 145, 149, 153, 157, 161, 166, 171, 176, 181, 186, 191, 196, 202, 208, 214, 220, 226, 233, 240}; // 240 old tune version, 47 sec

#define _P_NSIZE19 12
static const int _P_SIZE19[_P_NSIZE19] = {144, 162, 182, 204, 229, 257, 289, 325, 365, 410, 461, 480}; //480 old tune version, 54 sec

#define _P_NSIZE20 72
static const int _P_SIZE20[_P_NSIZE20] = {66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94, 96, 99, 102, 105, 108, 111, 114, 117, 120, 123, 126, 129, 133, 137, 141, 145, 149, 153, 157, 161, 166, 171, 176, 181, 186, 191, 196, 202, 208, 214, 220, 226, 233, 240, 247, 254, 261, 269, 277, 285, 293, 302, 311, 320, 330, 340, 350, 360, 371, 382, 393, 405, 417, 430, 443, 456, 480}; // 480 old version, 181 sec

#define _P_NSIZE21 8
static const int _P_SIZE21[_P_NSIZE21] = {33, 36, 39, 44, 49, 55, 64, 76}; // image edit app version

extern int m_P_NSIZE;
extern const int *m_P_SIZE;

bool CFaceDetector::Detect(unsigned char *image, uint32_t width, uint32_t height, std::vector<Daum::CObject> &objects)
{
	int		idx1 = 0;
	int 		face_count = 0;
	int		*rip = NULL, *rop = NULL;
	int		optimize_opt;

	bool 		apply_face_rip, apply_face_rop;

	struct SMyRect 	*face_rect = NULL;

	if( this->detector_type.compare("face") != 0){
		fprintf(stderr, "Error: Set detector is not face detector\n");
		return false;
	}

	optimize_opt = this->face_optimize_opt;
	apply_face_rip = this->apply_face_rip;
	apply_face_rop = this->apply_face_rop;
	switch(optimize_opt){
		case 1:
			m_P_NSIZE = _P_NSIZE1;
			m_P_SIZE = _P_SIZE1;
			break;
		case 2:
			m_P_NSIZE = _P_NSIZE2;
			m_P_SIZE = _P_SIZE2;
			break;
		case 3:
			m_P_NSIZE = _P_NSIZE3;
			m_P_SIZE = _P_SIZE3;
			break;
		case 4:
			m_P_NSIZE = _P_NSIZE4;
			m_P_SIZE = _P_SIZE4;
			break;
		case 5:
			m_P_NSIZE = _P_NSIZE5;
			m_P_SIZE = _P_SIZE5;
			break;
		case 6:
			m_P_NSIZE = _P_NSIZE6;
			m_P_SIZE = _P_SIZE6;
			break;
		case 7:
			m_P_NSIZE = _P_NSIZE7;
			m_P_SIZE = _P_SIZE7;
			break;
		case 8:
			m_P_NSIZE = _P_NSIZE8;
			m_P_SIZE = _P_SIZE8;
			break;
		case 9:
			m_P_NSIZE = _P_NSIZE9;
			m_P_SIZE = _P_SIZE9;
			break;
		case 10:
			m_P_NSIZE = _P_NSIZE10;
			m_P_SIZE = _P_SIZE10;
			break;
		case 11:
			m_P_NSIZE = _P_NSIZE11;
			m_P_SIZE = _P_SIZE11;
			break;
		case 12:
			m_P_NSIZE = _P_NSIZE12;
			m_P_SIZE = _P_SIZE12;
			break;
		case 13:
			m_P_NSIZE = _P_NSIZE13;
			m_P_SIZE = _P_SIZE13;
			break;
		case 14:
			m_P_NSIZE = _P_NSIZE14;
			m_P_SIZE = _P_SIZE14;
			break;
		case 15:
			m_P_NSIZE = _P_NSIZE15;
			m_P_SIZE = _P_SIZE15;
			break;
		case 16:
			m_P_NSIZE = _P_NSIZE16;
			m_P_SIZE = _P_SIZE16;
			break;
		case 17:
			m_P_NSIZE = _P_NSIZE17;
			m_P_SIZE = _P_SIZE17;
			break;
		case 18:
			m_P_NSIZE = _P_NSIZE18;
			m_P_SIZE = _P_SIZE18;
			break;
		case 19:
			m_P_NSIZE = _P_NSIZE19;
			m_P_SIZE = _P_SIZE19;
			break;
		case 20:
			m_P_NSIZE = _P_NSIZE20;
			m_P_SIZE = _P_SIZE20;
			break;
		case 21:
			m_P_NSIZE = _P_NSIZE21;
			m_P_SIZE = _P_SIZE21;
			break;
		default:
			fprintf(stderr, "Error: Face Optimize Option\n");
			return false;
	}

	face_count = fd_find(image, width, height, apply_face_rip, apply_face_rop);

	if( face_count == 0)
		return true;

	rip = (int *)malloc(sizeof(int) * face_count);
	memset(rip, 0, sizeof(int) * face_count);
	rop = (int *)malloc(sizeof(int) * face_count);
	memset(rop, 0, sizeof(int) * face_count);
	face_rect = (struct SMyRect *)malloc(sizeof(struct SMyRect) * face_count);
	memset(face_rect, 0, sizeof(struct SMyRect) * face_count);

	fd_where(face_rect, rip, rop);

	for(idx1 = 0; idx1 < face_count; idx1++){
		Daum::CObject elem;

		elem.x		= face_rect[idx1].left;
		elem.y 		= face_rect[idx1].top;
		elem.width 	= face_rect[idx1].right - face_rect[idx1].left;
		elem.height = face_rect[idx1].bottom - face_rect[idx1].top;
		elem.angle  = face_rect[idx1].angle;

		objects.push_back(elem);
	}

	if(rip) free(rip);
	rip = NULL;
	if(rop) free(rop);
	rop = NULL;
	if(face_rect) free(face_rect);
	face_rect = NULL;

	return true;
}

