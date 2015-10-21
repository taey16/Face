/**
*/

#ifndef __OBJECT_DETECTOR_HPP__
#define __OBJECT_DETECTOR_HPP__

#include "ConfigFile.h"
#include "object.hpp"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <sstream>
#include <vector>

#ifndef OD_VER_MAJOR
#define OD_VER_MAJOR    "0"
#endif
#ifndef OD_VER_MINOR
#define OD_VER_MINOR    "0.1.1"
#endif
#ifndef OD_VERSION
#define OD_VERSION      (OD_VER_MAJOR "." OD_VER_MINOR)
#endif


namespace Daum
{
	class CObjectDetector 
	{
		protected:
			bool		apply_face_rip;
			bool 		apply_face_rop;

			int 		face_optimize_opt;

			std::string	detector_type;
			std::string	configFilename;

			ConfigFile	*config;

		public:
			CObjectDetector()
			{
				this->detector_type = std::string("");
				this->config = NULL;

				this->apply_face_rip = true;
				this->apply_face_rop = true;

				this->face_optimize_opt = 0;
			}

			virtual ~CObjectDetector()
			{
				if(this->config) delete this->config;
				this->config = NULL;
			}

			std::string GetVersion(void)
			{
				return std::string(OD_VERSION);
			}

			void SetConfigureFile(const std::string &configFilename)
			{
				this->configFilename = configFilename;
				if(this->config) delete this->config;
				this->config = NULL;
				this->config = new ConfigFile(this->configFilename.c_str());

				return;
			}

			void SetConfigureString(const std::string &configString)
			{
				this->configFilename = "string";

				if(this->config) delete this->config;
				this->config = NULL;
				this->config = new ConfigFile();

				std::stringstream iostring;
				iostring << configString;
				iostring >> (*this->config);

				return;
			}

			std::string GetType(void)
			{
				return detector_type;
			}

			bool SetDetector(void)
			{
				if(this->config == NULL){
					fprintf(stderr, "Don't set the configure file\n");
					return false;
				}

				this->config->readInto(this->detector_type, "detector", std::string(""));

				this->config->readInto(this->apply_face_rip, "detector.face.apply_rip", false);
				this->config->readInto(this->apply_face_rop, "detector.face.apply_rop", false);
				this->config->readInto(this->face_optimize_opt, "detector.face.optimize_opt", 0);

				return true;
			}

			/** 
			Image Channel
			Face: Gray Image
			Person: RGB Image
			Car: RGB Image
			*/
			virtual bool Detect(unsigned char *image, uint32_t width, uint32_t height, std::vector<Daum::CObject> &objects)=0;

	};
}

#endif

