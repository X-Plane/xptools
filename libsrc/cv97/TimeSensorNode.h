/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	TimeSensorNode.h
*
******************************************************************/

#ifndef _CV97_TIMESENSOR_H_
#define _CV97_TIMESENSOR_H_

#include <time.h>
#ifdef WIN32
//#include <sys/timeb.h>
#endif

#include "SensorNode.h"

class TimeSensorNode : public SensorNode {

	SFBool *loopField;
	SFTime *cycleIntervalField;
	SFTime *startTimeField;
	SFTime *stopTimeField;
	SFTime *cycleTimeField;
	SFTime *timeField;
	SFFloat *fractionChangedField;

public:

	TimeSensorNode();
	~TimeSensorNode();

	////////////////////////////////////////////////
	//	Loop
	////////////////////////////////////////////////
	
	SFBool *getLoopField();

	void setLoop(bool value);
	void setLoop(int value);
	bool getLoop();
	bool isLoop();

	////////////////////////////////////////////////
	//	Cyble Interval
	////////////////////////////////////////////////
	
	SFTime *getCycleIntervalField();

	void setCycleInterval(double value);
	double getCycleInterval();

	////////////////////////////////////////////////
	//	Start time
	////////////////////////////////////////////////
	
	SFTime *getStartTimeField();

	void setStartTime(double value);
	double getStartTime();

	////////////////////////////////////////////////
	//	Stop time
	////////////////////////////////////////////////
	
	SFTime *getStopTimeField();

	void setStopTime(double value);
	double getStopTime();

	////////////////////////////////////////////////
	//	fraction_changed
	////////////////////////////////////////////////
	
	SFFloat *getFractionChangedField();

	void setFractionChanged(float value);
	float getFractionChanged();

	////////////////////////////////////////////////
	//	Cycle time
	////////////////////////////////////////////////
	
	SFTime *getCycleTimeField();

	void setCycleTime(double value);
	double getCycleTime();

	////////////////////////////////////////////////
	//	Time
	////////////////////////////////////////////////
	
	SFTime *getTimeField();

	void setTime(double value);
	double getTime();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	TimeSensorNode *next();
	TimeSensorNode *nextTraversal();

	////////////////////////////////////////////////
	//	Virtual functions
	////////////////////////////////////////////////
	
	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();
	void outputContext(ostream &printStream, char *indentString);
};

#endif

