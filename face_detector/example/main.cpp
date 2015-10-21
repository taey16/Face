/** main.cpp : 
* example code for object detector
*/

#include "object_detector.hpp"
#include "face_detector.h"
#include "ImageProcessing.h"

#include <sys/types.h>
#include <sys/time.h> 

#include <string>
#include <vector>

#include <fstream>
using namespace std;

int main(int argc, char* argv[])
{
	int	 			w, h, c;
	double				start_time, end_time;
	unsigned char 			*raw_img = NULL, *gray_raw_img = NULL;
	struct timeval 			start_, end_;
	std::string 			face_config_filename;

	std::vector<Daum::CObject>	face_list;
	Daum::CFaceDetector		*face_detector = NULL;
	IplImage 			*img = NULL, *rgb_img = NULL, *gray_img = NULL;
	CImageProcessing 		ip;
	face_config_filename	= std::string("../conf/face.conf");

	img = ip.load(argv[1]);
	ip.size(img, &w, &h, &c);
	rgb_img = ip.create(w, h, c);
	cvCvtColor(img, rgb_img, CV_BGR2RGB);

	raw_img = (unsigned char *)malloc(sizeof(unsigned char) * w * h * c);
	memset(raw_img, 0, sizeof(unsigned char) * w * h * c);
	ip.getraw_notnull(img, raw_img);
	std::cout << "Image Size: " << "width: " << w << " height: " << h << std::endl;

	gray_img = ip.create(w, h, 1);
	ip.bgr2gray(img, gray_img);

	gray_raw_img = (unsigned char *)malloc(sizeof(unsigned char) * w * h);
	memset(gray_raw_img, 0, sizeof(unsigned char) * w * h);
	ip.getraw_notnull(gray_img, gray_raw_img);

	/*********************/
	face_detector = new Daum::CFaceDetector;
	face_detector->SetConfigureFile(face_config_filename);
	face_detector->SetDetector();
	
	gettimeofday(&start_, NULL);
	face_detector->Detect(gray_raw_img, w, h, face_list);
	gettimeofday(&end_, NULL);

	start_time = start_.tv_sec * 1000.0 + start_.tv_usec / 1000.0;	
	end_time = end_.tv_sec * 1000.0 + end_.tv_usec / 1000.0;  

	std::cout << "Detector Type: " << face_detector->GetType();
	fprintf(stdout, "Elapsed Time (ms): %lf\n", end_time - start_time);
	fprintf(stdout, "\tDetected # of Object: %d\n", (int)face_list.size());

	string rect_file = string(argv[1]) + ".rect.txt";
	ofstream fout(rect_file.c_str(), ios::out);
	fout << face_list.size() << endl;
	for(unsigned int idx1 = 0; idx1 < face_list.size(); idx1++){
		fout << face_list[idx1].x << ' ' << face_list[idx1].y << ' ' << face_list[idx1].width << ' ' << face_list[idx1].height << ' ' << face_list[idx1].angle << endl;
		CvPoint	p1, p2;

		p1.x = face_list[idx1].x;
		p1.y = face_list[idx1].y;
		p2.x = face_list[idx1].width + face_list[idx1].x;
		p2.y = face_list[idx1].height + face_list[idx1].y;

		std::cout << face_list[idx1] << std::endl;

		cvDrawRect(img, p1, p2, cvScalar(0,0,255), 2);
	}
	ip.save("face_result.jpg", img);

	if(face_detector) delete face_detector;
	face_detector = NULL;
	
	ip.destroy(&img);
	ip.destroy(&gray_img);

	if(raw_img) free(raw_img);
	raw_img = NULL;
	if(gray_raw_img) free(gray_raw_img);
	gray_raw_img = NULL;

	return 0;
}

