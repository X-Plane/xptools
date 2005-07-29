/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	AudioClipNode.cpp
*
******************************************************************/

#ifdef WIN32
#include <windows.h>
#endif

#include "AudioClipNode.h"

AudioClipNode::AudioClipNode() 
{
	setHeaderFlag(false);
	setType(audioClipNodeString);

	// description exposed field
	descriptionField = new SFString("");
	addExposedField(descriptionFieldString, descriptionField);

	// loop exposed field
	loopField = new SFBool(false);
	addExposedField(loopFieldString, loopField);

	// startTime exposed field
	startTimeField = new SFTime(0.0f);
	addExposedField(startTimeFieldString, startTimeField);

	// stopTime exposed field
	stopTimeField = new SFTime(0.0f);
	addExposedField(stopTimeFieldString, stopTimeField);

	// pitch exposed field
	pitchField = new SFFloat(1.0f);
	addExposedField(pitchFieldString, pitchField);

	// url exposed field
	urlField = new MFString();
	addExposedField(urlFieldString, urlField);

	// isActive eventOut field
	isActiveField = new SFBool(false);
	addEventOut(isActiveFieldString, isActiveField);

	// time eventOut field
	durationChangedField = new SFTime(-1.0f);
	addEventOut(durationFieldString, durationChangedField);
}

AudioClipNode::~AudioClipNode() 
{
}

////////////////////////////////////////////////
//	Description
////////////////////////////////////////////////

SFString *AudioClipNode::getDescriptionField()
{
	if (isInstanceNode() == false)
		return descriptionField;
	return (SFString *)getExposedField(descriptionFieldString);
}

void AudioClipNode::setDescription(char * value) 
{
	getDescriptionField()->setValue(value);
}

char *AudioClipNode::getDescription() 
{
	return getDescriptionField()->getValue();
}

////////////////////////////////////////////////
//	Loop
////////////////////////////////////////////////

SFBool *AudioClipNode::getLoopField()
{
	if (isInstanceNode() == false)
		return loopField;
	return (SFBool *)getExposedField(loopFieldString);
}
	
void AudioClipNode::setLoop(bool value) 
{
	getLoopField()->setValue(value);
}

void AudioClipNode::setLoop(int value) 
{
	setLoop(value ? true : false);
}

bool AudioClipNode::getLoop() 
{
	return getLoopField()->getValue();
}

bool AudioClipNode::isLoop() 
{
	return getLoop();
}

////////////////////////////////////////////////
//	Pitch
////////////////////////////////////////////////

SFFloat *AudioClipNode::getPitchField()
{
	if (isInstanceNode() == false)
		return pitchField;
	return (SFFloat *)getExposedField(pitchFieldString);
}
	
void AudioClipNode::setPitch(float value) 
{
	getPitchField()->setValue(value);
}

float AudioClipNode::getPitch() 
{
	return getPitchField()->getValue();
}

////////////////////////////////////////////////
//	Start time
////////////////////////////////////////////////

SFTime *AudioClipNode::getStartTimeField()
{
	if (isInstanceNode() == false)
		return startTimeField;
	return (SFTime *)getExposedField(startTimeFieldString);
}
	
void AudioClipNode::setStartTime(double value) 
{
	getStartTimeField()->setValue(value);
}

double AudioClipNode::getStartTime() 
{
	return getStartTimeField()->getValue();
}

////////////////////////////////////////////////
//	Stop time
////////////////////////////////////////////////

SFTime *AudioClipNode::getStopTimeField()
{
	if (isInstanceNode() == false)
		return stopTimeField;
	return (SFTime *)getExposedField(stopTimeFieldString);
}
	
void AudioClipNode::setStopTime(double value) 
{
	getStopTimeField()->setValue(value);
}

double AudioClipNode::getStopTime() 
{
	return getStopTimeField()->getValue();
}

////////////////////////////////////////////////
//	isActive
////////////////////////////////////////////////

SFBool *AudioClipNode::getIsActiveField()
{
	if (isInstanceNode() == false)
		return isActiveField;
	return (SFBool *)getEventOut(isActiveFieldString);
}
	
void AudioClipNode::setIsActive(bool  value) 
{
	getIsActiveField()->setValue(value);
}

bool AudioClipNode::getIsActive() 
{
	return getIsActiveField()->getValue();
}

bool AudioClipNode::isActive() 
{
	return getIsActiveField()->getValue();
}

////////////////////////////////////////////////
//	duration_changed
////////////////////////////////////////////////

SFTime *AudioClipNode::getDurationChangedField()
{
	if (isInstanceNode() == false)
		return durationChangedField;
	return (SFTime *)getEventOut(durationFieldString);
}
	
void AudioClipNode::setDurationChanged(double value) 
{
	getDurationChangedField()->setValue(value);
}

double AudioClipNode::getDurationChanged() 
{
	return getDurationChangedField()->getValue();
}

////////////////////////////////////////////////
// Url
////////////////////////////////////////////////

MFString *AudioClipNode::getUrlField()
{
	if (isInstanceNode() == false)
		return urlField;
	return (MFString *)getExposedField(urlFieldString);
}

void AudioClipNode::addUrl(char * value) 
{
	getUrlField()->addValue(value);
}

int AudioClipNode::getNUrls() 
{
	return getUrlField()->getSize();
}

char *AudioClipNode::getUrl(int index) 
{
	return getUrlField()->get1Value(index);
}

void AudioClipNode::setUrl(int index, char *urlString) 
{
	getUrlField()->set1Value(index, urlString);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

AudioClipNode *AudioClipNode::next() 
{
	return (AudioClipNode *)Node::next(getType());
}

AudioClipNode *AudioClipNode::nextTraversal() 
{
	return (AudioClipNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	Virutual functions
////////////////////////////////////////////////
	
bool AudioClipNode::isChildNodeType(Node *node)
{
	return false;
}

void AudioClipNode::outputContext(ostream &printStream, char *indentString) 
{
	SFString *description = getDescriptionField();
	printStream << indentString << "\t" << "description " << description << endl;
	
	printStream << indentString << "\t" << "pitch " << getPitch() << endl;
	printStream << indentString << "\t" << "startTime " << getStartTime() << endl;
	printStream << indentString << "\t" << "stopTime " << getStopTime() << endl;

	SFBool *loop = getLoopField();
	printStream << indentString << "\t" << "loop " << loop << endl;

	if (0 < getNUrls()) {
		MFString *url = getUrlField();
		printStream << indentString << "\t" << "url [" << endl;
		url->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}

////////////////////////////////////////////////
//	Time
////////////////////////////////////////////////

void AudioClipNode::setCurrentTime(double time) 
{
	mCurrentTime = time;
}

double AudioClipNode::getCurrentTime() 
{
	return mCurrentTime;
}

////////////////////////////////////////////////
//	PlayAudioClip
////////////////////////////////////////////////

static void PlayAudioClip(AudioClipNode *ac)
{
#ifdef SUPPORT_SOUND
	if (0 < ac->getNUrls()) {
		char *filename = ac->getUrl(0);
		if (filename) {
#ifdef WIN32
			PlaySound(filename, NULL, SND_FILENAME | SND_ASYNC);
#endif
		}
	}
#endif
}

////////////////////////////////////////////////
//	StopAudioClip
////////////////////////////////////////////////

static void StopAudioClip(AudioClipNode *ac)
{
}

////////////////////////////////////////////////
//	AudioClipNode::initialize
////////////////////////////////////////////////

void AudioClipNode::initialize() 
{
	setIsActive(false);
	StopAudioClip(this);
	setCurrentTime(-1.0);
}

////////////////////////////////////////////////
//	AudioClipNode::uninitialize
////////////////////////////////////////////////

void AudioClipNode::uninitialize() 
{
	StopAudioClip(this);
}

////////////////////////////////////////////////
//	AudioClipNode::update
////////////////////////////////////////////////

void AudioClipNode::update() 
{
	double currentTime = getCurrentTime();

	if (currentTime < 0.0)
		currentTime = GetCurrentSystemTime();

	double startTime = getStartTime();
	double stopTime = getStopTime();

	bool bActive	= isActive();
	bool bLoop		= isLoop();

	if (bActive == false) {
		if (currentTime <= startTime) {
			if (bLoop == true && stopTime <= startTime)
				bActive = true;
			else if	(bLoop == true && startTime < stopTime)
				bActive = true;
			else if (bLoop == false && startTime < stopTime)
				bActive = true;

			if (bActive == true) {
				setIsActive(true);
				sendEvent(getIsActiveField());
				PlayAudioClip(this);
				setIsActive(false);
			}
		}
	}

	currentTime = GetCurrentSystemTime();
	setCurrentTime(currentTime);

/*	
	if (bActive == true) {
		if (bLoop == true && startTime < stopTime) {
			if (stopTime < currentTime)
				bActive = false;
		}
		else if (bLoop == false && stopTime <= startTime) {
			if (startTime + cycleInterval < currentTime)
				bActive = false;
		}

		if (bActive == false) {
			setIsActive(false);
			sendEvent(getIsActiveField());
		}
	}
*/
}

