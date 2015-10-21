/*
 * image_processing.cpp
 *
 *  Created on: 2013. 11. 10.
 *      Author: minu, hyunchul
 */

#include "image_processing.h"
#include <stdint.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "stop_watch.h"

namespace daum {

// http://tech-algorithm.com/articles/bilinear-image-scaling/
Image resize(const Image& src, Size dsize)
{
	float x_ratio = (float) (src.cols - 1) / dsize.width;
	float y_ratio = (float) (src.rows - 1) / dsize.height;

	Image res(dsize.height, dsize.width);
	uchar* dst = res.ptr();
	for ( int i=0 ; i<res.rows ; ++i )
	{
		float y_diff = y_ratio * i;
		int y = (int)y_diff;
		y_diff = y_diff - y;
		for ( int j=0 ; j<res.cols ; ++j )
		{
			float x_diff = x_ratio * j;
			int x = (int)x_diff;
			x_diff = x_diff - x;

			uchar val = src.at(y, x) * (1 - x_diff) * (1 - y_diff)
					+ src.at(y, x+1) * x_diff * (1 - y_diff)
					+ src.at(y+1, x) * y_diff * (1 - x_diff)
					+ src.at(y+1, x+1) * x_diff * y_diff;

			*dst++ = val;
		}
	}
	return res;
}

// http://tech-algorithm.com/articles/bilinear-image-scaling/
#define UPSHIFT 20
Image resize_fast(const Image& src, Size dsize)
{
	int x_ratio = ((src.cols - 1) << UPSHIFT) / dsize.width;
	int y_ratio = ((src.rows - 1) << UPSHIFT) / dsize.height;

	Image res(dsize.height, dsize.width);
	uchar* dst = res.ptr();

	uint64_t y = 0;
	for ( int i=0 ; i<res.rows ; ++i )
	{
		int yr = int(y >> UPSHIFT);
		uint64_t y_diff = y - (yr << UPSHIFT);
		uint64_t one_min_y_diff = (1<<UPSHIFT) - y_diff;
		uint64_t x = 0;
		for ( int j=0 ; j<res.cols ; ++j )
		{
			int xr = (int)(x >> UPSHIFT);
			uint64_t x_diff = x - (xr << UPSHIFT);
			uint64_t one_min_x_diff = (1<<UPSHIFT) - x_diff;

			uchar val = (src.at(yr, xr) * one_min_x_diff * one_min_y_diff
					+ src.at(yr, xr+1) * x_diff * one_min_y_diff
					+ src.at(yr+1, xr) * y_diff * one_min_x_diff
					+ src.at(yr+1, xr+1) * x_diff * y_diff) >> (UPSHIFT<<1);

			*dst++ = val;
			x += x_ratio;
		}
		y += y_ratio;
	}
	return res;
}

Image convert_gray(const uchar* src, int rows, int cols, bool BGR)
{
    static const int yuv_shift = 14;
    static const int tab[] = {0,1868,3736,5604,7472,9340,11208,13076,14944,16812,18680,20548,22416,24284,26152,28020,29888,31756,33624,35492,37360,39228,41096,42964,44832,46700,48568,50436,52304,54172,56040,57908,59776,61644,63512,65380,67248,69116,70984,72852,74720,76588,78456,80324,82192,84060,85928,87796,89664,91532,93400,95268,97136,99004,100872,102740,104608,106476,108344,110212,112080,113948,115816,117684,119552,121420,123288,125156,127024,128892,130760,132628,134496,136364,138232,140100,141968,143836,145704,147572,149440,151308,153176,155044,156912,158780,160648,162516,164384,166252,168120,169988,171856,173724,175592,177460,179328,181196,183064,184932,186800,188668,190536,192404,194272,196140,198008,199876,201744,203612,205480,207348,209216,211084,212952,214820,216688,218556,220424,222292,224160,226028,227896,229764,231632,233500,235368,237236,239104,240972,242840,244708,246576,248444,250312,252180,254048,255916,257784,259652,261520,263388,265256,267124,268992,270860,272728,274596,276464,278332,280200,282068,283936,285804,287672,289540,291408,293276,295144,297012,298880,300748,302616,304484,306352,308220,310088,311956,313824,315692,317560,319428,321296,323164,325032,326900,328768,330636,332504,334372,336240,338108,339976,341844,343712,345580,347448,349316,351184,353052,354920,356788,358656,360524,362392,364260,366128,367996,369864,371732,373600,375468,377336,379204,381072,382940,384808,386676,388544,390412,392280,394148,396016,397884,399752,401620,403488,405356,407224,409092,410960,412828,414696,416564,418432,420300,422168,424036,425904,427772,429640,431508,433376,435244,437112,438980,440848,442716,444584,446452,448320,450188,452056,453924,455792,457660,459528,461396,463264,465132,467000,468868,470736,472604,474472,476340,0,9617,19234,28851,38468,48085,57702,67319,76936,86553,96170,105787,115404,125021,134638,144255,153872,163489,173106,182723,192340,201957,211574,221191,230808,240425,250042,259659,269276,278893,288510,298127,307744,317361,326978,336595,346212,355829,365446,375063,384680,394297,403914,413531,423148,432765,442382,451999,461616,471233,480850,490467,500084,509701,519318,528935,538552,548169,557786,567403,577020,586637,596254,605871,615488,625105,634722,644339,653956,663573,673190,682807,692424,702041,711658,721275,730892,740509,750126,759743,769360,778977,788594,798211,807828,817445,827062,836679,846296,855913,865530,875147,884764,894381,903998,913615,923232,932849,942466,952083,961700,971317,980934,990551,1000168,1009785,1019402,1029019,1038636,1048253,1057870,1067487,1077104,1086721,1096338,1105955,1115572,1125189,1134806,1144423,1154040,1163657,1173274,1182891,1192508,1202125,1211742,1221359,1230976,1240593,1250210,1259827,1269444,1279061,1288678,1298295,1307912,1317529,1327146,1336763,1346380,1355997,1365614,1375231,1384848,1394465,1404082,1413699,1423316,1432933,1442550,1452167,1461784,1471401,1481018,1490635,1500252,1509869,1519486,1529103,1538720,1548337,1557954,1567571,1577188,1586805,1596422,1606039,1615656,1625273,1634890,1644507,1654124,1663741,1673358,1682975,1692592,1702209,1711826,1721443,1731060,1740677,1750294,1759911,1769528,1779145,1788762,1798379,1807996,1817613,1827230,1836847,1846464,1856081,1865698,1875315,1884932,1894549,1904166,1913783,1923400,1933017,1942634,1952251,1961868,1971485,1981102,1990719,2000336,2009953,2019570,2029187,2038804,2048421,2058038,2067655,2077272,2086889,2096506,2106123,2115740,2125357,2134974,2144591,2154208,2163825,2173442,2183059,2192676,2202293,2211910,2221527,2231144,2240761,2250378,2259995,2269612,2279229,2288846,2298463,2308080,2317697,2327314,2336931,2346548,2356165,2365782,2375399,2385016,2394633,2404250,2413867,2423484,2433101,2442718,2452335,8192,13091,17990,22889,27788,32687,37586,42485,47384,52283,57182,62081,66980,71879,76778,81677,86576,91475,96374,101273,106172,111071,115970,120869,125768,130667,135566,140465,145364,150263,155162,160061,164960,169859,174758,179657,184556,189455,194354,199253,204152,209051,213950,218849,223748,228647,233546,238445,243344,248243,253142,258041,262940,267839,272738,277637,282536,287435,292334,297233,302132,307031,311930,316829,321728,326627,331526,336425,341324,346223,351122,356021,360920,365819,370718,375617,380516,385415,390314,395213,400112,405011,409910,414809,419708,424607,429506,434405,439304,444203,449102,454001,458900,463799,468698,473597,478496,483395,488294,493193,498092,502991,507890,512789,517688,522587,527486,532385,537284,542183,547082,551981,556880,561779,566678,571577,576476,581375,586274,591173,596072,600971,605870,610769,615668,620567,625466,630365,635264,640163,645062,649961,654860,659759,664658,669557,674456,679355,684254,689153,694052,698951,703850,708749,713648,718547,723446,728345,733244,738143,743042,747941,752840,757739,762638,767537,772436,777335,782234,787133,792032,796931,801830,806729,811628,816527,821426,826325,831224,836123,841022,845921,850820,855719,860618,865517,870416,875315,880214,885113,890012,894911,899810,904709,909608,914507,919406,924305,929204,934103,939002,943901,948800,953699,958598,963497,968396,973295,978194,983093,987992,992891,997790,1002689,1007588,1012487,1017386,1022285,1027184,1032083,1036982,1041881,1046780,1051679,1056578,1061477,1066376,1071275,1076174,1081073,1085972,1090871,1095770,1100669,1105568,1110467,1115366,1120265,1125164,1130063,1134962,1139861,1144760,1149659,1154558,1159457,1164356,1169255,1174154,1179053,1183952,1188851,1193750,1198649,1203548,1208447,1213346,1218245,1223144,1228043,1232942,1237841,1242740,1247639,1252538,1257437};
    /*
    static const int R2Y = 4899;
	static const int G2Y = 9617;
	static const int B2Y = 1868;

    int b = 0, g = 0, r = (1 << (yuv_shift-1));
    int db = BGR ? B2Y : R2Y;
    int dg = 9617;
    int dr = BGR ? R2Y : B2Y;
    int tab[256*3];
    for( int i = 0; i < 256; i++, b += db, g += dg, r += dr )
    {
        tab[i] = b;
        tab[i+256] = g;
        tab[i+512] = r;
    }
    for ( int i=0 ; i<256*3 ; ++i )
    {
    	printf("%d,", tab[i]);
    }
    */

    const int* _tab = tab;
    Image res(rows, cols);
    uchar* dst = res.ptr();
    int n = rows * cols;
    for(int i = 0; i < n ; i++, src += 3)
    {
        dst[i] = (uchar)((_tab[src[0]] + _tab[src[1]+256] + _tab[src[2]+512]) >> yuv_shift);
    }
    return res;
}

Mat get_affine_transform_matrix(float scale_x, float scale_y, float theta_radian, Point2f& offset, Point2f& t)
{
#ifdef __USE_DAUMCV__
	Mat M(3, 3);
#else
	Mat M(3, 3, CV_32F);
#endif
	// M = T * R * S
	//  [X] = [[scale_x*cos(theta) -scale_y*sin(theta)]   t.x] * [x - offset.x] -> M = [[scale_x*cos(theta) -scale_y*sin(theta)]  t.x - scale_x*cos(theta)*offset.x + scale_y*sin(theta)*offset.y ] * [x]
	//  [Y]   [[scale_x*sin(theta)  scale_y*cos(theta)]   t.y]   [y - offset.y]        [[scale_x*sin(theta)  scale_y*cos(theta)]  t.y - scale_x*sin(theta)*offset.x - scale_y*cos(theta)*offset.y ]   [y]
	//	      [           0              0                1  ]                         [           0           0                  1                                                               ]
	M.at<float>(0,0) = scale_x * cosf(theta_radian);
	M.at<float>(1,0) = scale_x * sinf(theta_radian);
	M.at<float>(2,0) = 0.0f;
	M.at<float>(0,1) = -scale_y * sinf(theta_radian);
	M.at<float>(1,1) = scale_y * cosf(theta_radian);
	M.at<float>(2,1) = 0.0f;
	M.at<float>(0,2) = t.x - scale_x*cosf(theta_radian)*offset.x + scale_y*sinf(theta_radian)*offset.y;
	M.at<float>(1,2) = t.y - scale_x*sinf(theta_radian)*offset.x - scale_y*cosf(theta_radian)*offset.y;
	M.at<float>(2,2) = 1.0f;

	return M;
}


Mat get_transform_matrix(vector<Point2f>& x, vector<Point2f>& y, int DOF)
{
#ifdef __USE_DAUMCV__
	Mat Y(2, y.size());
	Mat A(2*x.size(), DOF);
	Mat M(3, 3);
#else
	Mat Y(2*y.size(), 1, CV_32F);
	Mat A(2*x.size(), DOF, CV_32F);
	Mat M(3, 3, CV_32F);
#endif

	for(int j=0; j<Y.rows; j+=2)
	{
		Y.at<float>(j,0) = y[j>>1].x;
		Y.at<float>(j+1,0) = y[j>>1].y;
	}

	if(DOF == 8)// perspective
	{
		//M = [ a b c ]
		//    [ d e f ]
		//	  [ g h 1 ]
		//Y = M*X -> y1 = (ax1 + bx2 + c)/(gx1 + hx2 + 1), y2 = (dx1 + ex2 + f)/(gx1 + hx2 + 1);
		// x1a + x2b + c - x1y1g - x2y1h = y1 -> [x1 x2 1 0  0  0 -x1y1 -x2y1] * [a b c d e f g h]' = [y1] -> A * p = Y -> p = (A'A)^-1 * A'Y
		// x1d + x2e + f - x1y2g - x2y2h = y2 -> [0  0  0 x1 x2 1 -x1y2 -x2y2]                        [y2]

		for(int i=0; i<A.rows; i+=2)
		{
			A.at<float>(i  ,0) = x[i>>1].x;
			A.at<float>(i  ,1) = x[i>>1].y;
			A.at<float>(i  ,2) = 1;
			A.at<float>(i  ,3) = 0;
			A.at<float>(i  ,4) = 0;
			A.at<float>(i  ,5) = 0;
			A.at<float>(i  ,6) = -x[i>>1].x * y[i>>1].x;
			A.at<float>(i  ,7) = -x[i>>1].y * y[i>>1].x;
			A.at<float>(i+1,0) = 0;
			A.at<float>(i+1,1) = 0;
			A.at<float>(i+1,2) = 0;
			A.at<float>(i+1,3) = x[i>>1].x;
			A.at<float>(i+1,4) = x[i>>1].y;
			A.at<float>(i+1,5) = 1;
			A.at<float>(i+1,6) = -x[i>>1].x * y[i>>1].y;
			A.at<float>(i+1,7) = -x[i>>1].y * y[i>>1].y;
		}

		Mat m = (A.t()*A).inv()*A.t()*Y;
		M.at<float>(0,0) = m.at<float>(0,0);
		M.at<float>(0,1) = m.at<float>(1,0);
		M.at<float>(0,2) = m.at<float>(2,0);
		M.at<float>(1,0) = m.at<float>(3,0);
		M.at<float>(1,1) = m.at<float>(4,0);
		M.at<float>(1,2) = m.at<float>(5,0);
		M.at<float>(2,0) = m.at<float>(6,0);
		M.at<float>(2,1) = m.at<float>(7,0);
		M.at<float>(2,2) = 1.0f;
	}
	else if(DOF == 6)// affine
	{
		//M = [ a b c ]
		//    [ d e f ]
		//	  [ 0 0 1 ]
		//Y = M*X -> y1 = (ax1 + bx2 + c), y2 = (dx1 + ex2 + f);
		// x1a + x2b + c = y1 -> [x1 x2 1 0  0  0] * [a b c d e f]' = [y1] -> A * p = Y -> p = (A'A)^-1 * A'Y
		// x1d + x2e + f = y2 -> [0  0  0 x1 x2 1]                    [y2]

		for(int i=0; i<A.rows; i+=2)
		{
			A.at<float>(i  ,0) = x[i>>1].x;
			A.at<float>(i  ,1) = x[i>>1].y;
			A.at<float>(i  ,2) = 1;
			A.at<float>(i  ,3) = 0;
			A.at<float>(i  ,4) = 0;
			A.at<float>(i  ,5) = 0;
			A.at<float>(i+1,0) = 0;
			A.at<float>(i+1,1) = 0;
			A.at<float>(i+1,2) = 0;
			A.at<float>(i+1,3) = x[i>>1].x;
			A.at<float>(i+1,4) = x[i>>1].y;
			A.at<float>(i+1,5) = 1;
		}

		Mat m = (A.t()*A).inv()*A.t()*Y;
		M.at<float>(0,0) = m.at<float>(0,0);
		M.at<float>(0,1) = m.at<float>(1,0);
		M.at<float>(0,2) = m.at<float>(2,0);
		M.at<float>(1,0) = m.at<float>(3,0);
		M.at<float>(1,1) = m.at<float>(4,0);
		M.at<float>(1,2) = m.at<float>(5,0);
		M.at<float>(2,0) = 0.0f;
		M.at<float>(2,1) = 0.0f;
		M.at<float>(2,2) = 1.0f;
	}
	else if(DOF == 4)// similarity
	{
		//M = [ a -b c ]
		//    [ b  a d ]
		//	  [ 0  0 1 ]
		//Y = M*X -> y1 = (ax1 - bx2 + c), y2 = (bx1 + ax2 + d);
		// x1a - x2b + c = y1 -> [x1 -x2 1 0] * [a b c d]' = [y1] -> A * p = Y -> p = (A'A)^-1 * A'Y
		// x1b + x2a + d = y2 -> [x2  x1 0 1]                [y2]

		for(int i=0; i<A.rows; i+=2)
		{
			A.at<float>(i  ,0) = x[i>>1].x;
			A.at<float>(i  ,1) = -x[i>>1].y;
			A.at<float>(i  ,2) = 1;
			A.at<float>(i  ,3) = 0;
			A.at<float>(i+1,0) = x[i>>1].y;
			A.at<float>(i+1,1) = x[i>>1].x;
			A.at<float>(i+1,2) = 0;
			A.at<float>(i+1,3) = 1;
		}

		Mat m = (A.t()*A).inv()*A.t()*Y;
		M.at<float>(0,0) = m.at<float>(0,0);
		M.at<float>(0,1) = -m.at<float>(1,0);
		M.at<float>(0,2) = m.at<float>(2,0);
		M.at<float>(1,0) = m.at<float>(1,0);
		M.at<float>(1,1) = m.at<float>(0,0);
		M.at<float>(1,2) = m.at<float>(3,0);
		M.at<float>(2,0) = 0.0f;
		M.at<float>(2,1) = 0.0f;
		M.at<float>(2,2) = 1.0f;

//		float *ptr = (float*)M.ptr();
//		printf("%f %f %f\n%f %f %f\n %f %f %f\n", *(ptr+0),*(ptr+1),*(ptr+2),*(ptr+3),*(ptr+4),*(ptr+5),*(ptr+6),*(ptr+7),*(ptr+8));
	}

	return M;
}

Image flip(const Image& src, bool flag)
{
	Image image(src.rows, src.cols);
	if(flag)	// horizontal flip
	{
		for(int i=0; i<src.rows; i++)
			for(int j=0; j<src.cols; j++)
				image.at(i,j) = src.at(i,src.cols-1-j);
	}
	else		// vertical flip
	{
		for(int j=0; j<src.cols; j++)
			for(int i=0; i<src.rows; i++)
				image.at(i,j) = src.at(src.rows-1-i,j);
	}

	return image;
}

Image transform(const Image& src, Mat& M)
{
	// [X] = M * [x]
	// [Y]       [y]
	// [Z]       [1]
	// X = X/Z
	// Y = Y/Z

	Point2f new_x, x;
	float x_diff, y_diff, x_diff_, y_diff_;
	int x1, y1, x2, y2;

	Image image(src.size().height, src.size().width);
	uchar *val_ptr = image.ptr();
	for(int i=0; i<src.size().height; i++)
	{
		for(int j=0; j<src.size().width; j++)
		{
			x.x = j;
			x.y = i;
			new_x = perturb_point(x, M);

			x1 = (int)(new_x.x);
			y1 = (int)(new_x.y);
			x2 = x1 + 1;
			y2 = y1 + 1;
			x_diff = new_x.x - x1;
			y_diff = new_x.y - y1;
			x_diff_ = 1- x_diff;
			y_diff_ = 1- y_diff;

			if ( x1>=0 && y1>=0 && y2<src.rows && x2<src.cols )
			{
				*val_ptr = src.at(y1, x1) * x_diff_ * y_diff_
					+ src.at(y1, x1+1) * x_diff * y_diff_
					+ src.at(y1+1, x1) * x_diff_ * y_diff
					+ src.at(y1+1, x1+1) * x_diff * y_diff;

			}
			else
			{
				*val_ptr = 0;
			}
			val_ptr++;
		}
	}
	return image;
}

Image transform_fast(const Image& src, Mat& M, int width, int height)
{
	int scale_factor = (1<<UPSHIFT);
	int64_t M_[3][3];
	for(int i=0; i<M.rows; i++)
	{
		for(int j=0; j<M.cols; j++)
		{
			M_[i][j] = (int64_t)(M.at<float>(i,j) * scale_factor);
		}
	}

	// [X] = M * [x]
	// [Y]       [y]
	// [Z]       [1]
	// X = X/Z
	// Y = Y/Z

	int64_t u, v, x_diff, y_diff, x_diff_, y_diff_;
	int64_t x1, y1, x2, y2;
	int64_t coord[3];

	if(width == 0) width = src.size().width;
	if(height == 0) height = src.size().height;
	Image image(height, width);
	uchar *val_ptr = image.ptr();
	for(int i=0; i<height; i++)
	{
		for(int j=0; j<width; j++)
		{
			coord[0] = (M_[0][0] * j + M_[0][1] * i + M_[0][2]);
			coord[1] = (M_[1][0] * j + M_[1][1] * i + M_[1][2]);
			coord[2] = (M_[2][0] * j + M_[2][1] * i + M_[2][2]);

			u = (coord[0] * scale_factor) / coord[2];
			v = (coord[1] * scale_factor) / coord[2];
			x1 = (u >> UPSHIFT);
			y1 = (v >> UPSHIFT);
			x2 = x1 + 1;
			y2 = y1 + 1;
			x_diff = u - (x1 * scale_factor);
			y_diff = v - (y1 * scale_factor);
			x_diff_ = scale_factor - x_diff;
			y_diff_ = scale_factor - y_diff;

			if ( x1>=0 && y1>=0 && y2<src.rows && x2<src.cols )
			{
				*val_ptr = (src.at(y1, x1) * x_diff_ * y_diff_
					+ src.at(y1, x1+1) * x_diff * y_diff_
					+ src.at(y1+1, x1) * x_diff_ * y_diff
					+ src.at(y1+1, x1+1) * x_diff * y_diff)>>(UPSHIFT<<1);
			}
			else
			{
				*val_ptr = 0;
			}
			val_ptr++;
		}
	}

	return image;
}

Image get_patch(const Image& image, const Point& center, Size size)
{
	int half_width = (size.width >> 1);
	int half_height = (size.height >> 1);
	int x1 = std::max(0, center.x - half_width);
	int y1 = std::max(0, center.y - half_height);
	int x2 = std::min(image.cols, center.x + half_width);
	int y2 = std::min(image.rows, center.y + half_height);

	Image patch(size.height, size.width);
	// x1 <= x < x2 and y1 <= y < y2;
	if ( x2 <= 0 || y2 <= 0 || x1 >= image.cols || y1 >= image.rows ) return patch;

	int start_col = 0;
	int start_row = 0;
	int end_col = patch.cols;
	int end_row = patch.rows;

	// TODO
	if ( x1 == 0 )
	{
		start_col = half_width - center.x;
	}
	if ( y1 == 0 )
	{
		start_row = half_height - center.y;
	}
	if ( x2 == image.cols )
	{
		end_col = start_col + (x2 - x1);
	}
	if ( y2 == image.rows )
	{
		end_row = start_row + (y2 - y1);
	}

	int type_size = sizeof(image.at(0,0));
	for ( int i=y1, j=start_row ; i<y2 && j<end_row ; ++i, ++j )
	{
		memcpy(patch.ptr(j)+start_col, image.ptr(i)+x1, (x2 - x1) * type_size);
	}
	return patch;
}


void preparation(\
		Image& processed_image, Mat& transform_matrix,\
		Image& image, Rect& rect, int cannonical_face_h, int cannonical_face_v)
{
	// crop and resize image to fit the face into canonical size
	// step1. get patch
	Point center_p(rect.x + (rect.width>>1), rect.y + (rect.height>>1));
	Image image_patch = get_patch(image, center_p, Size((rect.width<<1), (rect.height<<1)));

	// step2. resize into canonical size
	// TODO: implement resize function
	StopWatch resize_time;
	resize_time.start();
	// step2. resize into canonical size
	if(image_patch.rows > (1<<(31-UPSHIFT)) || image_patch.cols > (1<<(31-UPSHIFT)))
	{
		processed_image = resize(image_patch, Size((cannonical_face_h<<1), (cannonical_face_v<<1)));
	}
	else
	{
		processed_image = resize_fast(image_patch, Size((cannonical_face_h<<1), (cannonical_face_v<<1)));
	}
	resize_time.stop();

#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "FACIAL_FEATURE",	"Resize Time = %.3f", resize_time.elapsed_msecs());
#else

#endif

	// step3. transform matrix: from image to processed_image
	float scale_x = (float)processed_image.size().width/image_patch.size().width;
	float scale_y = (float)processed_image.size().height/image_patch.size().height;
	Point2f offset(center_p.x, center_p.y);
	Point2f t(processed_image.size().width>>1, processed_image.size().height>>1);
	transform_matrix = get_affine_transform_matrix(scale_x, scale_y, 0.0f, offset, t);
}

void perturb_shape(vector<Point2f>& new_shape, vector<Point2f>& shape, Mat& M)
{
	vector<Point2f> shape_temp;
	shape_temp.reserve(shape.size());
	for(unsigned int i=0; i<shape.size(); i++)
	{
		shape_temp.push_back(perturb_point(shape[i], M));
	}
	new_shape.swap(shape_temp);
}

Point2f perturb_point(Point2f& p, Mat& M)
{
#ifdef __USE_DAUMCV__
	Mat p_mat(3,1);
#else
	Mat p_mat(3,1,CV_32F);
#endif
	p_mat.at<float>(0,0) = p.x;
	p_mat.at<float>(1,0) = p.y;
	p_mat.at<float>(2,0) = 1.0f;

	Mat p_new_mat = M * p_mat;

	Point2f p_new;
	p_new.x = p_new_mat.at<float>(0,0)/p_new_mat.at<float>(2,0);
	p_new.y = p_new_mat.at<float>(1,0)/p_new_mat.at<float>(2,0);

	return p_new;
}

};
