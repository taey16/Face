/**
*/

#ifndef __OBJECT_HPP__
#define __OBJECT_HPP__

#include <string>
#include <sstream>

#ifndef OBJECT_VER_MAJOR
#define OBJECT_VER_MAJOR    "0"
#endif
#ifndef OBJECT_VER_MINOR
#define OBJECT_VER_MINOR    "0.1.1"
#endif
#ifndef OBJECT_VERSION
#define OBJECT_VERSION      (OBJECT_VER_MAJOR "." OBJECT_VER_MINOR)
#endif

namespace Daum
{
	class CObject
	{
	public:
		int	x, y;
		int	width;
		int	height;
		int	angle;

	public:
		CObject()
		{
			this->x 	= 0;
			this->y 	= 0;
			this->width 	= 0;
			this->height 	= 0;
			this->angle		= 0;
		}

		virtual ~CObject()
		{
		}

		std::string GetVersion(void)
		{
			return std::string(OBJECT_VERSION);
		}

		void operator=(const CObject& oprd)
		{
			this->x = oprd.x;
			this->y= oprd.y;
			this->width = oprd.width;
			this->height = oprd.height;
			this->angle = oprd.angle;
		}

		friend std::ostream& operator<<(std::ostream& os, Daum::CObject& oprd)
		{
				os << oprd.x << ", " << oprd.y << ", " << oprd.width << ", " << oprd.height << ", " << oprd.angle;

				return os;
		}
	};
}

#endif

