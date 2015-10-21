/*
 * stop_watch.h
 *
 *  Created on: Dec 10, 2013
 *      Author: minu
 */

#ifndef STOP_WATCH_H_
#define STOP_WATCH_H_

#include <sys/time.h>

namespace daum {

class StopWatch {
public:
	StopWatch();
	~StopWatch();

	void start();
	void stop();
	void restart();
	void reset();
	double accumulated_secs();
	double accumulated_msecs();
	double elapsed_secs();
	double elapsed_msecs();

private:
	struct timeval _tstart, _tend;
	struct timezone _tz;
	double accumulated_msecs_;
	bool is_running_;
};

}

#endif /* STOP_WATCH_HPP_ */
