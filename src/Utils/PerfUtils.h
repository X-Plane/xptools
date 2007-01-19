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
	#if __MACH__
//		#include <Carbon/Carbon.h>
	#if !DEV 
	fixt his
	#endif
	#else
		#include <Timer.h>
	#endif
	#include <time.h>
#elif IBM
	#include <time.h>
#elif LIN
	#error Linux not yet implemented.
#else
	#error PLATFORM NOT DEFINED
#endif

class	PerfTimer {

#if APL && 0
	unsigned long long	mStart;
#else
	clock_t				mStart;
#endif	
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
#if APL && 0
		::Microseconds((UnsignedWide *) &mStart);
#else	
		mStart = clock();
#endif		
	}
	
	inline	void Stop(void)
	{
#if APL	&& 0
		unsigned long long stopTime;
		::Microseconds((UnsignedWide *) &stopTime);
		unsigned long long delta = stopTime - mStart;
		mTime += (double) delta / 1000000.0;
#else		
		clock_t stopTime = clock();
		clock_t delta = stopTime - mStart;
		mTime += ((double) delta) / ((double) CLOCKS_PER_SEC);
#endif		
		++mCalls;
	}

};

class	StElapsedTime {
#if APL && 0
	unsigned long long 	mStartTime;
#else
	clock_t				mStartTime;
#endif	
	const char *		mName;
public:
	StElapsedTime(const char * inName): mName(inName)
	{
#if APL	 && 0
		::Microseconds((UnsignedWide *) &mStartTime);
#else
		mStartTime = clock();
#endif				
	}
	~StElapsedTime()
	{
#if APL	&& 0
		unsigned long long stopTime;
		::Microseconds((UnsignedWide *) &stopTime);		
		unsigned long long delta = stopTime - mStartTime;
		printf("%s - %lf seconds.\n", mName, (double) delta / 1000000.0);
#else
		clock_t stopTime = clock();
		printf("%s - %lf seconds.\n", mName, (double) (stopTime - mStartTime) / (double) CLOCKS_PER_SEC);
		
#endif		
	}
};

#endif