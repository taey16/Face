
// http://en.wikipedia.org/wiki/Methods_of_computing_square_roots#Reciprocal_of_the_square_root
inline float inv_sqrt(float x) {
	float xhalf = 0.5f * x;
	union {
		float x;
		int i;
	} u;
	u.x = x;
	u.i = 0x5f3759df - (u.i >> 1);
    /* The next line can be repeated any number of times to increase accuracy */
    u.x = u.x * (1.5f - xhalf * u.x * u.x); // error: min = 0.00000000, max = 0.05423927, avg = 0.02040900
    u.x = u.x * (1.5f - xhalf * u.x * u.x); // error: min = 0.00000000, max = 0.00014496, avg = 0.00004089
    u.x = u.x * (1.5f - xhalf * u.x * u.x); // error: min = 0.00000000, max = 0.00000381, avg = 0.00000081
	return u.x;
}

inline float fast_sqrt(float x) {
	return 1.0f / inv_sqrt(x);
}

