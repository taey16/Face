/*
 * feature.h
 *
 *  Created on: Nov 11, 2013
 *      Author: hyunchul
 */

#ifndef FEATURE_H_
#define FEATURE_H_

#include <vector>
using namespace std;

#include "typedef.h"
#include "common_feature.h"

namespace daum {

class Feature {
public:
	Feature();
	virtual ~Feature();

	void set_configuration(int cell_size, int block_num, int bin_num);
	void calc_HOG_descriptor(Image& image);
	void calc_HOG_descriptor_new(Image& image);
	void calc_HOG_descriptor_for_motion(Image& image);
	vector<float> feature;

private:
	struct FEATURE_CONFIGURE {
		int cell_size;
		int block_num;
		int bin_num;
	} feature_configure;
};

} /* namespace daum */

#endif /* FEATURE_H_ */
