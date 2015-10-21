#include <iostream>
#include <fstream>

#include "face_detector.h"
#include "facial_feature_detector.h"
#include "image_processing.h" // transform(); facial_feature_detector/include
#include "typedef.h"
#include "stop_watch.h"

#include "CommonUtil.hpp"

#include <cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace daum;

int main(int argc, char** argv)
{
	// set ffd conf file
	string conf_file  = "../conf/ffd.conf";
	string model_file = "";
	FacialFeatureDetector ffd;
	const int resized = 128;

	// get ffd model filename
	ifstream fin(conf_file);
	if ( !fin.fail() ) {
		fin >> model_file;
		cout << "SUCCESS Loading model file... " << model_file << endl;
	} else {
		cout<< "ERROR loading model file: " << model_file << endl;
		exit(-1);
	}
	// load ffd model
	string version;
	ffd.load_model(model_file, &version, true);
	printf("Processing %d images...\n", argc-1);

	// set Face detector
	Daum::CFaceDetector face_detector;
	face_detector.SetConfigureFile("../conf/face.conf");
	face_detector.SetDetector();

	for ( int i=1 ; i<argc ; ++i )
	{
		// get original image
		string filepath(argv[i]);
		printf("[%3d] %s\n", i, filepath.c_str());
		cv::Mat image = cv::imread(filepath);
		cv::Mat mat_gray, mat_color;
		cv::cvtColor(image, mat_gray, CV_BGR2GRAY);
		cv::cvtColor(image, mat_color, CV_BGR2RGB);
		// split color 3 channels into R, G, B channel, respectively
		std::vector<cv::Mat> mat_RGB(3);
		cv::split(mat_color, mat_RGB);
		// convert cv::Mat to Image (gray Image for fd, ffd)
		Image img_gray(mat_gray.rows, mat_gray.cols, mat_gray.ptr());
		// convert cv::Mat to Image (R, G, B, for output aligned-cropped face image)
		Image img_R(mat_RGB[0].rows, mat_RGB[0].cols, mat_RGB[0].ptr());
		Image img_G(mat_RGB[1].rows, mat_RGB[1].cols, mat_RGB[1].ptr());
		Image img_B(mat_RGB[2].rows, mat_RGB[2].cols, mat_RGB[2].ptr());
		// init. output detected faceList
		std::vector<Daum::CObject> face_list;

		// do fd
		StopWatch detection_time;
		detection_time.start();
		face_detector.Detect(mat_gray.ptr(), mat_gray.cols, mat_gray.rows, face_list);
		printf("Face Detection Time: %d ms\n", (int)detection_time.elapsed_msecs());

		// write output of fd into file
		//string result_imgname(filepath + ".ffd.jpg");
		//string result_ptsname(filepath + ".ffd.txt");
		//ofstream ptsout(result_ptsname.c_str());
		//ptsout << face_list.size() << endl;

		// for each detected face area
		for ( int i=0 ; i<(int)face_list.size() ; ++i )
		{
			puts("------------------------------------------------------------------------");
			printf("Face: x=%d, y=%d, width=%d, height=%d, angle=%d\n",
				face_list[i].x, face_list[i].y, face_list[i].width, face_list[i].height, face_list[i].angle);
			// get Rect of detected face area in original Image gray
			Rect face_rect = Rect(face_list[i].x, face_list[i].y, face_list[i].width, face_list[i].height);
			
			// do ffd
			StopWatch ffd_time;
			ffd_time.start();
			float confidence = ffd.detect(img_gray, face_rect, face_list[i].angle);
			printf("Facial Feature Detection Time: %d ms\n", (int)ffd_time.elapsed_msecs());

			//vector<Point2f> point2f_face(4);
			//point2f_face[0].x = face_list[i].x;
			//point2f_face[0].y = face_list[i].y;
			//point2f_face[1].x = face_list[i].x+face_list[i].width;
			//point2f_face[1].y = face_list[i].y;
			//point2f_face[2].x = face_list[i].x;
			//point2f_face[2].y = face_list[i].y+face_list[i].height;
			//point2f_face[3].x = face_list[i].x+face_list[i].width;
			//point2f_face[3].y = face_list[i].y+face_list[i].height;
			//perturb_shape(point2f_face, point2f_face, ffd.face_motion_matrix_source_to_aligned);
			////Rect aligned_face_rect = Rect(point2f_face[0].x, point2f_face[0].y, point2f_face[3].x - point2f_face[0].x, point2f_face[3].y - point2f_face[0].y);

			// wrap original Image(gray, R, G, B) into aligned 2d ref. space
			Image img_aligned_face_gray = transform_fast( img_gray, ffd.face_motion_matrix_aligned_to_source, img_gray.cols, img_gray.rows );
			Image img_aligned_face_R= transform_fast( img_R, ffd.face_motion_matrix_aligned_to_source, img_R.cols, img_R.rows );
			Image img_aligned_face_G= transform_fast( img_G, ffd.face_motion_matrix_aligned_to_source, img_G.cols, img_G.rows );
			Image img_aligned_face_B= transform_fast( img_B, ffd.face_motion_matrix_aligned_to_source, img_B.cols, img_B.rows );
			// convert aligned Image into cv::Mat
			cv::Mat mat_aligned_face_gray(img_gray.rows, img_gray.cols, CV_8UC1);
			cv::Mat mat_aligned_face_RGB(mat_RGB[0].rows, mat_RGB[0].cols, CV_8UC3);
			for (int r=0; r<img_gray.rows; r++) {
				for(int c=0; c<img_gray.cols; c++) { 
					mat_aligned_face_gray.at<unsigned char>(r,c) = img_aligned_face_gray.at(r,c);
					mat_aligned_face_RGB.at<cv::Vec3b>(r,c)[0]= img_aligned_face_R.at(r,c);
					mat_aligned_face_RGB.at<cv::Vec3b>(r,c)[1]= img_aligned_face_G.at(r,c);
					mat_aligned_face_RGB.at<cv::Vec3b>(r,c)[2]= img_aligned_face_B.at(r,c);
				}
			}
			cv::cvtColor(mat_aligned_face_RGB, mat_aligned_face_RGB, CV_RGB2BGR);

			// get ffd points for each face area
			// facial_feature_points: coordinate for original image coordinate
			// aligned_facial_feature_points: coordinate for 2d ref. image coordinate
			vector<Point2f>& feature_points = ffd.facial_feature_points;
			vector<Point2f>& aligned_feature_points = ffd.aligned_facial_feature_points;

			
			// point index for "model_motion_parameter_58.bin"
			//[0 ~ 4], left-eyeblow (clock-wise)
			//[5 ~ 9], right-eyeblow(clock-wise)
			//[10 ~ 13], nose-line (up to down)
			//[14 ~ 18], nairs (left to right)
			//[19 ~ 24], left-eye (clock-wise)
			//[25 ~ 30], right-eye(clock-wise)
			//[31 ~ 42], lip-outline(clock-wise)
			//[43 ~ 48], lip-inline(clock-wise)
			//[49 ~ 57], jaw (left to right)
			
			// get eyes, lip, nose points
			Point2f mean_left_eye_pt(0, 0);
			Point2f mean_right_eye_pt(0, 0);
			Point2f mean_lip_pt(0, 0);
			std::vector<Point2f> face_boundary_pt(2);
			face_boundary_pt[0].x = 10000;
			face_boundary_pt[0].y = 10000;
			face_boundary_pt[1].x = 0;
			face_boundary_pt[1].y = 0;
			Point2f nose_tip_pt = aligned_feature_points[13];
			for (int p=0; p<(int)aligned_feature_points.size(); p++) {
				if (face_boundary_pt[0].x >= aligned_feature_points[p].x) {
					face_boundary_pt[0].x = aligned_feature_points[p].x;
				}
				if (face_boundary_pt[0].y >= aligned_feature_points[p].y) {
					face_boundary_pt[0].y = aligned_feature_points[p].y;
				}
				if (face_boundary_pt[1].x <= aligned_feature_points[p].x) {
					face_boundary_pt[1].x = aligned_feature_points[p].x;
				}
				if (face_boundary_pt[1].y <= aligned_feature_points[p].y) {
					face_boundary_pt[1].y = aligned_feature_points[p].y;
				}
			}
			for (int p=19; p<25; p++) {
				mean_left_eye_pt.x += aligned_feature_points[p].x;
				mean_left_eye_pt.y += aligned_feature_points[p].y;
				mean_right_eye_pt.x += aligned_feature_points[p+6].x;
				mean_right_eye_pt.y += aligned_feature_points[p+6].y;
			}
			mean_left_eye_pt.x /= 6.;
			mean_left_eye_pt.y /= 6.;
			mean_right_eye_pt.x /= 6.;
			mean_right_eye_pt.y /= 6.;
			for (int p=31; p<43; p++) {
				mean_lip_pt.x += aligned_feature_points[p].x;
				mean_lip_pt.y += aligned_feature_points[p].y;
			}
			mean_lip_pt.x /= 12.;
			mean_lip_pt.y /= 12.;

			// draw detected face area in original color image
			cv::rectangle(image, cv::Rect(face_rect.x, face_rect.y, face_rect.width, face_rect.height), cv::Scalar(0, 0, 255), 2);
			//ptsout << feature_points.size() << endl;
			// draw all ffd points in original color image
			for ( int j=0 ; j<(int)feature_points.size() ; ++j )
			{
				Point2f& pt = feature_points[j];
				cv::Point2f center(pt.x, pt.y);
				cv::circle(image, center, img_gray.cols * 0.005, cv::Scalar(0, 255, 0), -img_gray.cols * 0.005);
				//ptsout << pt.x << ' ' << pt.y << endl;
			}

			// draw eyes, nose, lip points in 2d aligned coordinate
			cv::circle(mat_aligned_face_RGB, (cv::Point2f&)mean_left_eye_pt, mat_aligned_face_gray.cols * 0.005, cv::Scalar(0, 255, 0), -mat_aligned_face_gray.cols * 0.005);
			cv::circle(mat_aligned_face_RGB, (cv::Point2f&)mean_right_eye_pt, mat_aligned_face_gray.cols * 0.005, cv::Scalar(0, 255, 0), -mat_aligned_face_gray.cols * 0.005);
			cv::circle(mat_aligned_face_RGB, (cv::Point2f&)nose_tip_pt, mat_aligned_face_gray.cols * 0.005, cv::Scalar(0, 255, 0), -mat_aligned_face_gray.cols * 0.005);
			cv::circle(mat_aligned_face_RGB, (cv::Point2f&)mean_lip_pt, mat_aligned_face_gray.cols * 0.005, cv::Scalar(0, 255, 0), -mat_aligned_face_gray.cols * 0.005);

			// get aligned cropped face area
			if (face_boundary_pt[0].x-10 < 1) face_boundary_pt[0].x = 11;
			if (face_boundary_pt[0].y-10 < 1) face_boundary_pt[0].x = 11;
			cv::Rect rect_aligned_cropped_face_RGB = cv::Rect( face_boundary_pt[0].x-10, face_boundary_pt[0].y-10, (face_boundary_pt[1].x+10)-(face_boundary_pt[0].x+1-10), (face_boundary_pt[1].y+10)-(face_boundary_pt[0].y+1-10)); 
			//cv::rectangle(mat_aligned_face_RGB, rect_aligned_cropped_face_RGB, cv::Scalar(0, 255, 0), 1);

			printf("Confidence = %.3f\n", confidence);
			puts("------------------------------------------------------------------------");

			// crop aligned face area
			cv::Mat mat_aligned_cropped_face_RGB = mat_aligned_face_RGB(rect_aligned_cropped_face_RGB);
			std::string filepath_aligned_face = filepath + std::to_string((long long int)i) + ".aligned.png";
			std::string filepath_aligned_cropped_face = filepath + std::to_string((long long int)i) + ".aligned.cropped.png";
			std::string filepath_resized_aligned_cropped_face = filepath + std::to_string((long long int)i) + ".aligned.cropped.resized.png";
			// save resized aligned face area
			cv::Mat mat_resized_aligned_cropped_face_RGB( resized, resized, CV_8UC3, cv::Scalar(0,0,0));
			cv::resize( mat_aligned_cropped_face_RGB, mat_resized_aligned_cropped_face_RGB, mat_resized_aligned_cropped_face_RGB.size(), 0, 0, cv::INTER_LANCZOS4 );
			// save cropped aligned face area and wrapped image
			imwrite( filepath_aligned_face, mat_aligned_face_RGB);
			imwrite( filepath_aligned_cropped_face, mat_aligned_cropped_face_RGB);
			imwrite( filepath_resized_aligned_cropped_face, mat_resized_aligned_cropped_face_RGB);
		}
		// save output of ffd in original color image
		std::string filepath_original = filepath + ".original.png";
		cv::imwrite(filepath_original, image);
	}
	return 0;
}

