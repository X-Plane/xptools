/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	NavigationInfoNode.h
*
******************************************************************/

#ifndef _CV97_NAVIGATIONINFO_H_
#define _CV97_NAVIGATIONINFO_H_

#include "BindableNode.h"

class NavigationInfoNode : public BindableNode {

	SFFloat *visibilityLimitField;
	MFFloat *avatarSizeField;
	MFString *typeField;
	SFBool *headlightField;
	SFFloat *speedField;
	
public:

	NavigationInfoNode();
	~NavigationInfoNode();

	////////////////////////////////////////////////
	// Type
	////////////////////////////////////////////////

	MFString *getTypeField();

	void addType(char *value);
	int getNTypes();
	char *getType(int index);

	////////////////////////////////////////////////
	// avatarSize
	////////////////////////////////////////////////

	MFFloat *getAvatarSizeField();

	void addAvatarSize(float value);
	int getNAvatarSizes();
	float getAvatarSize(int index);

	////////////////////////////////////////////////
	//	Headlight
	////////////////////////////////////////////////

	SFBool *getHeadlightField();
	
	void setHeadlight(bool value);
	void setHeadlight(int value);
	bool getHeadlight();

	////////////////////////////////////////////////
	//	VisibilityLimit
	////////////////////////////////////////////////

	SFFloat *getVisibilityLimitField();

	void setVisibilityLimit(float value);
	float getVisibilityLimit();

	////////////////////////////////////////////////
	//	Speed
	////////////////////////////////////////////////
	
	SFFloat *getSpeedField();

	void setSpeed(float value);
	float getSpeed();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	bool isChildNodeType(Node *node);
	NavigationInfoNode *next();
	NavigationInfoNode *nextTraversal();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////
	
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	infomation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif

