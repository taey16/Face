#include "Define_op.h"
#include "ImageProcess.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

#define BIT_SHIFT		15

void Resize_BI(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h)
{
	/* < 10 */
	int udtx, udty;
	/* < 10 */
	int uda_11;
	/* < 10 */
	int uda_22;
	int i, j;
	int utx, uty;
	int locgray4 = 0;
	int ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0;
	unsigned int aa, ab, ac, ad;
	
	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = des_data;	

	/* Affine transformation */
	
	uda_11 = ( src_w << 10 ) / des_w;
	uda_22 = ( src_h << 10 ) / des_h;

	for ( j = 0; j < des_h; j++ ) {
		for ( i = 0; i < des_w; i++ ) {
			locgray4 = 0;

			udtx = uda_11 * i;
			udty = uda_22 * j;

			utx = udtx >> 10;
			uty = udty >> 10;
			udtx -= ( utx << 10 );
			udty -= ( uty << 10 );
			pucsrcp = src_data + ( uty * src_w ) + utx;

			ucgray1 = *pucsrcp;

			if ( i >= des_w - 1 ) {
				ucgray2 = *pucsrcp;
				udtx = 1 << 10;
			}
			else {
				ucgray2 = *( pucsrcp + 1 );
				locgray4 += 1;
			}

			if ( j >= des_h - 1 ) {
				ucgray3 = *pucsrcp;
				udty = 1 << 10;
			}
			else {
				ucgray3 = *( pucsrcp + src_w );
				locgray4 += src_w;
			}

			ucgray4 = *( pucsrcp + locgray4 );

			aa = ucgray1 << 20;
			ab = ( ( ucgray2 - ucgray1 ) * ( udtx << 10 ) );
			ac = ( ( ucgray3 - ucgray1 ) * ( udty << 10 ) );
			ad = ( ucgray1 + ucgray4 - ucgray3 - ucgray2 ) * udtx * udty;

			*( pucdstp + i ) = ( unsigned char ) ( ( aa + ab + ac + ad ) >> 20 );
		}
		
		pucdstp += des_w;
	}
}

void Resize_NN(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h)
{
	int i;
	unsigned int utx, uty=0;
	unsigned int uda_11 = (src_w << BIT_SHIFT) / des_w;
	unsigned int uda_22 = (src_h << BIT_SHIFT) / des_h;
	unsigned char *psrc_data = src_data;
	unsigned char *pdes_data = des_data;	

	while(des_h--) {
		utx = 0;
		i = des_w;
		while(i--) {
			utx += uda_11;						
			*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));		
		}		
		uty += uda_22;		
		psrc_data = src_data + (uty>>BIT_SHIFT)*src_w;
	}
}

void image2LBP8_1(unsigned char* src_data, int src_w, int src_h, unsigned char* lbp_data)
{
	unsigned int cx;
	unsigned int size = src_w*(src_h-2)-2;
	unsigned int data_lbp;		
	unsigned char *pt = src_data;
	unsigned char *plbp = lbp_data+(src_w+1);
	
	while(size--) {
		cx = *(pt+src_w+1);
		
		data_lbp = ((unsigned int)(cx - *pt) & 0x80000000) >> 31;
		data_lbp |= ((unsigned int)(cx - *(pt+1)) & 0x80000000) >> 30;
		data_lbp |= ((unsigned int)(cx - *(pt+2)) & 0x80000000) >> 29;
		data_lbp |= ((unsigned int)(cx - *(pt+src_w)) & 0x80000000) >> 24;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_w+2))) & 0x80000000) >> 28;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_w+src_w))) & 0x80000000) >> 25;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_w+src_w+1))) & 0x80000000) >> 26;
		data_lbp |= ((unsigned int)(cx - *(pt+(src_w+src_w+2))) & 0x80000000) >> 27;
		
		pt++;
		*plbp++ = data_lbp;	
	}
}


void image2MCT(unsigned char *src_data, int src_w, int src_h, unsigned short *mct)
{
	int x, y, w=src_w, h=src_h-1, src_size=src_w*src_h;
	int m1, m2, m3, m4, m5, m6, m7, m8, m9;
	int mu1, mu2, mu3, mu4, mu5, mu6, mu7, mu8, mu9;
	int data_mean;
	unsigned short data_mct;	
	unsigned char *pt1, *pt2=src_data+src_w, *pt3;

	memset(mct, 0, sizeof(unsigned short)*src_size);
	unsigned short *pmct = mct+src_w+1;

	pt1 = pt2-src_w;
	pt3 = pt2+src_w;
	for(y=1; y<h; y++) {
		m1 = *pt1++;
		m4 = *pt2++;
		m7 = *pt3++;
		m2 = *pt1++;
		m5 = *pt2++;
		m8 = *pt3++;		
		for(x=2; x<w; x++) {
			m3 = *pt1++;
			m6 = *pt2++;
			m9 = *pt3++;
			data_mean = ((m1+m2+m3+m4+m5+m6+m7+m8+m9));

			mu1 = data_mean - (( m1 << 3 ) + m1);
			mu2 = data_mean - (( m2 << 3 ) + m2);
			mu3 = data_mean - (( m3 << 3 ) + m3);
			mu4 = data_mean - (( m4 << 3 ) + m4);
			mu5 = data_mean - (( m5 << 3 ) + m5);
			mu6 = data_mean - (( m6 << 3 ) + m6);
			mu7 = data_mean - (( m7 << 3 ) + m7);
			mu8 = data_mean - (( m8 << 3 ) + m8);
			mu9 = data_mean - (( m9 << 3 ) + m9);

			data_mct = ((mu1>>31)&1) + (((mu2>>31)&1)<<1) + (((mu3>>31)&1)<<2) +
				(((mu4>>31)&1)<<3) + (((mu5>>31)&1)<<4) + (((mu6>>31)&1)<<5) +
				(((mu7>>31)&1)<<6) + (((mu8>>31)&1)<<7) + (((mu9>>31)&1)<<8);

			m1 = m2;
			m4 = m5;
			m7 = m8;
			m2 = m3;
			m5 = m6;
			m8 = m9;

			*pmct++ = data_mct;
		}
		pmct++;
		pmct++;
	}
}

void Resize(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data, int des_w, int des_h)
//void Resize(unsigned char *srcData, int nSrcW, int nSrcH, unsigned char *desData, int nDesW, int nDesH)
{
	int i;
	unsigned int utx, uty=0;
	unsigned int uda_11 = (src_w << BIT_SHIFT) / des_w;
	unsigned int uda_22 = (src_h << BIT_SHIFT) / des_h;
	unsigned char *psrc_data = src_data;
	unsigned char *pdes_data = des_data;	

	while(des_h--) {
		utx = 0;
		i = des_w;
		while(i--) {
			utx += uda_11;						
			*pdes_data++ = *(psrc_data + (utx>>BIT_SHIFT));		
		}		
		uty += uda_22;		
		psrc_data = src_data + (uty>>BIT_SHIFT)*src_w;
	}
/*
	int udtx=0, udty=0, udtx_=0, udty_=0;
	int uda_11;
	int uda_22;
	int i, j;
	int utx=0, uty=0;
	int locgray4 = 0;
	int ucgray1 = 0, ucgray2 = 0, ucgray3 = 0, ucgray4 = 0;
	int nSrcIndex=0;
	unsigned char *pucsrcp = 0;
	unsigned char *pucdstp = desData;
	unsigned int aa, ab, ac, ad;
	
	uda_11 = (nSrcW << 10) / nDesW;
	uda_22 = (nSrcH << 10) / nDesH;

	for(j = 0; j < nDesH; j++) {
		udtx = 0;
		udtx_ = 0;
		utx = 0;
		for(i = 0; i < nDesW; i++) {
			locgray4 = 0;						
			pucsrcp = srcData + (nSrcIndex + utx);
			ucgray1 = *pucsrcp;

			if(i >= nDesW - 1) {
				ucgray2 = *pucsrcp;
				udtx = 1 << 10;
			}
			else {
				ucgray2 = *(pucsrcp + 1);
				locgray4 += 1;
			}

			if (j >= nDesH - 1) {
				ucgray3 = *pucsrcp;
				udty = 1 << 10;
			}
			else {
				ucgray3 = *( pucsrcp + nSrcW );
				locgray4 += nSrcW;
			}
			ucgray4 = *( pucsrcp + locgray4 );

			aa = ucgray1 << 20;
			ab = ((ucgray2 - ucgray1) * (udtx << 10));
			ac = ((ucgray3 - ucgray1) * (udty << 10));
			ad = (ucgray1 + ucgray4 - ucgray3 - ucgray2) * udtx * udty;
			*(pucdstp + i) = (unsigned char)((aa + ab + ac + ad)>> 20);

			udtx_ += uda_11;
			udtx = udtx_;			
			utx = udtx >> 10;			
			udtx -= ( utx << 10 );
		}
		
		udty_ += uda_22;
		udty = udty_;
		uty = udty >> 10;
		udty -= ( uty << 10 );
		nSrcIndex = uty * nSrcW;
		pucdstp += nDesW;
	}
*/
}


void image2MCT9_1(unsigned char* src_data, int src_w, int src_h, unsigned short* mct_data)
{
	unsigned int m;
	unsigned int size = src_w*(src_h-2)-2;
	unsigned int data_mct;		
	unsigned char *m1 = src_data;
	unsigned char *m2 = m1 + 1;
	unsigned char *m3 = m2 + 1;
	unsigned char *m8 = m1 + src_w;
	unsigned char *m9 = m8 + 1;
	unsigned char *m4 = m9 + 1;
	unsigned char *m7 = m8 + src_w;
	unsigned char *m6 = m7 + 1;
	unsigned char *m5 = m6 + 1;
	unsigned short *pmct = mct_data+(src_w+1);
	
	while(size--) {
		m = *m1 + *m2 + *m3 + *m4 + *m5 + *m6 + *m7 + *m8 + *m9;
		data_mct = ((unsigned int)(m - (*m1 << 3) - *m1) & 0x80000000) >> 31;
		data_mct |= ((unsigned int)(m - (*m2 << 3) - *m2) & 0x80000000) >> 30;
		data_mct |= ((unsigned int)(m - (*m3 << 3) - *m3) & 0x80000000) >> 29;
		data_mct |= ((unsigned int)(m - (*m8 << 3) - *m8) & 0x80000000) >> 24;
		data_mct |= ((unsigned int)(m - (*m9 << 3) - *m9) & 0x80000000) >> 23;
		data_mct |= ((unsigned int)(m - (*m4 << 3) - *m4) & 0x80000000) >> 28;
		data_mct |= ((unsigned int)(m - (*m7 << 3) - *m7) & 0x80000000) >> 25;
		data_mct |= ((unsigned int)(m - (*m6 << 3) - *m6) & 0x80000000) >> 26;
		data_mct |= ((unsigned int)(m - (*m5 << 3) - *m5) & 0x80000000) >> 27;
		m1++;	m2++;	m3++;	
		m4++;	m5++;	m6++;	
		m7++;	m8++;	m9++;	
		*pmct++ = data_mct;	
	}
}

int Region(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data, struct SMyRect rcRegion)
{	
	int roi_x = rcRegion.left, roi_y = rcRegion.top;
	int roi_w = rcRegion.right - rcRegion.left;
	int roi_h = rcRegion.bottom - rcRegion.top;

	int i, src_index, region_index;
	if(roi_x+roi_w > src_w) return 0;
	if(roi_y+roi_h > src_h) return 0;
		
	src_index = roi_y*src_w + roi_x;
	region_index = 0;
	for(i = 0; i<roi_h; i++) {
		memcpy(des_data+region_index, src_data+src_index, roi_w);
		src_index += src_w;
		region_index += roi_w;
	}
	return 1;
}

void FlipX(unsigned char* src_data, int src_w, int src_h, unsigned char* des_data)
{
	int y, k, w;
	unsigned char* raw_data = (unsigned char*)malloc(src_w);
	
	unsigned char* praw;
	unsigned char* pdes = des_data;

	for(k=0, y=0; y<src_h; y++) {
		memcpy(raw_data, src_data+k, src_w);		
		praw = raw_data + src_w-1;
		w = src_w;
		while(w--) *pdes++ = *praw--;
		k += src_w;
	}
	free(raw_data);
}

void Position_25L2Org(struct SMyRect *src_rc, int cx, int cy)
{
	struct SMyPoint des_pt;
	int ptx_org = ((src_rc->left + src_rc->right)>>1) - cx;
	int pty_org = -((src_rc->top + src_rc->bottom)>>1) + cy;
	int w = src_rc->right - src_rc->left;
	int h = src_rc->bottom - src_rc->top;

	des_pt.x = 0.906307787036650 * ptx_org + 0.422618261740699 * pty_org + cx;
	des_pt.y = -( 0.906307787036650 * pty_org - 0.422618261740699 * ptx_org ) + cy;

	src_rc->left = des_pt.x - (w>>1);
	src_rc->right = src_rc->left + w;
	src_rc->top = des_pt.y - (h>>1);
	src_rc->bottom = src_rc->top + h;
}

void Position_25R2Org(struct SMyRect *src_rc, int cx, int cy)
{
	struct SMyPoint des_pt;
	int ptx_org = ((src_rc->left + src_rc->right)>>1) - cx;
	int pty_org = -((src_rc->top + src_rc->bottom)>>1) + cy;
	int w = src_rc->right - src_rc->left;
	int h = src_rc->bottom - src_rc->top;

	des_pt.x = 0.906307787036650 * ptx_org - 0.422618261740699 * pty_org + cx;
	des_pt.y = -( 0.906307787036650 * pty_org + 0.422618261740699 * ptx_org ) + cy;

	src_rc->left = des_pt.x - (w>>1);
	src_rc->right = src_rc->left + w;
	src_rc->top = des_pt.y - (h>>1);
	src_rc->bottom = src_rc->top + h;
}

// Histogram equalization for small image ( lower than 128 x 128 )
void he_small(unsigned char *src, int w, int h)
{
	int i, size = w * h;
	unsigned char *pSrc = src;
	unsigned short *map_table = (unsigned short*)malloc( sizeof(unsigned short) * 256 );
	
	unsigned short *pMap1 = map_table+1, *pMap2 = map_table;
	memset( map_table, 0, sizeof(unsigned short) * 256 );
	for(i=size; i--; ) map_table[*pSrc++]++;
	for(i=255; i--; ) *pMap1++ += *pMap2++;
	
	unsigned int hist_value = 66846720 / map_table[255]; // 255 * 2^18
	pMap1 = map_table;
	for(i=256; i--; ) {
		*pMap1 = ( *pMap1 * hist_value ) >> 18;
		pMap1++;
	}
	pSrc = src;
	for(i=size; i--; ) {
		*pSrc = (unsigned char)map_table[*pSrc];
		pSrc++;
	}
	free(map_table);
}

int he(unsigned char *image, int w, int h, unsigned char *amhe_data)
{
	int i, size = w * h, hist_value;
	unsigned char* pimage;
	int* pmap_table;
	int map_table[256];
	memset(map_table, 0, sizeof(int)*256);
	memcpy(amhe_data, image, size);

	pimage = amhe_data;
	for(i=0; i<size; i++) map_table[*pimage++]++;
	pmap_table = map_table;
	for(i=1; i<256; i++) {
		*(pmap_table +1) += *pmap_table;
		pmap_table++;
	}
	hist_value = 33423360/map_table[255];	
	pmap_table = map_table;
	for(i=0; i<256; i++) {
		*pmap_table = (*pmap_table*hist_value)>>17;
		pmap_table++;
	}
	pimage = amhe_data;
	for(i=0; i<size; i++) {
		*pimage = (unsigned char)map_table[*pimage];
		pimage++;
	}
	return 1;
}

void Rotate(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data)
{
	int i;
	int utx=0, uty=0;
	int uda_x, uda_y;
	int des_w = src_w, des_h = src_h;
	unsigned char *pstart_data;
	unsigned char *psrc_data;
	unsigned char *pdes_data = des_data;

	if ( !angle ) {
		uda_x = (src_w << 10) / des_w;
		uda_y = (src_h << 10) / des_h;
		pstart_data = src_data;
		psrc_data = pstart_data;
		while(des_h--) {
			utx = 0;
			i = des_w;
			while(i--) {									
				*pdes_data++ = *(psrc_data + (utx>>10));		
				utx += uda_x;
			}		
			uty += uda_y;		
			psrc_data = pstart_data + (uty>>10)*src_w;
		}
	} else if(angle == 90) {
		uda_x = (src_w << 10) / des_h;
		uda_y = (src_h << 10) / des_w;
		pstart_data = src_data + (src_h * src_w);
		psrc_data = pstart_data;

		while(des_h--) {
			uty = 0;
			i = des_w;
			while(i--) {
				*pdes_data++ = *(psrc_data - (uty>>10)*src_w);		
				uty += uda_y;
			}		
			utx += uda_x;	
			psrc_data = pstart_data + (utx>>10);			
		}

	} else if(angle == -90){
		uda_x = (src_w << 10) / des_h;
		uda_y = (src_h << 10) / des_w;
		pstart_data = src_data + src_h;
		psrc_data = pstart_data;

		while(des_h--) {
			uty = 0;
			i = des_w;
			while(i--) {
				*pdes_data++ = *(psrc_data + (uty>>10)*src_w);		
				uty += uda_y;
			}		
			utx += uda_x;	
			psrc_data = pstart_data - (utx>>10);			
		}

	} else if(angle == 45){
		const int scx = src_w>>1;
		const int scy = src_h>>1;
		const int dcx = des_w>>1;
		const int dcy = des_h>>1;
		const unsigned char *psrc = src_data;
		unsigned char *pdst = des_data;
		const int ratio45_sft10 = 724;
		const int alpha_sft10 = (scx<<10) + ratio45_sft10 * (dcx + dcy);
		const int beta_sft10 =  (scy<<10) + ratio45_sft10 * (dcy - dcx);
		const int gamma = (beta_sft10 - (src_h<<10))/ratio45_sft10;
		const int etha  = (alpha_sft10 / ratio45_sft10);
		int temp;
		int interval;

		while(des_h--) 
		{
			i = des_w;
			while(i--) 
			{
				int X = (alpha_sft10 - ratio45_sft10 * (des_h + i))>>10;
				int Y = (beta_sft10  - ratio45_sft10 * (des_h - i))>>10;

				if( X >= 0 && X < src_w && Y >= 0 && Y < src_h){
					*pdst++ = psrc[Y*src_w+X];
				}
				else{
					if(Y>=src_h || X < 0){
						temp = (Y>=src_h)?(des_h - gamma):(etha - des_h);
						interval = i - temp;
						if(interval>1){
							while(interval--) *pdst++ = 0;
							i = temp;
							*pdst++ = 0;
						}
						else *pdst++ = 0;
					}
					else{
						while(i--) *pdst++ = 0;
						*pdst++ = 0;
						break;
					}
				}
			}
		}
	}
	else if(angle == -45){
		const int scx = src_w>>1;
		const int scy = src_h>>1;
		const int dcx = des_w>>1;
		const int dcy = des_h>>1;
		const unsigned char *psrc = src_data;
		unsigned char *pdst = des_data;
		const int ratio45_sft10 = 724;
		const int alpha_sft10 = (scx<<10) - ratio45_sft10 * (dcy - dcx);
		const int beta_sft10 =  (scy<<10) + ratio45_sft10 * (dcx + dcy);
		const int gamma = (alpha_sft10 / ratio45_sft10);
		const int etha  = (beta_sft10  / ratio45_sft10);
		int temp;
		int interval;

		while(des_h--) 
		{
			i = des_w;
			while(i--) 
			{
				int X = (alpha_sft10 - ratio45_sft10 * (i - des_h))>>10;
				int Y = (beta_sft10 -  ratio45_sft10 * (i + des_h))>>10;

				if( X >= 0 && X < src_w && Y >= 0 && Y < src_h){
					*pdst++ = psrc[Y*src_w+X];
				}
				else{
					if(Y < 0 || X < 0){
						temp = (X < 0) ? (gamma + des_h) :(etha - des_h);
						interval = i - temp;
						if(interval>1){
							while(interval--) *pdst++ = 0;
							i = temp;
							*pdst++ = 0;
						}
						else *pdst++ = 0;
					}
					else{
						while(i--) *pdst++ = 0;
						*pdst++ = 0;
						break;
					}
				}
			}
		}
	}
	else {		
		const int cosT = int(cos(-angle*0.0174532889)*1024);
		const int sinT = int(sin(-angle*0.0174532889)*1024);
		const int inv_ratio_sft10_x_cost = cosT<<10;
		const int inv_ratio_sft10_x_sint = sinT<<10;

		const int scx_sft20 = src_w<<19;
		const int scy_sft20 = src_h<<19;
		const int dcx = des_w>>1;
		const int dcy = des_h>>1;

		const unsigned char *psrc = src_data;
		unsigned char *pdst = des_data;

		int inv_ratio_sft10_x_sint_x_dy = -inv_ratio_sft10_x_sint*dcy;
		int inv_ratio_sft10_x_cost_x_dy = -inv_ratio_sft10_x_cost*dcy;

		//for( y = 0; y < des_h; y++ )
		while(des_h--) 
		{
			int inv_ratio_sft10_x_sint_x_dx = -inv_ratio_sft10_x_sint*dcx;
			int inv_ratio_sft10_x_cost_x_dx = -inv_ratio_sft10_x_cost*dcx;

			//for( x = 0; x < des_w; x++ )
			i = des_w;
			while(i--) 
			{
				int X = (scx_sft20 + inv_ratio_sft10_x_cost_x_dx - inv_ratio_sft10_x_sint_x_dy + 524288)>>20;
				int Y = (scy_sft20 + inv_ratio_sft10_x_sint_x_dx + inv_ratio_sft10_x_cost_x_dy + 524288)>>20;

				if( X >= 0 && X < src_w && Y >= 0 && Y < src_h )
					*pdst++ = psrc[src_w*Y+X];
				else
					*pdst++ = 0;

				inv_ratio_sft10_x_sint_x_dx += inv_ratio_sft10_x_sint;
				inv_ratio_sft10_x_cost_x_dx += inv_ratio_sft10_x_cost;
			}
			inv_ratio_sft10_x_sint_x_dy += inv_ratio_sft10_x_sint;
			inv_ratio_sft10_x_cost_x_dy += inv_ratio_sft10_x_cost;
		}
	}
}

void InvRotate(SMyRect src, int cpx, int cpy, int angle, SMyRect* dest)
{
	SMyPoint src_cp, des_cp;
	int x, y, wh;
	double radian = angle*0.0174532889;

	src_cp.x = ( src.left + src.right ) >> 1;
	src_cp.y = ( src.top + src.bottom ) >> 1;    

	x = src_cp.x - cpx;
	y = cpy - src_cp.y; 	
	des_cp.x = int( x * cos(radian) - y * sin(radian));
	des_cp.y = int( x * sin(radian) + y * cos(radian));
	des_cp.x += cpx;
	des_cp.y = cpy - des_cp.y;

	wh = src.right - src.left;
	dest->left = des_cp.x - (wh>>1);
	dest->right = dest->left + wh;
	dest->top = des_cp.y - (wh>>1);
	dest->bottom = dest->top + wh; 
	dest->angle = angle;
}


void Rotate_25(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data)
{
	int i;	
	int des_w = src_w, des_h = src_h;

	//int cos_v=928, sin_v=433;
	//if(angle < 0) sin_v=-433;

	int cosT = 928;//int(cos(-angle*0.0174532889)*1024);
	int sinT = 433;//int(sin(-angle*0.0174532889)*1024);
	if(angle > 0) sinT=-433;

	const int inv_ratio_sft10_x_cost = cosT<<10;
	const int inv_ratio_sft10_x_sint = sinT<<10;
	const int scx_sft20 = src_w<<19;
	const int scy_sft20 = src_h<<19;
	const int dcx = des_w>>1;
	const int dcy = des_h>>1;
	const unsigned char *psrc = src_data;
	unsigned char *pdst = des_data;

	int inv_ratio_sft10_x_sint_x_dy = -inv_ratio_sft10_x_sint*dcy;
	int inv_ratio_sft10_x_cost_x_dy = -inv_ratio_sft10_x_cost*dcy;

	//for( y = 0; y < des_h; y++ )
	while(des_h--) 
	{
		int inv_ratio_sft10_x_sint_x_dx = -inv_ratio_sft10_x_sint*dcx;
		int inv_ratio_sft10_x_cost_x_dx = -inv_ratio_sft10_x_cost*dcx;

		//for( x = 0; x < des_w; x++ )
		i = des_w;
		while(i--) 
		{
			int X = (scx_sft20 + inv_ratio_sft10_x_cost_x_dx - inv_ratio_sft10_x_sint_x_dy + 524288)>>20;
			int Y = (scy_sft20 + inv_ratio_sft10_x_sint_x_dx + inv_ratio_sft10_x_cost_x_dy + 524288)>>20;

			if( X >= 0 && X < src_w && Y >= 0 && Y < src_h )
				*pdst++ = psrc[src_w*Y+X];
			else
				*pdst++ = 0;

			inv_ratio_sft10_x_sint_x_dx += inv_ratio_sft10_x_sint;
			inv_ratio_sft10_x_cost_x_dx += inv_ratio_sft10_x_cost;
		}
		inv_ratio_sft10_x_sint_x_dy += inv_ratio_sft10_x_sint;
		inv_ratio_sft10_x_cost_x_dy += inv_ratio_sft10_x_cost;
	}
}


int rgb2gray(unsigned char *src_data, int src_w, int src_h, unsigned char *des_data)
{
        int i, size;
        unsigned int r, g, b, val;
        unsigned char *ptr, *des_ptr;

        size = src_w * src_h;
        ptr = src_data;
        des_ptr = des_data;
        for(i = 0; i < size; i++){
                r = (unsigned int)*ptr++;
                g = (unsigned int)*ptr++;
                b = (unsigned int)*ptr++;
                val = (r * 9794);
                val += (g * 19235);
                val += (b * 3736);
                *des_ptr++ = (val>>BIT_SHIFT);
        }

        return 0;
}

void DrawRoiForFace(int iFaceNum, unsigned char *pData, int Wd, int Ht, struct SMyRect *pFaces) {
        int i, j, k, xx, yy;
        //int Wd, Ht;
        //Wd = pImg->m_nWidth; Ht = pImg->m_nHeight;

        for (i = 0 ; i < iFaceNum ; i++){
                // y-drawing for face
                for (j = pFaces[i].top ; j <= pFaces[i].bottom ; j++){
                        for (k=0; k<3; k++)
                        {
                                xx = (pFaces[i].left+k);

                                pData[j*Wd*3 + xx*3 + 2] = 255;
                                pData[j*Wd*3 + xx*3 + 1] = 255;
                                pData[j*Wd*3 + xx*3 + 0] = 0;

                                xx = (pFaces[i].left-k) + ( pFaces[i].right - pFaces[i].left );
                                pData[j*Wd*3 + xx*3 + 2] = 255;
                                pData[j*Wd*3 + xx*3 + 1] = 255;
                                pData[j*Wd*3 + xx*3 + 0] = 0;
                        }
                }
                // x-drawing for face
                for (j = pFaces[i].left ; j <= pFaces[i].right ; j++){
                        for (k=0; k<3; k++)
                        {
                                yy = (pFaces[i].top+k);
                                pData[yy*Wd*3 + j*3 + 2] = 255;
                                pData[yy*Wd*3 + j*3 + 1] = 255;
                                pData[yy*Wd*3 + j*3 + 0] = 0;

                                yy = (pFaces[i].top-k) + ( pFaces[i].bottom - pFaces[i].top );
                                pData[yy*Wd*3 + j*3 + 2] = 255;
                                pData[yy*Wd*3 + j*3 + 1] = 255;
                                pData[yy*Wd*3 + j*3 + 0] = 0;
                        }
                }

        }
}

void DrawRoiForFaceGray(int iFaceNum, unsigned char *pData, int Wd, int Ht, struct SMyRect *pFaces) {
        int i, j, k, xx, yy;
        //int Wd, Ht;
        //Wd = pImg->m_nWidth; Ht = pImg->m_nHeight;

        for (i = 0 ; i < iFaceNum ; i++){
                // y-drawing for face
                for (j = pFaces[i].top ; j <= pFaces[i].bottom ; j++){
                        for (k=0; k<3; k++)
                        {
                                xx = (pFaces[i].left+k);

                                pData[j*Wd + xx + 0] = 255;

                                xx = (pFaces[i].left-k) + ( pFaces[i].right - pFaces[i].left );
                                pData[j*Wd + xx + 0] = 255;
                        }
                }
                // x-drawing for face
                for (j = pFaces[i].left ; j <= pFaces[i].right ; j++){
                        for (k=0; k<3; k++)
                        {
                                yy = (pFaces[i].top+k);
                                pData[yy*Wd + j + 0] = 255;

                                yy = (pFaces[i].top-k) + ( pFaces[i].bottom - pFaces[i].top );
                                pData[yy*Wd + j + 0] = 255;
                        }
                }

        }
}

int AddBorder(unsigned char *raw_data, int src_w, int src_h, unsigned char *des_data, int border_size)
{
        int i;
        int des_w, des_h;
        int size;

        unsigned char *src_ptr, *des_ptr;

        if(!raw_data) return -1;
        if(!des_data) return -1;
        if(border_size < 0) return -1;

        des_w = src_w + (border_size<<1);
        des_h = src_h + (border_size<<1);
        size = des_w * des_h;

        memset(des_data, 0, sizeof(unsigned char) * size);

        src_ptr = raw_data;
        des_ptr = des_data + des_w * border_size + border_size;
        for(i = 0; i < src_h; i++){
                memcpy(des_ptr, src_ptr, sizeof(unsigned char) * src_w);
                src_ptr += src_w;
                des_ptr += des_w;
        }

        return 0;
}

int Rotate_90(unsigned char *src_data, int src_w, int src_h, int angle, unsigned char *des_data)
{
        int i, j;

        unsigned char *src_ptr, *des_ptr1, *des_ptr2;

        if(angle > 0){
                src_ptr = src_data;
                des_ptr1 = des_data + src_h - 1;
                des_ptr2 = des_ptr1;
                for(i = 0; i < src_h; i++){
                        for(j = 0; j < src_w; j++){
                                *des_ptr1 = *src_ptr++;
                                des_ptr1 += src_h;
                        }
                        des_ptr2--;
                        des_ptr1 = des_ptr2;
                }
        }
        else{
                src_ptr = src_data;
                des_ptr1 = des_data + (src_w - 1) * src_h + 1;
                des_ptr2 = des_ptr1;

                for(i = 0; i < src_h; i++){
                        for(j = 0; j < src_w; j++){
                                *des_ptr1 = *src_ptr++;
                                des_ptr1 -= src_h;
                        }
                        des_ptr2++;
                        des_ptr1 = des_ptr2;
                }
        }

        return 0;
}

