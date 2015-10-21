/**
 * common util file
 * Author: Taewan Ethan Kim taey1600@gmail.com
 */

#ifndef __DAUM_COMMON_UTIL__
#define __DAUM_COMMON_UTIL__

#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

namespace Daum 
{
	class Utils
	{
		public:
			Utils() { }
			~Utils() { }
			template<class T> 
			struct index_cmp 
			{
				index_cmp(const T arr) : arr(arr) {}
				bool operator()(const size_t a, const size_t b) const { return arr[a] < arr[b]; }
				const T arr;
			};

			template<class T> 
			struct index_cmp_similarity
			{
				index_cmp_similarity(const T arr) : arr(arr) {}
				bool operator()(const size_t a, const size_t b) const { return arr[a] > arr[b]; }
				const T arr;
			};

			static int FindIdx(std::vector<int>& list, int item)
			{
				int index = -1;
				std::vector<int>::iterator idx = std::find(list.begin(), list.end(), item);
				if (idx != list.end()) {
					index = idx - list.begin();
				}
				return index;
			};

			static unsigned long getTickCount()
			{
				struct timeval gettick;
				unsigned long tick;

				gettimeofday(&gettick, NULL);
				tick = gettick.tv_sec*1000 + gettick.tv_usec/1000;
				return tick; //return (millisecond)
			}

			static int SplitString(std::string original, std::string token, std::vector<std::string>* results)
			{
				int cutAt;
				std::string localOriginal = original;

				// loop for finding a token
				while( (cutAt = (int)localOriginal.find_first_of(token)) != (int)localOriginal.npos) {
					if( cutAt > 0 ) // found
					{
						results->push_back(localOriginal.substr(0, cutAt));
					}
					localOriginal = localOriginal.substr(cutAt+1); // 
				}
				if(localOriginal.length() > 0) {
					results->push_back(localOriginal.substr(0, cutAt));
				}
				return results->size();
			}

			static int loadString(std::string filename, std::vector<std::string>* results)
			{
				std::ifstream infile(filename.c_str(), std::ios::in);

				if (infile.is_open() ) {
					while(infile.good()) {
						std::string line;
						std::getline(infile, line);
						if(line.size() > 0) {
							results->push_back(line);
						}
					}
				} else {
					std::cerr<<"ERROR Daum::Utils::loadString (check filename)"<<std::endl;
					return -1;
				}
				return 0;
			}

			static int loadStringAndUrl(std::string filename,std::vector<std::string>* docid,std::vector<std::string>* url)
			{
				std::ifstream infile(filename.c_str(),std::ios::in);
				while(infile.good()) {
					std::string line;
					std::getline(infile,line);
					if(line.size() > 1) {
						std::vector<std::string> item;
						SplitString(line,std::string(" "),&item);
						if(item.size() == 2) {
							docid->push_back(item[0]);
							url->push_back(item[1]);
						} else {
							docid->push_back(item[0]);
						}
					}
				}
				return 0;
			}

			static int loadGT(std::string gtFilename,std::vector<std::string>& gtFilenameList,std::vector<std::vector<std::string> >& gt_ap, int db_id)
			{
				// db_id : uk : 1, oxford, holidays = 0
				std::ifstream infile(gtFilename.c_str(), std::ios::in);

		        while(infile.good()) {
	                std::string line;
	                std::getline(infile, line);
	                if(line.size() > 1) {
                        std::vector<std::string>gt;
                        std::vector<std::string> item;
                        SplitString(line,std::string(" "),&item);
						gtFilenameList.push_back(item[0]);

						if( db_id == 0 ) // for oxford, holidays
						{
							for(int ii=1;ii<(int)item.size();ii++) {
                               	gt.push_back(item[ii]);
                        	}
						} else if(db_id == 1)  // for uk
						{
							for(int ii=0;ii<(int)item.size();ii++) {
                               	gt.push_back(item[ii]);
                        	}
						} else {
							std::cout<<"ERROR check db_id( uk : 1, oxford,holdiays : 0)" <<std::endl;
							return -1;
						}
                        gt_ap.push_back(gt);
	                }
		        }
				return 0;
			}

			static float computeRecallK(int queryId, int queryLabel, std::vector<int>& labelList, float nres, int K)
			{

				if ( (int)labelList.size() < K) {
					std::cout<< "comoputePrecisionK (retrieved item (size of labelList) is smaller than K" << std::endl;
				}

				int hit_count = 0;
				for(int i=0; i<K; i++) {
					if( queryLabel == labelList[i] )
						hit_count++;
				}
				return ((float)hit_count / nres);

			}

			static float computePrecisionK(int queryId, int queryLabel, std::vector<int>& labelList, int K)
			{
				if ( (int)labelList.size() < K) {
					std::cout<< "comoputePrecisionK (retrieved item (size of labelList) is smaller than K" << std::endl;
				}

				int hit_count = 0;
				for(int i=0; i<K; i++) {
					if( queryLabel == labelList[i] )
						hit_count++;
				}

				return ((float)hit_count / K);

			}

			static float computeAP(int queryId, int queryClassId, std::vector<int>& bit_ranking, std::vector<int>& ranks, float nres, int K)
			{

				if( (int)bit_ranking.size() < K) {
					std::cout<< "comoputeAP (retrieved item (size of bit_ranking) is smaller than K" << std::endl;
				}

		        for(int ii=0; ii<K; ii++) {
	                if(bit_ranking[ii] == queryClassId) {
                        ranks.push_back(ii);
	                }
		        }

				std::sort(ranks.begin(), ranks.end());
				float ap_value = ap(ranks, nres);
				return ap_value;
			}

			static float computeAP(int queryId, std::vector<std::string>& rankedlist, std::vector<std::vector<std::string > >& gt_ap, std::vector<int>& ranks)
			{

		        for(int ii=0; ii<(int)gt_ap[queryId].size(); ii++) {
	                std::vector<std::string>::iterator ind;
	                ind = std::find(rankedlist.begin(),rankedlist.end(),gt_ap[queryId][ii]);
	                if ( ind != rankedlist.end() ) {
                        int rankOrder = ind - rankedlist.begin();
                        //bit_ranking[order] = 1;
						ranks.push_back(rankOrder);
	                }
		        }
				std::sort(ranks.begin(),ranks.end());
		        ///std::vector<int> ranks;
		        //for(int ii=0;ii<(int)bit_ranking.size();ii++) {
		        //        if(bit_ranking[ii] == 1) {
		        //                ranks.push_back(ii);
		        //        }
		        //}
				int nres = (int)gt_ap[queryId].size();
				float ap_value = ap(ranks, nres);
				return ap_value;
			}

			static float ap(std::vector<int>& ranks, float nres)
			{
		        float ap = 0.;
		        double precision_0 = 0.;
		        double precision_1 = 0.;
		        //int nres = (int)gt_ap[queryId].size();
				double recall_step = 1.0/(double)nres;
		        for(int ntp=0;ntp<(int)ranks.size();ntp++) {
	                if (ranks[ntp] == 0) {
						precision_0 = 1.0;
					} else {
						precision_0 = double(ntp)/ranks[ntp];
					}
	                precision_1 = double(ntp+1)/(ranks[ntp]+1);
	                ap += (precision_1+precision_0)*recall_step/2.0;
		        }
				return ap;
			}

			static int computeKS(int queryId,std::vector<std::string>& rankedlist,std::vector<std::vector<std::string> > gt_ap)
			{
		        int hit_count = 0;
		        for(int ii=0;ii<(int)gt_ap[queryId].size();ii++) {
	                std::vector<std::string>::iterator ind;
	                ind = std::find(rankedlist.begin(),rankedlist.end(),gt_ap[queryId][ii]);
	                if (ind != rankedlist.end() ) {
                        int order = ind - rankedlist.begin();
						if (order < 4)
                        	hit_count++;
	                }
		        }
				return hit_count;
			}

			static int printConfigurationFile(std::string& filename)
			{
				std::ifstream is;
				is.open(filename.c_str());
				std::string line;
				if( is.good() ) {
					while( !is.eof() ) {
						is >> line;
						std::cout << line << std::endl;
					}
				} else {
					std::cout << "ERROR printConfigurationFile (check input conf. file path)" << std::endl;
					return -1;
				}
				is.close();
				return 0;
			}

			static int ivecs_read(std::string& filename, std::vector<std::vector<int> >& v)
			{
				std::ifstream is(filename.c_str(), std::ios::in | std::ios::binary );
				if( is.is_open() ) {
					int d = -1;
					int vecsizeof = -1;

					is.read((char*)&d, sizeof(int));
					vecsizeof = 1 * 4 + d * 4;

					is.seekg(0, std::ios::end);
					int a = 1;
					uint64_t  size = (uint64_t)is.tellg();
					uint64_t bmax = size / vecsizeof;
					uint64_t b = bmax;

					uint64_t n = b - a + 1;

					v.resize(n);
					is.seekg(0, std::ios::beg);
					for(uint64_t nn=0; nn<n; nn++) {
						v[nn].resize(d);
						int* vec = new int[d+1];
						is.read((char*)vec, sizeof(int) * (d+1) );
						std::copy(vec+1, vec+d+1, v[nn].begin());
						delete [] vec;
					}
				}	
				return 0;
			}

			static int fvecs_read(std::string& filename, std::vector<std::vector<float> >& v)
			{
				std::ifstream is(filename.c_str(), std::ios::in | std::ios::binary );
				if( is.is_open() ) {
					int d = -1;
					int vecsizeof = -1;

					is.read((char*)&d, sizeof(int));
					vecsizeof = 1 * 4 + d * 4;

					is.seekg(0, std::ios::end);
					int a = 1;
					uint64_t size = (uint64_t)is.tellg();
					uint64_t bmax = size / vecsizeof;
					uint64_t b = bmax;

					uint64_t n = b - a + 1;

					v.resize(n);

					is.seekg(0, std::ios::beg);

					for(uint64_t nn=0; nn<n; nn++) {
						v[nn].resize(d);

						float* vec = new float[d+1];
						is.read((char*)vec, sizeof(float) * (d+1) );

						std::copy(vec+1, vec+d+1, v[nn].begin());

						delete [] vec;
					}
				}
				is.close();
				return 0;
			}

			/*
			static int loadCIFAR_10(std::vector<int>& labelList, std::vector<Daum::ImageFormat>& dataList, std::string& filename)
			{
				int num_images_per_batch = 10000;	
				int width,height;
				width = height = 32;
				int nChannels = 3;
				//int imageSize = width*height*nChannels;
				int imageSize_c = width*height;
				//int dataSize_per_batch = num_images_per_batch*1+num_images_per_batch*imageSize;

				std::ifstream file(filename.c_str(), std::ios::in|std::ios::binary);

				if( file.is_open() ) 
				{
					labelList.resize(num_images_per_batch);
					dataList.resize(num_images_per_batch);

					for(int n=0; n<num_images_per_batch; n++) 
					{
						char label;
						file.read(&label, sizeof(char));

						if(label > 9)
							std::cout << "ERROR " << n << "th label is " << label << std::endl;
						labelList[n] = (int)label;

						Daum::ImageFormat image(width,height,nChannels);

						unsigned char* raw_1 = new unsigned char[imageSize_c];
						unsigned char* raw_2 = new unsigned char[imageSize_c];
						unsigned char* raw_3 = new unsigned char[imageSize_c];

						file.read((char*)raw_1, imageSize_c*sizeof(char));
						file.read((char*)raw_2, imageSize_c*sizeof(char));
						file.read((char*)raw_3, imageSize_c*sizeof(char));

						memcpy(&(image.r[0]), raw_1, imageSize_c*sizeof(char));
						memcpy(&(image.g[0]), raw_2, imageSize_c*sizeof(char));
						memcpy(&(image.b[0]), raw_3, imageSize_c*sizeof(char));

						if(raw_1) { delete [] raw_1; raw_1=NULL; }
						if(raw_2) { delete [] raw_2; raw_2=NULL; }
						if(raw_3) { delete [] raw_3; raw_3=NULL; }

						dataList[n] = image;
					}
				} else {
					std::cout << "ERROR load_CIFAR_10" << std::endl;
				}
				file.close();
				return 0;
			}
			*/
	}; // end of namespace
}

#endif 
