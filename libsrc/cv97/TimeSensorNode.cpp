/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	AudioClip.cpp
*
******************************************************************/

#include "TimeSensorNode.h"

TimeSensorNode::TimeSensorNode()
{
	setHeaderFlag(false);
	setType(timeSensorNodeString);

	// loop exposed field
	loopField = new SFBool(false);
	addExposedField(loopFieldString, loopField);

	// cybleInterval exposed field
	cycleIntervalField = new SFTime(1.0);
	addExposedField(cycleIntervalFieldString, cycleIntervalField);

	// startTime exposed field
	startTimeField = new SFTime(0.0f);
	addExposedField(startTimeFieldString, startTimeField);

	// stopTime exposed field
	stopTimeField = new SFTime(0.0f);
	addExposedField(stopTimeFieldString, stopTimeField);

	// cycleTime eventOut field
	cycleTimeField = new SFTime(-1.0f);
	addEventOut(cycleTimeFieldString, cycleTimeField);

	// time eventOut field
	timeField = new SFTime(-1.0f);
	addEventOut(timeFieldString, timeField);

	// fraction_changed eventOut field
	fractionChangedField = new SFFloat(0.0f);
	addEventOut(fractionFieldString, fractionChangedField);
}

TimeSensorNode::~TimeSensorNode()
{
}

////////////////////////////////////////////////
//	Loop
////////////////////////////////////////////////

SFBool *TimeSensorNode::getLoopField()
{
	if (isInstanceNode() == false)
		return loopField;
	return (SFBool *)getExposedField(loopFieldString);
}

void TimeSensorNode::setLoop(bool value)
{
	getLoopField()->setValue(value);
}

void TimeSensorNode::setLoop(int value)
{
	setLoop(value ? true : false);
}

bool TimeSensorNode::getLoop()
{
	return getLoopField()->getValue();
}

bool TimeSensorNode::isLoop()
{
	return getLoop();
}


////////////////////////////////////////////////
//	Cyble Interval
////////////////////////////////////////////////

SFTime *TimeSensorNode::getCycleIntervalField()
{
	if (isInstanceNode() == false)
		return cycleIntervalField;
	return (SFTime *)getExposedField(cycleIntervalFieldString);
}

void TimeSensorNode::setCycleInterval(double value)
{
	getCycleIntervalField()->setValue(value);
}

double TimeSensorNode::getCycleInterval()
{
	return getCycleIntervalField()->getValue();
}

////////////////////////////////////////////////
//	Start time
////////////////////////////////////////////////

SFTime *TimeSensorNode::getStartTimeField()
{
	if (isInstanceNode() == false)
		return startTimeField;
	return (SFTime *)getExposedField(startTimeFieldString);
}

void TimeSensorNode::setStartTime(double value)
{
	getStartTimeField()->setValue(value);
}

double TimeSensorNode::getStartTime()
{
	return getStartTimeField()->getValue();
}

////////////////////////////////////////////////
//	Stop time
////////////////////////////////////////////////

SFTime *TimeSensorNode::getStopTimeField()
{
	if (isInstanceNode() == false)
		return stopTimeField;
	return (SFTime *)getExposedField(stopTimeFieldString);
}

void TimeSensorNode::setStopTime(double value)
{
	getStopTimeField()->setValue(value);
}

double TimeSensorNode::getStopTime()
{
	return getStopTimeField()->getValue();
}

////////////////////////////////////////////////
//	fraction_changed
////////////////////////////////////////////////

SFFloat *TimeSensorNode::getFractionChangedField()
{
	if (isInstanceNode() == false)
		return fractionChangedField;
	return (SFFloat *)getEventOut(fractionFieldString);
}

void TimeSensorNode::setFractionChanged(float value)
{
	getFractionChangedField()->setValue(value);
}

float TimeSensorNode::getFractionChanged()
{
	return getFractionChangedField()->getValue();
}

////////////////////////////////////////////////
//	Cycle time
////////////////////////////////////////////////

SFTime *TimeSensorNode::getCycleTimeField()
{
	if (isInstanceNode() == false)
		return cycleTimeField;
	return (SFTime *)getEventOut(cycleTimeFieldString);
}

void TimeSensorNode::setCycleTime(double value)
{
	getCycleTimeField()->setValue(value);
}

double TimeSensorNode::getCycleTime()
{
	return getCycleTimeField()->getValue();
}

////////////////////////////////////////////////
//	Time
////////////////////////////////////////////////

SFTime *TimeSensorNode::getTimeField()
{
	if (isInstanceNode() == false)
		return timeField;
	return (SFTime *)getEventOut(timeFieldString);
}

void TimeSensorNode::setTime(double value)
{
	getTimeField()->setValue(value);
}

double TimeSensorNode::getTime()
{
	return getTimeField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

TimeSensorNode *TimeSensorNode::next()
{
	return (TimeSensorNode *)Node::next(getType());
}

TimeSensorNode *TimeSensorNode::nextTraversal()
{
	return (TimeSensorNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	Virtual functions
////////////////////////////////////////////////

bool TimeSensorNode::isChildNodeType(Node *node)
{
	return false;
}

void TimeSensorNode::initialize()
{
	setCycleTime(-1.0f);
	setIsActive(false);
}

void TimeSensorNode::uninitialize()
{
}

void TimeSensorNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *bEnabled = getEnabledField();
	SFBool *loop = getLoopField();

	printStream << indentString << "\t" << "cycleInterval " << getCycleInterval() << endl;
	printStream << indentString << "\t" << "enabled " << bEnabled << endl;
	printStream << indentString << "\t" << "loop " << loop << endl;
	printStream << indentString << "\t" << "startTime " << getStartTime() << endl;
	printStream << indentString << "\t" << "stopTime " << getStopTime() << endl;
}

//////////////////////////////////////////////
//	AudioClip::update
////////////////////////////////////////////////

void TimeSensorNode::update()
{
	static double currentTime = 0;

	double startTime = getStartTime();
	double stopTime = getStopTime();
	double cycleInterval = getCycleInterval();

	bool bActive	= isActive();
	bool bEnable	= isEnabled();
	bool bLoop		= isLoop();

	if (currentTime == 0)
		currentTime = GetCurrentSystemTime();

	// isActive
	if (bEnable == false && bActive == true) {
		setIsActive(false);
		sendEvent(getIsActiveField());
		return;
	}

	if (bActive == false && bEnable == true) {
		if (startTime <= currentTime) {
			if (bLoop == true && stopTime <= startTime)
				bActive = true;
			else if (bLoop == false && stopTime <= startTime)
				bActive = true;
			else if (currentTime <= stopTime) {
				if (bLoop == true && startTime < stopTime)
					bActive = true;
				else if	(bLoop == false && startTime < (startTime + cycleInterval) && (startTime + cycleInterval) <= stopTime)
					bActive = true;
				else if (bLoop == false && startTime < stopTime && stopTime < (startTime + cycleInterval))
					bActive = true;
			}
		}
		if (bActive) {
			setIsActive(true);
			sendEvent(getIsActiveField());
			setCycleTime(currentTime);
			sendEvent(getCycleTimeField());
		}
	}

	currentTime = GetCurrentSystemTime();

	if (bActive == true && bEnable == true) {
		if (bLoop == true && startTime < stopTime) {
			if (stopTime < currentTime)
				bActive = false;
		}
		else if (bLoop == false && stopTime <= startTime) {
			if (startTime + cycleInterval < currentTime)
				bActive = false;
		}
		else if (bLoop == false && startTime < (startTime + cycleInterval) && (startTime + cycleInterval) <= stopTime) {
			if (startTime + cycleInterval < currentTime)
				bActive = false;
		}
		else if (bLoop == false && startTime < stopTime && stopTime < (startTime + cycleInterval)) {
			if (stopTime < currentTime)
				bActive = false;
		}

		if (bActive == false) {
			setIsActive(false);
			sendEvent(getIsActiveField());
		}
	}

	if (bEnable == false || isActive() == false)
		return;

	// fraction_changed
	double	fraction = fmod(currentTime - startTime, cycleInterval);
	if (fraction == 0.0 && startTime < currentTime)
		fraction = 1.0;
	else
		fraction /= cycleInterval;
	setFractionChanged((float)fraction);
	sendEvent(getFractionChangedField());

	// cycleTime
	double	cycleTime		= getCycleTime();
	double	cycleEndTime	= cycleTime + cycleInterval;
	while (cycleEndTime < currentTime) {
		setCycleTime(cycleEndTime);
		cycleEndTime += cycleInterval;
		setCycleTime(currentTime);
		sendEvent(getCycleTimeField());
	}

	// time
	setTime(currentTime);
	sendEvent(getTimeField());
}

