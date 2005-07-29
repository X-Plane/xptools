/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MovieTextureNode.cpp
*
******************************************************************/

#include "MovieTextureNode.h"

MovieTextureNode::MovieTextureNode() 
{
	setHeaderFlag(false);
	setType(movieTextureNodeString);

	///////////////////////////
	// Exposed Field 
	///////////////////////////

	// url field
	urlField = new MFString();
	addExposedField(urlFieldString, urlField);

	// loop exposed field
	loopField = new SFBool(false);
	addExposedField(loopFieldString, loopField);

	// startTime exposed field
	startTimeField = new SFTime(0.0f);
	addExposedField(startTimeFieldString, startTimeField);

	// stopTime exposed field
	stopTimeField = new SFTime(0.0f);
	addExposedField(stopTimeFieldString, stopTimeField);

	// speed exposed field
	speedField = new SFFloat(1.0f);
	addExposedField(speedTimeFieldString, speedField);

	///////////////////////////
	// EventOut
	///////////////////////////

	// isActive eventOut field
	isActiveField = new SFBool(false);
	addEventOut(isActiveFieldString, isActiveField);

	// time eventOut field
	durationChangedField = new SFTime(-1.0f);
	addEventOut(durationFieldString, durationChangedField);
}

MovieTextureNode::~MovieTextureNode() 
{
}

////////////////////////////////////////////////
// Url
////////////////////////////////////////////////

MFString *MovieTextureNode::getUrlField()
{
	if (isInstanceNode() == false)
		return urlField;
	return (MFString *)getExposedField(urlFieldString);
}

void MovieTextureNode::addUrl(char *value) 
{
	getUrlField()->addValue(value);
}

int MovieTextureNode::getNUrls() 
{
	return getUrlField()->getSize();
}

char *MovieTextureNode::getUrl(int index) 
{
	return getUrlField()->get1Value(index);
}

void MovieTextureNode::setUrl(int index, char *urlString) 
{
	getUrlField()->set1Value(index, urlString);
}

////////////////////////////////////////////////
//	Loop
////////////////////////////////////////////////

SFBool *MovieTextureNode::getLoopField()
{
	if (isInstanceNode() == false)
		return loopField;
	return (SFBool *)getExposedField(loopFieldString);
}
	
void MovieTextureNode::setLoop(bool value) 
{
	getLoopField()->setValue(value);
}

void MovieTextureNode::setLoop(int value) 
{
	setLoop(value ? true : false);
}

bool MovieTextureNode::getLoop() 
{
	return getLoopField()->getValue();
}

bool MovieTextureNode::isLoop() 
{
	return getLoop();
}

////////////////////////////////////////////////
//	Speed
////////////////////////////////////////////////

SFFloat *MovieTextureNode::getSpeedField()
{
	if (isInstanceNode() == false)
		return speedField;
	return (SFFloat *)getExposedField(speedTimeFieldString);
}
	
void MovieTextureNode::setSpeed(float value) 
{
	getSpeedField()->setValue(value);
}

float MovieTextureNode::getSpeed() 
{
	return getSpeedField()->getValue();
}

////////////////////////////////////////////////
//	Start time
////////////////////////////////////////////////

SFTime *MovieTextureNode::getStartTimeField()
{
	if (isInstanceNode() == false)
		return startTimeField;
	return (SFTime *)getExposedField(startTimeFieldString);
}
	
void MovieTextureNode::setStartTime(double value) 
{
	getStartTimeField()->setValue(value);
}

double MovieTextureNode::getStartTime() 
{
	return getStartTimeField()->getValue();
}

////////////////////////////////////////////////
//	Stop time
////////////////////////////////////////////////

SFTime *MovieTextureNode::getStopTimeField()
{
	if (isInstanceNode() == false)
		return stopTimeField;
	return (SFTime *)getExposedField(stopTimeFieldString);
}
	
void MovieTextureNode::setStopTime(double value) 
{
	getStopTimeField()->setValue(value);
}

double MovieTextureNode::getStopTime() 
{
	return getStopTimeField()->getValue();
}

////////////////////////////////////////////////
//	isActive
////////////////////////////////////////////////

SFBool *MovieTextureNode::getIsActiveField()
{
	if (isInstanceNode() == false)
		return isActiveField;
	return (SFBool *)getEventOut(isActiveFieldString);
}
	
void MovieTextureNode::setIsActive(bool value) 
{
	getIsActiveField()->setValue(value);
}

bool MovieTextureNode::getIsActive() 
{
	return getIsActiveField()->getValue();
}

bool MovieTextureNode::isActive() 
{
	return getIsActiveField()->getValue();
}

////////////////////////////////////////////////
//	duration_changed
////////////////////////////////////////////////

SFTime *MovieTextureNode::getDurationChangedField()
{
	if (isInstanceNode() == false)
		return durationChangedField;
	return (SFTime *)getEventOut(durationFieldString);
}
	
void MovieTextureNode::setDurationChanged(double value) 
{
	getDurationChangedField()->setValue(value);
}

double MovieTextureNode::getDurationChanged() 
{
	return getDurationChangedField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

MovieTextureNode *MovieTextureNode::next() 
{
	return (MovieTextureNode *)Node::next(getType());
}

MovieTextureNode *MovieTextureNode::nextTraversal() 
{
	return (MovieTextureNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool MovieTextureNode::isChildNodeType(Node *node)
{
	return false;
}

void MovieTextureNode::initialize() 
{
}

void MovieTextureNode::uninitialize() 
{
}

void MovieTextureNode::update() 
{
}

////////////////////////////////////////////////
//	infomation
////////////////////////////////////////////////

void MovieTextureNode::outputContext(ostream &printStream, char *indentString) 
{
	SFBool *loop = getLoopField();
	SFBool *repeatS = getRepeatSField();
	SFBool *repeatT = getRepeatTField();

	printStream << indentString << "\t" << "loop " << loop << endl;
	printStream << indentString << "\t" << "speed " << getSpeed() << endl;
	printStream << indentString << "\t" << "startTime " << getStartTime() << endl;
	printStream << indentString << "\t" << "stopTime " << getStopTime() << endl;
	printStream << indentString << "\t" << "repeatS " << repeatS << endl;
	printStream << indentString << "\t" << "repeatT " << repeatT << endl;

	if (0 < getNUrls()) {
		MFString *url = getUrlField();
		printStream << indentString << "\t" << "url [" << endl;
		url->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}
