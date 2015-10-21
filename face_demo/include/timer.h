
/*****************************************************************************
Windows와 Linux 환경에서 동작하는 실행시간 측정 라이브러리

Author: MinWoo Byeon (mwbyun@gmail.com)
Reference: http://www.ibm.com/developerworks/kr/library/l-rt1/
*****************************************************************************/

#ifdef _WIN32
#include <Windows.h>

static LARGE_INTEGER _tstart, _tend;
static LARGE_INTEGER freq;

inline void start_timer()
{
    static int first = 1;

    if(first) {
        QueryPerformanceFrequency(&freq);
        first = 0;
    }
    QueryPerformanceCounter(&_tstart);
}

inline double get_time()
{
    QueryPerformanceCounter(&_tend);
    return ((double)_tend.QuadPart - (double)_tstart.QuadPart)/((double)freq.QuadPart);
}
#else
#include <sys/time.h>
static struct timeval _tstart, _tend;
static struct timezone tz;

inline void start_timer()
{
    gettimeofday(&_tstart, &tz);
}

inline double get_time()
{
    double t1, t2;
    gettimeofday(&_tend,&tz);
    t1 =  (double)_tstart.tv_sec + (double)_tstart.tv_usec/(1000*1000);
    t2 =  (double)_tend.tv_sec + (double)_tend.tv_usec/(1000*1000);
    return t2-t1;
}
#endif

