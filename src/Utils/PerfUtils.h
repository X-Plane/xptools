/*
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#ifndef PERFUTILS_H
#define PERFUTILS_H

#if __profile__
#include <Profiler.h>

struct	StProfile {
	unsigned char * fname_;
	StProfile(unsigned char * fname) : fname_(fname) { ProfilerSetStatus(1); };
	~StProfile() { ProfilerSetStatus(0); ProfilerDump(fname_); ProfilerClear(); };
};

struct StProfileInit {
	StProfileInit() { ProfilerInit(collectDetailed, bestTimeBase, 1000, 300); }
	~StProfileInit() { ProfilerTerm(); }
};

#endif


#if APL
#include <mach/mach_time.h>
#elif LIN
#include <time.h>
#elif IBM
	// XDEFs should have gotten windows.
#else
	#error platform not defined
#endif




inline unsigned long long query_hpc()
{
	#if APL
		return mach_absolute_time();
	#elif IBM
		LARGE_INTEGER cntr;
		QueryPerformanceCounter  (&cntr);
		return cntr.QuadPart;		
	#elif LIN
		struct timespec tp;
		int ret = clock_gettime(CLOCK_MONOTONIC, &tp);
		if(ret != 0) return 0;
		unsigned long long usecs = (unsigned long long)tp.tv_sec * 1000000ULL + (unsigned long long)tp.tv_nsec / 1000ULL;
		return usecs;
	#else
		#error not implemented
	#endif
}

inline double hpc_to_microseconds(unsigned long long in_hpc_delta)
{
	#if APL
		mach_timebase_info_data_t	 ifo;
		mach_timebase_info(&ifo);

		double delta = in_hpc_delta;
		double num = ifo.numer;
		double denom = ifo.denom;
		return delta * num / (denom * 1000.0);
	#elif IBM
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);	

		double delta = in_hpc_delta;
		double denom = freq.QuadPart;
		return (delta * 1000000.0 /denom );
	#elif LIN
		return in_hpc_delta;
	#else
		#error not implemented
	#endif
}








class	PerfTimer {

	unsigned long long	mStart;
	double				mTime;
	unsigned long		mCalls;
	const char *		mName;

public:

	PerfTimer(const char * inName) :
		mName(inName), mTime(0), mCalls(0)
	{
	}

	~PerfTimer()
	{
	}

	void	GetStats(double& totalSeconds, unsigned long& calls)
	{
		totalSeconds = mTime;
		calls = mCalls;
	}

	inline	void Start(void)
	{
		mStart = query_hpc();
	}

	inline	void Stop(void)
	{
		unsigned long long stopTime = query_hpc();
		unsigned long long delta = stopTime - mStart;
		mTime += hpc_to_microseconds(delta);
		++mCalls;
	}

};

class	StElapsedTime {
	unsigned long long 	mStartTime;
	const char *		mName;
public:
	StElapsedTime(const char * inName): mName(inName)
	{
		mStartTime = query_hpc();
	}
	~StElapsedTime()
	{
		unsigned long long stopTime = query_hpc();
		unsigned long long delta = stopTime - mStartTime;
		printf("%s - %lf seconds.\n", mName, hpc_to_microseconds(delta) / 1000000.0);
	}
};

#endif
