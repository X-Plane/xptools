/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	AudioClipNode.h
*
******************************************************************/

#ifndef _CV97_AUDIOCLLIP_H_
#define _CV97_AUDIOCLLIP_H_

#include "VRMLField.h"
#include "Node.h"

#define isPlayingPrivateFieldName		"isPlaying"

class AudioClipNode : public Node {

	SFString *descriptionField;
	SFBool *loopField;
	SFTime *startTimeField;
	SFTime *stopTimeField;
	SFFloat *pitchField;
	MFString *urlField;
	SFBool *isActiveField;
	SFTime *durationChangedField;

	double	mCurrentTime;

public:

	AudioClipNode();
	~AudioClipNode();

	////////////////////////////////////////////////
	//	Description
	////////////////////////////////////////////////

	SFString *getDescriptionField();

	void setDescription(char * value);
	char *getDescription();

	////////////////////////////////////////////////
	//	Loop
	////////////////////////////////////////////////

	SFBool *getLoopField();

	void setLoop(bool value);
	void setLoop(int value);
	bool getLoop();
	bool isLoop();

	////////////////////////////////////////////////
	//	Pitch
	////////////////////////////////////////////////

	SFFloat *getPitchField();

	void setPitch(float value);
	float getPitch();

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
	//	isActive
	////////////////////////////////////////////////

	SFBool *getIsActiveField();

	void setIsActive(bool  value);
	bool  getIsActive();
	bool  isActive();

	////////////////////////////////////////////////
	//	duration_changed
	////////////////////////////////////////////////

	SFTime *getDurationChangedField();

	void setDurationChanged(double value);
	double getDurationChanged();

	////////////////////////////////////////////////
	// Url
	////////////////////////////////////////////////

	MFString *getUrlField();

	void addUrl(char * value);
	int getNUrls();
	char *getUrl(int index);
	void setUrl(int index, char *urlString);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	AudioClipNode *next();
	AudioClipNode *nextTraversal();

	////////////////////////////////////////////////
	//	Virutual functions
	////////////////////////////////////////////////

	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	void outputContext(ostream &printStream, char *indentString);

	////////////////////////////////////////////////
	//	Time
	////////////////////////////////////////////////

	void setCurrentTime(double time);
	double getCurrentTime();
};

#endif
