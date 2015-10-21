/*
 * stop_watch.cpp
 *
 *  Created on: Dec 10, 2013
 *      Author: minu
 */

#include "stop_watch.h"

namespace daum {

StopWatch::StopWatch() {
	reset();
}

StopWatch::~StopWatch() {

}

void StopWatch::start() {
	gettimeofday(&_tstart, &_tz);
	is_running_ = true;
}

void StopWatch::stop() {
	gettimeofday(&_tend, &_tz);
	is_running_ = false;
	accumulated_msecs_ += elapsed_msecs();
}

void StopWatch::restart() {
	reset();
	start();
}

void StopWatch::reset() {
	gettimeofday(&_tstart, &_tz);
	gettimeofday(&_tend, &_tz);
	accumulated_msecs_ = 0.0;
	is_running_ = false;
}

double StopWatch::accumulated_secs() {
	return accumulated_msecs_ * 1000.0;
}

double StopWatch::accumulated_msecs() {
	return accumulated_msecs_;
}

double StopWatch::elapsed_secs() {
	return elapsed_msecs() * 1000.0;
}

double StopWatch::elapsed_msecs() {
	if (is_running_) {
		gettimeofday(&_tend, &_tz);
	}

	double t1 = (double) _tstart.tv_sec * 1000 + (double) _tstart.tv_usec / 1000;
	double t2 = (double) _tend.tv_sec * 1000 + (double) _tend.tv_usec / 1000;
	return t2 - t1;
}

}
