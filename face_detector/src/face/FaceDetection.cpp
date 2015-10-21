#include "FaceDetectionParameters_MCT_ROP0.h"
#include "FaceDetectionParameters_MCT_ROP90L.h"
#include "FaceDetectionParameters_MCT_ROP90R.h"
#include "FaceDetection.h"
#include "ImageProcess.h"

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>

#define FD_MAX_FIND						200
#define FD_INTERSECTION_VALID_COUNT		2
#define FD_BIT_SHIFT					15

int 		m_P_NSIZE;				// �ִ� �Ƕ�̵� layer ��
const int 	*m_P_SIZE;				// �Ƕ�̵� layer�� layer size

int		m_nFaceCount = 0;								//���� �Ǻ��� �󱼰��� ����
struct SMyRect	m_rcFace[FD_MAX_FIND];
bool	m_bFaceValid[FD_MAX_FIND];				//������ ���߿��� ��ȿ���� �ʴٰ� �ǴܵǴ� ��
int		m_nFaceConfidence[FD_MAX_FIND];		//���ļ� �Ǻ��� ���߿��� ���� ���� ���� CONFIDENCE(���� ���� �н��� �̿�� ���� �����Ϳ� ���� ����)
int		m_nFaceRIP[FD_MAX_FIND];
int		m_nFaceROP[FD_MAX_FIND];

int		m_nFaceFinalCount = 0;
struct SMyRect	m_rcFinalFace[FD_MAX_FIND];					//���� �Ǻ��� �󱼰��� ��ġ
int		m_nFaceFinalRIP[FD_MAX_FIND];
int		m_nFaceFinalROP[FD_MAX_FIND];

int		m_nFaceCandidateCountFrontal = 0;						//���ĺ��� �Ǻ� �� ����
int		m_nFaceCandidateCountProfileL = 0;						//���ĺ��� �Ǻ� �� ����
int		m_nFaceCandidateCountProfileR = 0;						//���ĺ��� �Ǻ� �� ����
struct SMyRect	m_rcFaceCandidateFrontal[FD_MAX_FIND];			//���ĺ��� �Ǻ� �� ��ġ
struct SMyRect	m_rcFaceCandidateProfileL[FD_MAX_FIND];			//���ĺ��� �Ǻ� �� ��ġ
struct SMyRect	m_rcFaceCandidateProfileR[FD_MAX_FIND];			//���ĺ��� �Ǻ� �� ��ġ
int		m_nFaceCandidateIntersectionCountFrontal[FD_MAX_FIND];	//���ļ� �Ǻ��� ���� ����
int		m_nFaceCandidateIntersectionCountProfileL[FD_MAX_FIND];	//���ļ� �Ǻ��� ���� ����
int		m_nFaceCandidateIntersectionCountProfileR[FD_MAX_FIND];	//���ļ� �Ǻ��� ���� ����
int		m_nFaceCandidateConfidenceFrontal[FD_MAX_FIND];		//���ļ� �Ǻ��� ���߿��� ���� ���� ���� CONFIDENCE(���� ���� �н��� �̿�� ���� �����Ϳ� ���� ����)
int		m_nFaceCandidateConfidenceProfileL[FD_MAX_FIND];		//���ļ� �Ǻ��� ���߿��� ���� ���� ���� CONFIDENCE(���� ���� �н��� �̿�� ���� �����Ϳ� ���� ����)
int		m_nFaceCandidateConfidenceProfileR[FD_MAX_FIND];		//���ļ� �Ǻ��� ���߿��� ���� ���� ���� CONFIDENCE(���� ���� �н��� �̿�� ���� �����Ϳ� ���� ����)
bool	m_bFaceCandidateValidFrontal[FD_MAX_FIND];				//������ ���߿��� ��ȿ���� �ʴٰ� �ǴܵǴ� ��
bool	m_bFaceCandidateValidProfileL[FD_MAX_FIND];				//������ ���߿��� ��ȿ���� �ʴٰ� �ǴܵǴ� ��
bool	m_bFaceCandidateValidProfileR[FD_MAX_FIND];				//������ ���߿��� ��ȿ���� �ʴٰ� �ǴܵǴ� ��

int fd_find_run(unsigned char* srcData, int nSrcW, int nSrcH, bool bROP, bool bRotateMode, int nAngle);
void fd_scan_frontal(int resize_w, int resize_h, int src_w, int src_h, unsigned short* nMCTData, int nAngle);		//Adaboost�� ����� �󱼰��� ����
void fd_scan_profile(int resize_w, int resize_h, int src_w, int src_h, unsigned short* nMCTData, int nLRMode, int nAngle);		//Adaboost�� ����� �󱼰��� ����
void fd_enroll_Frontal(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence, int nAngle);	//����� ����ǥ�� ���
void fd_enroll_ProfileL(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence, int nAngle);
void fd_enroll_ProfileR(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence, int nAngle);
bool fd_isintersection(struct SMyRect rc1, struct SMyRect rc2);						//�ΰ��� �簢����(����� �󱼿����� ��ǥ)�� �޾Ƽ� ��ġ������ üũ 
void fd_final();

/*
#define P_NSIZE2 19
static const int P_SIZE2[P_NSIZE2] = {60, 80, 100, 120, 140, 160, 190, 240, 290, 320, 350, 390, 420, 440, 480, 520, 560, 600, 640};
 */

int fd_find(unsigned char* srcData, int nSrcW, int nSrcH, bool bRIP, bool bROP)						//�󱼰��� ����
{
	int i;
	int nAngle = 15;//25;
	unsigned char* nRotateData = NULL;
	m_nFaceCount = 0;
	m_nFaceFinalCount = 0;
	memset(m_nFaceConfidence, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceRIP, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceROP, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_bFaceValid, 0, sizeof(bool)*FD_MAX_FIND);

	fd_find_run(srcData, nSrcW, nSrcH, bROP, false, 0);
	if(bRIP) {

		nRotateData = (unsigned char*)malloc(nSrcW*nSrcH);

		for(i=1; i<=3; i++)
		{
			int Angle_temp = i*nAngle;

			Rotate(srcData, nSrcW, nSrcH, Angle_temp, nRotateData);
			fd_find_run(nRotateData, nSrcW, nSrcH, bROP, true, Angle_temp);

			Rotate(srcData, nSrcW, nSrcH, -Angle_temp, nRotateData);
			fd_find_run(nRotateData, nSrcW, nSrcH, bROP, true, -Angle_temp);
		}

		free(nRotateData);
	}
	fd_final();

	m_nFaceFinalCount = 0;
	for(i=0; i<m_nFaceCount; i++) {
		if(m_bFaceValid[i]) {
			m_nFaceFinalRIP[m_nFaceFinalCount] = m_nFaceRIP[i];
			m_nFaceFinalROP[m_nFaceFinalCount] = m_nFaceROP[i];
			m_rcFinalFace[m_nFaceFinalCount++] = m_rcFace[i];				
		}
	}	
	return m_nFaceFinalCount;
}

int fd_find_run(unsigned char* srcData, int nSrcW, int nSrcH, bool bROP, bool bRotateMode, int nAngle)	//�󱼰��� ����
{	
	int i, nResizeW, nResizeH;
	int nRatioHeightAndWidth = nSrcW > nSrcH ? (nSrcH<<FD_BIT_SHIFT)/nSrcW : (nSrcW<<FD_BIT_SHIFT)/nSrcH;

	unsigned char* nResizeData = (unsigned char*)malloc(nSrcW*nSrcH);
	unsigned short* nMCTData = (unsigned short*)malloc(sizeof(unsigned short)*nSrcW*nSrcH);

	int size = 32;	
	SMyRect rcDes;

	m_nFaceCandidateCountFrontal = 0;	
	m_nFaceCandidateCountProfileL = 0;	
	m_nFaceCandidateCountProfileR = 0;	
	memset(m_nFaceCandidateIntersectionCountFrontal, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountProfileL, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateIntersectionCountProfileR, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceFrontal, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceProfileL, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_nFaceCandidateConfidenceProfileR, 0, sizeof(int)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidFrontal, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidProfileL, 0, sizeof(bool)*FD_MAX_FIND);
	memset(m_bFaceCandidateValidProfileR, 0, sizeof(bool)*FD_MAX_FIND);

	for(i = 0; i < m_P_NSIZE; i++){
		size = m_P_SIZE[i];

		nResizeW = nSrcW > nSrcH ? size : (size * nRatioHeightAndWidth)>>FD_BIT_SHIFT;
		nResizeH = nSrcW < nSrcH ? size : (size * nRatioHeightAndWidth)>>FD_BIT_SHIFT;
		if(nResizeW > nSrcW || nResizeH > nSrcH) break;
		if(nResizeW <= 16 || nResizeH <= 16) continue;

		Resize(srcData, nSrcW, nSrcH, nResizeData, nResizeW, nResizeH);
		image2MCT(nResizeData, nResizeW, nResizeH, nMCTData);

		fd_scan_frontal(nResizeW, nResizeH, nSrcW, nSrcH, nMCTData, nAngle);
		if(bROP) { 
			fd_scan_profile(nResizeW, nResizeH, nSrcW, nSrcH, nMCTData, 0, nAngle);
			fd_scan_profile(nResizeW, nResizeH, nSrcW, nSrcH, nMCTData, 1, nAngle);
		}
	}
	free(nResizeData);
	free(nMCTData);

	for(i=0; i<m_nFaceCandidateCountFrontal; i++) {
		if(m_bFaceCandidateValidFrontal[i]) {
			if(m_nFaceCandidateIntersectionCountFrontal[i] > FD_INTERSECTION_VALID_COUNT) {
				if(bRotateMode) {
					m_nFaceRIP[m_nFaceCount] = nAngle;
					InvRotate(m_rcFaceCandidateFrontal[i], (nSrcW >> 1), (nSrcH >> 1), nAngle, &rcDes);
					m_rcFaceCandidateFrontal[i] =  rcDes;
				}
				else m_nFaceRIP[m_nFaceCount] = 0;				

				m_nFaceROP[m_nFaceCount] = 0;
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceFrontal[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount++] = m_rcFaceCandidateFrontal[i];
			}
		}
	}
	for(i=0; i<m_nFaceCandidateCountProfileL; i++) {
		if(m_bFaceCandidateValidProfileL[i]) {
			if(m_nFaceCandidateIntersectionCountProfileL[i] > FD_INTERSECTION_VALID_COUNT) {
				if(bRotateMode) {
					m_nFaceRIP[m_nFaceCount] = nAngle;
					InvRotate(m_rcFaceCandidateProfileL[i], (nSrcW >> 1), (nSrcH >> 1), nAngle, &rcDes);
					m_rcFaceCandidateProfileL[i] =  rcDes;
				}
				else m_nFaceRIP[m_nFaceCount] = 0;

				m_nFaceROP[m_nFaceCount] = -90;
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceProfileL[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount++] = m_rcFaceCandidateProfileL[i];
			}
		}
	}
	for(i=0; i<m_nFaceCandidateCountProfileR; i++) {
		if(m_bFaceCandidateValidProfileR[i]) {
			if(m_nFaceCandidateIntersectionCountProfileR[i] > FD_INTERSECTION_VALID_COUNT) {
				if(bRotateMode) {
					m_nFaceRIP[m_nFaceCount] = nAngle;				
					InvRotate(m_rcFaceCandidateProfileR[i], (nSrcW >> 1), (nSrcH >> 1), nAngle, &rcDes);
					m_rcFaceCandidateProfileR[i] =  rcDes;
				}
				else m_nFaceRIP[m_nFaceCount] = 0;

				m_nFaceROP[m_nFaceCount] = 90;
				m_nFaceConfidence[m_nFaceCount] = m_nFaceCandidateConfidenceProfileR[i];
				m_bFaceValid[m_nFaceCount] = true;
				m_rcFace[m_nFaceCount++] = m_rcFaceCandidateProfileR[i];
			}
		}
	}
	return m_nFaceCount;
}

bool fd_where(struct SMyRect* rcFace, int* nFaceRIP, int* nFaceROP)
{
	if(m_nFaceFinalCount == 0) return false;
	memcpy(nFaceRIP, m_nFaceFinalRIP, sizeof(int)*m_nFaceFinalCount);
	memcpy(nFaceROP, m_nFaceFinalROP, sizeof(int)*m_nFaceFinalCount);
	memcpy(rcFace, m_rcFinalFace, sizeof(struct SMyRect)*m_nFaceFinalCount);
	return true;
}

void fd_scan_frontal(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned short* nMCTData, int nAngle)
{	
	int x, y, c, t;
	int nSearchW, nSearchH;
	nSearchW = nResizeW-FD_FRONTAL_MCT_WINDOW_WIDTH0;
	nSearchH = nResizeH-FD_FRONTAL_MCT_WINDOW_HEIGHT0-2; 
	int nMCTDataStep;
	int nRatioSrcPerResize = (nSrcW<<FD_BIT_SHIFT)/nResizeW;
	int nFaceX, nFaceY, nFaceW, nFaceH;
	int nMCTAlphaCount=0;	
	int nMCTConfidence, nMCTOneCascadeFeatureSize;
	bool bFind = false;
	unsigned short *pMCTData = nMCTData;
	unsigned char *pMCTFeatureY, *pMCTFeatureX;
	int	nMCTFeatureGlobalPosition[FD_FRONTAL_MCT_FEATURE_SIZE_TOTAL];
	int *pMCTFeatureGlobalPosition = nMCTFeatureGlobalPosition;	

	nMCTDataStep=FD_FRONTAL_MCT_WINDOW_WIDTH0+nResizeW;
	nFaceW = (FD_FRONTAL_MCT_WINDOW_WIDTH0*nRatioSrcPerResize)>>FD_BIT_SHIFT;
	nFaceH = (FD_FRONTAL_MCT_WINDOW_HEIGHT0*nRatioSrcPerResize)>>FD_BIT_SHIFT;
	pMCTFeatureY = m_nFrontalMCTFeatureY0, pMCTFeatureX = m_nFrontalMCTFeatureX0;

	for(t=0; t<FD_FRONTAL_MCT_FEATURE_SIZE_TOTAL;) {
		*pMCTFeatureGlobalPosition++ = *pMCTFeatureY++ * nResizeW + *pMCTFeatureX++;
		*pMCTFeatureGlobalPosition++ = *pMCTFeatureY++ * nResizeW + *pMCTFeatureX++;		
		t+=2;
	}

	for(y=0; y<nSearchH; y+=2) {
		for(x=0; x<nSearchW; x++) {
			nMCTAlphaCount = 0;			
			pMCTFeatureGlobalPosition = nMCTFeatureGlobalPosition;				
			for(c=0; c<FD_FRONTAL_MCT_CASCADE_SIZE; c++) { 
				nMCTConfidence = 0;
				bFind = false;
				nMCTOneCascadeFeatureSize = m_nFrontalMCTFeatureSize[c];								
				for(t=0; t<nMCTOneCascadeFeatureSize; ) {
					nMCTConfidence += m_nFrontalMCTAlpha[(nMCTAlphaCount + *(pMCTData + *pMCTFeatureGlobalPosition++))];
					nMCTAlphaCount += FD_FRONTAL_MCT_VALUE_RANGE;

					nMCTConfidence += m_nFrontalMCTAlpha[(nMCTAlphaCount + *(pMCTData + *pMCTFeatureGlobalPosition++))];
					nMCTAlphaCount += FD_FRONTAL_MCT_VALUE_RANGE;

					t+=2;
				}				
				if(nMCTConfidence > m_nFrontalMCTThreshold[c]) break;				
				bFind = true;
			}		


			if(bFind) {
				nFaceX = (x*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				nFaceY = (y*nRatioSrcPerResize)>>FD_BIT_SHIFT;

				nMCTConfidence /= 2;				
				fd_enroll_Frontal(nFaceX, nFaceY, nFaceW, nFaceH, nMCTConfidence, nAngle);
			}
			pMCTData++;
		}
		pMCTData += nMCTDataStep;
	}
}

void fd_scan_profile(int nResizeW, int nResizeH, int nSrcW, int nSrcH, unsigned short* nMCTData, int nLRMode, int nAngle)
{	
	int x, y, c, t;
	int nSearchW, nSearchH;
	nSearchW = nResizeW-FD_PROFILE_MCT_WINDOW_WIDTH0;
	nSearchH = nResizeH-FD_PROFILE_MCT_WINDOW_HEIGHT0-2;
	int nMCTDataStep;
	int nRatioSrcPerResize = (nSrcW<<FD_BIT_SHIFT)/nResizeW;
	int nFaceX, nFaceY, nFaceW, nFaceH;
	int nMCTAlphaCount=0;	
	int nMCTConfidence, nMCTOneCascadeFeatureSize;
	bool bFind = false;
	unsigned short *pMCTData = nMCTData;
	unsigned char *pMCTFeatureY, *pMCTFeatureX, *pProfileMCTAlpha;
	int	nMCTFeatureGlobalPosition[FD_PROFILE_MCT_FEATURE_SIZE_TOTAL];
	int *pMCTFeatureGlobalPosition = nMCTFeatureGlobalPosition;	

	nMCTDataStep=FD_PROFILE_MCT_WINDOW_WIDTH0+nResizeW;
	nFaceW = (FD_PROFILE_MCT_WINDOW_WIDTH0*nRatioSrcPerResize)>>FD_BIT_SHIFT;
	nFaceH = (FD_PROFILE_MCT_WINDOW_HEIGHT0*nRatioSrcPerResize)>>FD_BIT_SHIFT;

	pMCTFeatureY = m_nLProfileMCTFeatureY0;
	if(nLRMode) { pMCTFeatureX = m_nRProfileMCTFeatureX0; pProfileMCTAlpha = m_nRProfileMCTAlpha; }
	else { pMCTFeatureX = m_nLProfileMCTFeatureX0; pProfileMCTAlpha = m_nLProfileMCTAlpha; }

	for(t=0; t<FD_PROFILE_MCT_FEATURE_SIZE_TOTAL;) {
		*pMCTFeatureGlobalPosition++ = *pMCTFeatureY++ * nResizeW + *pMCTFeatureX++;
		*pMCTFeatureGlobalPosition++ = *pMCTFeatureY++ * nResizeW + *pMCTFeatureX++;		
		t+=2;
	}

	for(y=0; y<nSearchH; y+=2) {
		for(x=0; x<nSearchW; x++) {
			nMCTAlphaCount = 0;			
			pMCTFeatureGlobalPosition = nMCTFeatureGlobalPosition;
			for(c=0; c<FD_PROFILE_MCT_CASCADE_SIZE; c++) { 
				nMCTConfidence = 0;
				bFind = false;
				nMCTOneCascadeFeatureSize = m_nProfileMCTFeatureSize[c];								
				for(t=0; t<nMCTOneCascadeFeatureSize; ) {
					nMCTConfidence += pProfileMCTAlpha[(nMCTAlphaCount + *(pMCTData + *pMCTFeatureGlobalPosition++))];
					nMCTAlphaCount += FD_PROFILE_MCT_VALUE_RANGE;

					nMCTConfidence += pProfileMCTAlpha[(nMCTAlphaCount + *(pMCTData + *pMCTFeatureGlobalPosition++))];
					nMCTAlphaCount += FD_PROFILE_MCT_VALUE_RANGE;

					t+=2;
				}
				if(nMCTConfidence > m_nProfileMCTThreshold[c]) break;
				bFind = true;
			}

			if(bFind) {
				nFaceX = (x*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				nFaceY = (y*nRatioSrcPerResize)>>FD_BIT_SHIFT;
				if(nLRMode == 0) fd_enroll_ProfileL(nFaceX, nFaceY, nFaceW, nFaceH, nMCTConfidence, nAngle);
				else fd_enroll_ProfileR(nFaceX, nFaceY, nFaceW, nFaceH, nMCTConfidence, nAngle);
			}
			pMCTData++;
		}
		pMCTData += nMCTDataStep;
	}
}

void fd_enroll_Frontal(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence, int nAngle)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountFrontal) {
		printf("error\n");
		//AfxMessageBox("error");
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;	
	rcFace.angle = nAngle;

	//������ ��ϵ� �� ��ǥ�� ������(IsInterction())�� �����ϴ����� Ȯ���ϰ�,
	//���� �����ϸ� m_nFaceCandidateIntersectionCount�� �ϳ� �����ϰ�
	//���� ��ǥ�� ���� ��ǥ ���̿��� nConfidence�� ���� ������ ��ü�� �Ѵ�.
	for(i=0; i<m_nFaceCandidateCountFrontal; i++) {
		if(m_bFaceCandidateValidFrontal[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateFrontal[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountFrontal[i]++;
			if(m_nFaceCandidateConfidenceFrontal[i] > nConfidence) {
				m_rcFaceCandidateFrontal[i] = rcFace;
				m_nFaceCandidateConfidenceFrontal[i] = nConfidence;
			}
			break;
		}
	}

	//������ ��ϵ� �� ��ǥ�� �������� �����ϴ� ���� ������
	//���ο� �󱼷� ����, �߰��� ����Ѵ�.
	if(bIntersection == false) {
		m_rcFaceCandidateFrontal[m_nFaceCandidateCountFrontal] = rcFace; 
		m_nFaceCandidateIntersectionCountFrontal[m_nFaceCandidateCountFrontal]++;
		m_nFaceCandidateConfidenceFrontal[m_nFaceCandidateCountFrontal] = nConfidence;
		m_bFaceCandidateValidFrontal[m_nFaceCandidateCountFrontal] = true;
		m_nFaceCandidateCountFrontal++;
	}

	//������ ��ϵ� �͵� �߿� ���α׷��� ������ �Ǹ鼭 ��ȣ���� �������� �����ϴ� ���� �߻��� �� �ִµ�,
	//�̶� �� ���̿��� �������� �߻��� �ϸ�, nConfidence�� ���� ���� �������� ��ó���� �Ѵ�.
	for(i=0; i<m_nFaceCandidateCountFrontal; i++) {
		if(m_bFaceCandidateValidFrontal[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountFrontal; j++) {
			if(m_bFaceCandidateValidFrontal[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateFrontal[i], m_rcFaceCandidateFrontal[j]) == false) continue;
			if(m_nFaceCandidateConfidenceFrontal[i] > m_nFaceCandidateConfidenceFrontal[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountFrontal[j] += m_nFaceCandidateIntersectionCountFrontal[i];
				m_bFaceCandidateValidFrontal[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountFrontal[i] += m_nFaceCandidateIntersectionCountFrontal[j];
				m_bFaceCandidateValidFrontal[j] = false;
			}					
		}
	}
}

void fd_enroll_ProfileL(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence, int nAngle)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountProfileL) {
		printf("error\n");
		//AfxMessageBox("error");
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;
	rcFace.angle = nAngle;

	//������ ��ϵ� �� ��ǥ�� ������(IsInterction())�� �����ϴ����� Ȯ���ϰ�,
	//���� �����ϸ� m_nFaceCandidateIntersectionCount�� �ϳ� �����ϰ�
	//���� ��ǥ�� ���� ��ǥ ���̿��� nConfidence�� ���� ������ ��ü�� �Ѵ�.
	for(i=0; i<m_nFaceCandidateCountProfileL; i++) {
		if(m_bFaceCandidateValidProfileL[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateProfileL[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountProfileL[i]++;
			if(m_nFaceCandidateConfidenceProfileL[i] > nConfidence) {
				m_rcFaceCandidateProfileL[i] = rcFace;
				m_nFaceCandidateConfidenceProfileL[i] = nConfidence;
			}
			break;
		}
	}

	//������ ��ϵ� �� ��ǥ�� �������� �����ϴ� ���� ������
	//���ο� �󱼷� ����, �߰��� ����Ѵ�.
	if(bIntersection == false) {
		m_rcFaceCandidateProfileL[m_nFaceCandidateCountProfileL] = rcFace; 
		m_nFaceCandidateIntersectionCountProfileL[m_nFaceCandidateCountProfileL]++;
		m_nFaceCandidateConfidenceProfileL[m_nFaceCandidateCountProfileL] = nConfidence;
		m_bFaceCandidateValidProfileL[m_nFaceCandidateCountProfileL] = true;
		m_nFaceCandidateCountProfileL++;
	}

	//������ ��ϵ� �͵� �߿� ���α׷��� ������ �Ǹ鼭 ��ȣ���� �������� �����ϴ� ���� �߻��� �� �ִµ�,
	//�̶� �� ���̿��� �������� �߻��� �ϸ�, nConfidence�� ���� ���� �������� ��ó���� �Ѵ�.
	for(i=0; i<m_nFaceCandidateCountProfileL; i++) {
		if(m_bFaceCandidateValidProfileL[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountProfileL; j++) {
			if(m_bFaceCandidateValidProfileL[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateProfileL[i], m_rcFaceCandidateProfileL[j]) == false) continue;
			if(m_nFaceCandidateConfidenceProfileL[i] > m_nFaceCandidateConfidenceProfileL[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountProfileL[j] += m_nFaceCandidateIntersectionCountProfileL[i];
				m_bFaceCandidateValidProfileL[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountProfileL[i] += m_nFaceCandidateIntersectionCountProfileL[j];
				m_bFaceCandidateValidProfileL[j] = false;
			}					
		}
	}
}

void fd_enroll_ProfileR(int nFaceX, int nFaceY, int nFaceW, int nFaceH, int nConfidence, int nAngle)
{
	int i, j;
	bool bIntersection = false;
	struct SMyRect rcFace;
	if(FD_MAX_FIND <= m_nFaceCandidateCountProfileR) {
		printf("error\n");
		//AfxMessageBox("error");
		return;
	}
	rcFace.left = nFaceX;
	rcFace.top = nFaceY;
	rcFace.right = nFaceX + nFaceW;
	rcFace.bottom = nFaceY + nFaceH;	
	rcFace.angle = nAngle;

	//������ ��ϵ� �� ��ǥ�� ������(IsInterction())�� �����ϴ����� Ȯ���ϰ�,
	//���� �����ϸ� m_nFaceCandidateIntersectionCount�� �ϳ� �����ϰ�
	//���� ��ǥ�� ���� ��ǥ ���̿��� nConfidence�� ���� ������ ��ü�� �Ѵ�.
	for(i=0; i<m_nFaceCandidateCountProfileR; i++) {
		if(m_bFaceCandidateValidProfileR[i] == false) continue;
		if(fd_isintersection(m_rcFaceCandidateProfileR[i], rcFace)) {			
			bIntersection = true;
			m_nFaceCandidateIntersectionCountProfileR[i]++;
			if(m_nFaceCandidateConfidenceProfileR[i] > nConfidence) {
				m_rcFaceCandidateProfileR[i] = rcFace;
				m_nFaceCandidateConfidenceProfileR[i] = nConfidence;
			}
			break;
		}
	}

	//������ ��ϵ� �� ��ǥ�� �������� �����ϴ� ���� ������
	//���ο� �󱼷� ����, �߰��� ����Ѵ�.
	if(bIntersection == false) {
		m_rcFaceCandidateProfileR[m_nFaceCandidateCountProfileR] = rcFace; 
		m_nFaceCandidateIntersectionCountProfileR[m_nFaceCandidateCountProfileR]++;
		m_nFaceCandidateConfidenceProfileR[m_nFaceCandidateCountProfileR] = nConfidence;
		m_bFaceCandidateValidProfileR[m_nFaceCandidateCountProfileR] = true;
		m_nFaceCandidateCountProfileR++;
	}

	//������ ��ϵ� �͵� �߿� ���α׷��� ������ �Ǹ鼭 ��ȣ���� �������� �����ϴ� ���� �߻��� �� �ִµ�,
	//�̶� �� ���̿��� �������� �߻��� �ϸ�, nConfidence�� ���� ���� �������� ��ó���� �Ѵ�.
	for(i=0; i<m_nFaceCandidateCountProfileR; i++) {
		if(m_bFaceCandidateValidProfileR[i] == false) continue;
		for(j=i+1; j<m_nFaceCandidateCountProfileR; j++) {
			if(m_bFaceCandidateValidProfileR[j] == false) continue;
			if(fd_isintersection(m_rcFaceCandidateProfileR[i], m_rcFaceCandidateProfileR[j]) == false) continue;
			if(m_nFaceCandidateConfidenceProfileR[i] > m_nFaceCandidateConfidenceProfileR[j]) {	//valid is j
				m_nFaceCandidateIntersectionCountProfileR[j] += m_nFaceCandidateIntersectionCountProfileR[i];
				m_bFaceCandidateValidProfileR[i] = false;
			}
			else { //valid is i
				m_nFaceCandidateIntersectionCountProfileR[i] += m_nFaceCandidateIntersectionCountProfileR[j];
				m_bFaceCandidateValidProfileR[j] = false;
			}					
		}
	}
}

bool fd_isintersection(struct SMyRect rc1, struct SMyRect rc2)
{
	int w, is_left, is_top, is_right, is_bottom;
	w = (rc1.right - rc1.left)>>3;
	is_left = rc1.left + w;
	is_top = rc1.top + w;
	is_right = rc1.right - w;	
	is_bottom = rc1.bottom - w;

	w = (rc2.right - rc2.left)>>3;
	rc2.left += w;
	rc2.top += w;
	rc2.right -= w;	
	rc2.bottom -= w;

	if(rc2.left > is_left)		is_left = rc2.left;
	if(rc2.top > is_top)		is_top = rc2.top;
	if(rc2.right < is_right)	is_right = rc2.right;
	if(rc2.bottom < is_bottom)	is_bottom = rc2.bottom;
	if(is_left > is_right) return false;
	if(is_top  > is_bottom) return false;
	return true;
}

void fd_final()
{
	//������ ��ϵ� �͵� �߿� ���α׷��� ������ �Ǹ鼭 ��ȣ���� �������� �����ϴ� ���� �߻��� �� �ִµ�,
	//�̶� �� ���̿��� �������� �߻��� �ϸ�, nConfidence�� ���� ���� �������� ��ó���� �Ѵ�.
	int i, j;
	for(i=0; i<m_nFaceCount; i++) {
		if(m_bFaceValid[i] == false) continue;
		for(j=i+1; j<m_nFaceCount; j++) {
			if(m_bFaceValid[j] == false) continue;
			if(fd_isintersection(m_rcFace[i], m_rcFace[j]) == false) continue;
			if(m_nFaceConfidence[i] > m_nFaceConfidence[j]) {	//valid is j
				m_bFaceValid[i] = false;
			}
			else { //valid is i
				m_bFaceValid[j] = false;
			}					
		}
	}
}
