/**
* Version 1.0 : 
*		feature:MCT, classifier: Ada-boost
*/

#ifndef __FACE_DETECTOR_H__
#define __FACE_DETECTOR_H__

#include "object.hpp"
#include "object_detector.hpp"

#include <stdint.h>

#include <string>
#include <sstream>
#include <vector>

#ifndef FD_VER_MAJOR
#define FD_VER_MAJOR    "1"
#endif
#ifndef FD_VER_MINOR
#define FD_VER_MINOR    "0"
#endif
#ifndef FD_VERSION
#define FD_VERSION      (FD_VER_MAJOR "." FD_VER_MINOR)
#endif


namespace Daum
{
	class CFaceDetector : public CObjectDetector
	{
		public:
			CFaceDetector()
			{
			}
			
			virtual ~CFaceDetector()
			{
			}

			std::string GetVersion(void)
			{
				return std::string(FD_VERSION);
			}

			bool Detect(unsigned char *image, uint32_t width, uint32_t height, std::vector<Daum::CObject> &objects);
	};
}

#endif

