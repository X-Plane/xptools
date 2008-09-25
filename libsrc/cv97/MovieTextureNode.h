/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MovieTextureNode.h
*
******************************************************************/

#ifndef _CV97_MOVIETEXTURE_H_
#define _CV97_MOVIETEXTURE_H_

#include "VRMLField.h"
#include "TextureNode.h"

class MovieTextureNode : public TextureNode {

	MFString *urlField;
	SFBool *loopField;
	SFTime *startTimeField;
	SFTime *stopTimeField;
	SFFloat *speedField;
	SFBool *isActiveField;
	SFTime *durationChangedField;

public:

	MovieTextureNode();
	~MovieTextureNode();

	////////////////////////////////////////////////
	// Url
	////////////////////////////////////////////////

	MFString *getUrlField();

	void addUrl(char *value);
	int getNUrls();
	char *getUrl(int index);
	void setUrl(int index, char *urlString);

	////////////////////////////////////////////////
	//	Loop
	////////////////////////////////////////////////

	SFBool *getLoopField();

	void setLoop(bool value);
	void setLoop(int value);
	bool getLoop();
	bool isLoop();

	////////////////////////////////////////////////
	//	Speed
	////////////////////////////////////////////////

	SFFloat *getSpeedField();

	void setSpeed(float value);
	float getSpeed();

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

	void setIsActive(bool value);
	bool getIsActive();
	bool isActive();

	////////////////////////////////////////////////
	//	duration_changed
	////////////////////////////////////////////////

	SFTime *getDurationChangedField();

	void setDurationChanged(double value);
	double getDurationChanged();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	MovieTextureNode *next();
	MovieTextureNode *nextTraversal();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////

	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	Urlmation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif
