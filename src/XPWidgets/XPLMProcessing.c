#include "XPLMProcessing.h"
#include "XPLMPrivate.h"
#include <vector>
#include <time.h>

struct	XPLMFlightLoopRecord {
	float				lastCallTime;
	int					lastCallNumber;
	float				interval;
	XPLMFlightLoop_f	callback;
	void *				refcon;
};

typedef	std::vector<XPLMFlightLoopRecord>	XPLMFlightLoopVector;
static	XPLMFlightLoopVector				gFlightLoops;
static	float								gLastCallTime = 0;
static	int									gLastCallNumber = 0;

float		XPLMGetElapsedTime(void)
{
	return (float) clock() / (float) CLOCKS_PER_SEC;
}

int			XPLMGetCycleNumber(void)
{
	return gLastCallNumber;
}

void			XPLMRegisterFlightLoopCallback(
					XPLMFlightLoop_f	inFlightLoop,
					float				inInterval,
					void *				inRefcon)
{
	XPLMFlightLoopRecord	loop;
	loop.lastCallTime = XPLMGetElapsedTime();
	loop.lastCallNumber = gLastCallNumber;
	loop.interval = inInterval;
	loop.callback = inFlightLoop;
	loop.refcon = inRefcon;
	gFlightLoops.push_back(loop);
}

void		XPLMUnregisterFlightLoopCallback(
					XPLMFlightLoop_f	inFlightLoop, void * inRefcon)
{
	for (XPLMFlightLoopVector::iterator iter = gFlightLoops.begin(); iter != gFlightLoops.end(); ++iter)
	{
		if ((iter->callback == inFlightLoop) && (iter->refcon == inRefcon))
		{
			gFlightLoops.erase(iter);
			return;
		}
	}
}

void		XPLMSetFlightLoopCallbackInterval(
					XPLMFlightLoop_f	inFlightLoop,
					float				inInterval,
					int					inRelativeToNow,
					void *				inRef)
{
	for (XPLMFlightLoopVector::iterator iter = gFlightLoops.begin(); iter != gFlightLoops.end(); ++iter)
	{
		if ((iter->callback == inFlightLoop) && (iter->refcon == inRef))
		{
			iter->interval = inInterval;
			if (inRelativeToNow)
				iter->lastCallTime = XPLMGetElapsedTime();

		}
	}
}

void XPLMDoFlightLoopProcessing(float inElapsedTime)
{
	++gLastCallNumber;
	for (XPLMFlightLoopVector::iterator iter = gFlightLoops.begin(); iter != gFlightLoops.end(); ++iter)
	{
		if (iter->interval > 0)
		{
			float targetTime = iter->lastCallTime + iter->interval;
			if (targetTime <= inElapsedTime)
			{
				{
					iter->interval = iter->callback(inElapsedTime - iter->lastCallTime,
													inElapsedTime - gLastCallTime,
													gLastCallNumber,
													iter->refcon);
					iter->lastCallTime = inElapsedTime;
					iter->lastCallNumber = gLastCallNumber;
				}
			}
		} else if (iter->interval < 0)
		{
			// This one wants a callback in a number of frames.
			long	targetNumber = iter->lastCallNumber - iter->interval;
			if (targetNumber <= gLastCallNumber)
			{
				// Run the flight loop
				{
					iter->interval = iter->callback(inElapsedTime - iter->lastCallTime,
													inElapsedTime - gLastCallTime,
													gLastCallNumber,
													iter->refcon);
					iter->lastCallTime = inElapsedTime;
					iter->lastCallNumber = gLastCallNumber;
				}
			}
		}
	}
}

void XPLMProcessingCleanupHook(void)
{
	gFlightLoops.clear();
}