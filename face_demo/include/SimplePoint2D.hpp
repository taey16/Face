
#ifndef __DAUM_SIMPLE_POINT2D_HPP__
#define __DAUM_SIMPLE_POINT2D_HPP__

#include <vector>
#include <iostream>
#include <fstream>

#include <cv.h>
#include <opencv2/highgui/highgui.hpp>

#ifndef Point2f
#define 

namespace Daum
{
	class SimplePoint2D
	{
		public:
			std::string fileName;
			int class_id;
			int numberOfPoints;
			std::vector<_Point2D> data;
		public:
			SimplePoint2D()
			{
				fileName = "";
				numberOfPoints = 0;
				data.resize(numberOfPoints);
				this->class_id = -1;
			}

			SimplePoint2D(int numberOfPoints, int class_id = 0)
			{
				this->SetFeatureDimension(dimension);
				this->class_id = class_id;
			}

			/**
			 * @fn Constructor
			 * @brief
			 *  Set dimension size, allocate memory and assign vector components
			 * @param dimension, data array, class_id (default=0)
			 */
			SimpleGlobalFeature(int dimension, float** data, int class_id = 0)
			{
				this->SetFeatureDimension(dimension, *data);
				this->class_id = class_id;
			}

			/**
			 * @fn Copy-constructor
			 * @brief
			 * copy SimpleGlobalFeature instance
			 * @param dimension, data array, class_id (default=0)
			 */
			SimpleGlobalFeature(const SimpleGlobalFeature& oprd)
			{
				*this = oprd;
			}


			virtual ~SimpleGlobalFeature()
			{
			}

			/**
			 * @fn SetFeatureDimension
			 * @brief
			 *  Set dimension size and allocate memory ( intital value : 0)
			 * @param dimension
			 */
			int SetFeatureDimension(int dim)
			{
				this->dimension = dim;
				this->data.clear();
				this->data.resize(dim, 0.);
				return 0;
			}

			/**
			 * @fn SetFeatureDimension
			 * @brief
			 *  Set dimension size and copy raw feature vector
			 * @param dimension raw vector pointer
			 */
			int SetFeatureDimension(int dim, float* data)
			{
				this->SetFeatureDimension( dim );
				std::copy(data, data+dim, this->data.begin());
				return 0;
			}

			int Append( const SimpleGlobalFeature& oprd)
			{
				int returnValue = 0;
				if ( this->class_id != oprd.class_id ) {
					std::cerr<<"ERROR Daum::SimpleGlobalFeature::Append (original class_id and appended class_id is mismatched)" <<std::endl;
					returnValue = -1;
				}
				this->dimension += oprd.dimension;
				this->data.insert( this->data.end(), oprd.data.begin(), oprd.data.end() );
				return returnValue;
			}

			void operator=(const SimpleGlobalFeature& oprd)
			{
				this->SetFeatureDimension(oprd.dimension);
				this->class_id = oprd.class_id;
				std::copy(oprd.data.begin(), oprd.data.end(), this->data.begin());
			}

			friend std::ostream& operator<<(std::ostream& os, SimpleGlobalFeature& oprd)
			{
				os.write((const char*)&oprd.dimension, 		sizeof(oprd.dimension));
				os.write((const char*)&oprd.class_id,		sizeof(oprd.class_id));
				os.write((const char*)&oprd.data[0], 		sizeof(float)*oprd.dimension);
				return os;
			}

			friend std::istream& operator>>(std::istream& is, SimpleGlobalFeature& oprd)
			{
				is.read((char*)&oprd.dimension,	sizeof(oprd.dimension));
				is.read((char*)&oprd.class_id,		sizeof(oprd.class_id));
				oprd.SetFeatureDimension(oprd.dimension);
				is.read((char*)&oprd.data[0], 		sizeof(float)*oprd.dimension);
				return is;
			}
	};

	/** 
	 * @defgroup SimpleGlobalFeatureIO utility class for saving and loading features
	 * @addtogroup SimpleGlobalFeatureIO
	 * @{
	 */
	class SimpleGlobalFeatureIO
	{
		public:

			/**
			 * @fn DeblankString
			 * @brief
			 *  Deblank inputString( in-place) 
			 * @param inputString
			 */
			static int DeblankString( std::string& inputString )
			{
				inputString.erase(
								std::find_if(	inputString.rbegin(), inputString.rend(), 
												std::not1( std::ptr_fun<int, int>(std::isspace) ) ).base(), 
												inputString.end() );

				return 0;
			}

			/**
			 * @fn DeblankString
			 * @brief
			 *  Deblank a vector of inputStrings (in-place) 
			 * @param inputString
			 */
			static int DeblankString( std::vector<std::string>& inputString)
			{
				uint64_t numberOfStrings = inputString.size();
				for(uint64_t i=0; i<numberOfStrings; i++) {
					DeblankString( inputString[i] );
				}
				return 0;
			}

			/**
			 * @fn SaveSimpleGlobalFeatures
			 * @brief
			 *  Save a vector of SimpleGlobalFeatures into the disc
			 * @param filename, a vector of SimpleGlobalFeatures
			 */
			static int SaveSimpleGlobalFeatures(std::string& filename, std::vector<Daum::SimpleGlobalFeature>* featureList)
			{
				if(featureList == 0) {
					std::cout<< "SimpleGlobalFeatureIO::SaveSimpleGlobalFeatures (*featureList is null)" <<std::endl;
					return -1;
				}
				std::ofstream of; 
				of.open(filename.c_str(), std::ios_base::binary);
				if( of.good() )
					SimpleGlobalFeatureIO::Streaming(of, featureList);
				else {
					std::cout<< "SimpleGlobalFeatureIO::SaveSimpleGlobalFeatures (check filename : " <<filename << std::endl;
					return -1;
				}
				of.close();
				return 0;
			}

			/**
			 * @fn SaveSimpleGlobalFeatures
			 * @brief
			 *  Save a SimpleGlobalFeatures into the disc
			 * @param filename, a SimpleGlobalFeature
			 */
			static int SaveSimpleGlobalFeatures(std::string& filename, Daum::SimpleGlobalFeature* feature)
			{
				if(feature == 0) {
					std::cout<< "SimpleGlobalFeatureIO::SaveSimpleGlobalFeatures (*featureList is null)" <<std::endl;
					return -1;
				}
				std::ofstream of; 
				of.open(filename.c_str(), std::ios_base::binary);

				if( of.good() )
					SimpleGlobalFeatureIO::Streaming(of, feature);
				else {
					std::cout<< "SimpleGlobalFeatureIO::SaveSimpleGlobalFeatures (check filename : " <<filename << std::endl;
					return -1;
				}
				of.close();
				return 0;
			}

			/**
			 * @fn LoadSimpleGlobalFeatures
			 * @brief
			 *  Load a vector of SimpleGlobalFeatures from the disc
			 * @param filename, a vector of SimpleGlobalFeatures
			 */
			static int LoadSimpleGlobalFeatures( std::string& filename, std::vector<Daum::SimpleGlobalFeature>* featureList)
			{
				if(featureList == 0) {
					std::cout<< "SimpleGlobalFeatureIO::SaveSimpleGlobalFeatures (*featureList is null)" <<std::endl;
					return -1;
				}
				std::ifstream inf;
				inf.open(filename.c_str(), std::ios_base::binary);

				if( inf.good() )
					SimpleGlobalFeatureIO::Streaming(inf, featureList);
				else {
					std::cout<< "SimpleGlobalFeatureIO::LoadSimpleGlobalFeatures (check filename : " <<filename <<std::endl;
					return -1;
				}
				inf.close();
				return 0;
			}

			/**
			 * @fn LoadSimpleGlobalFeatures
			 * @brief
			 *  Load a SimpleGlobalFeatures from the disc
			 * @param filename, a SimpleGlobalFeature
			 */
			static int LoadSimpleGlobalFeatures(std::string& filename, Daum::SimpleGlobalFeature* feature)
			{
				if(feature == 0) {
					std::cout<< "SimpleGlobalFeatureIO::LoadSimpleGlobalFeatures (*featureList is null)" <<std::endl;
					return -1;
				}
				std::ifstream inf;
				inf.open(filename.c_str(), std::ios_base::binary);

				if( inf.good() )
					SimpleGlobalFeatureIO::Streaming(inf, feature);
				else {
					std::cout<< "SimpleGlobalFeatureIO::LoadSimpleGlobalFeatures (check filename : " <<filename<<std::endl;
					return -1;
				}
				inf.close();
				return 0;
			}

			static void Streaming(std::ostream& of, std::vector<Daum::SimpleGlobalFeature>* featureList)
			{
				if(featureList == 0)
					return;

				// feature info
				int count = (int)featureList->size();
				of.write((const char*)&count, sizeof(count));
				for(int i=0; i<count; i++) {   
					of << (*featureList)[i];
				}   
			}

			static void Streaming(std::ostream& of, Daum::SimpleGlobalFeature* feature)
			{
				if(feature == 0)
					return;
				of << (*feature);
			}


			static void Streaming(std::istream& inf, std::vector<Daum::SimpleGlobalFeature>* featureList)
			{
				if(featureList == 0)
					return;

				int count = 0;
				inf.read((char*)&count, sizeof(count));
				for(int i=0; i<count; i++) {
					Daum::SimpleGlobalFeature featureData;
					inf >> featureData;
					featureList->push_back(featureData);
				}
			}

			static void Streaming(std::istream& inf, Daum::SimpleGlobalFeature* feature)
			{
				if(feature == 0)
					return;
				inf >> (*feature);
			}

	};
	/** @} */ // addtogroup SimpleGlobalFeatureIO
	/** @} */ // addtogroup Structure
}

#endif
