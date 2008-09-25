/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SoundNode.h
*
******************************************************************/

#ifndef _CV97_SOUND_H_
#define _CV97_SOUND_H_

#include "VRMLField.h"
#include "Node.h"

class SoundNode : public Node {

	SFFloat *minFrontField;
	SFFloat *maxFrontField;
	SFFloat *minBackField;
	SFFloat *maxBackField;
	SFFloat *intensityField;
	SFFloat *priorityField;
	SFVec3f *directionField;
	SFVec3f *locationField;
	SFBool *spatializeField;

public:

	SoundNode();
	~SoundNode();

	////////////////////////////////////////////////
	//	Direction
	////////////////////////////////////////////////

	SFVec3f *getDirectionField();

	void setDirection(float value[]);
	void setDirection(float x, float y, float z);
	void getDirection(float value[]);

	////////////////////////////////////////////////
	//	Location
	////////////////////////////////////////////////

	SFVec3f *getLocationField();

	void setLocation(float value[]);
	void setLocation(float x, float y, float z);
	void getLocation(float value[]);

	////////////////////////////////////////////////
	//	MinFront
	////////////////////////////////////////////////

	SFFloat *getMinFrontField();

	void setMinFront(float value);
	float getMinFront();

	////////////////////////////////////////////////
	//	MaxFront
	////////////////////////////////////////////////

	SFFloat *getMaxFrontField();

	void setMaxFront(float value);
	float getMaxFront();

	////////////////////////////////////////////////
	//	MinBack
	////////////////////////////////////////////////

	SFFloat *getMinBackField();

	void setMinBack(float value);
	float getMinBack();

	////////////////////////////////////////////////
	//	MaxBack
	////////////////////////////////////////////////

	SFFloat *getMaxBackField();

	void setMaxBack(float value);
	float getMaxBack();

	////////////////////////////////////////////////
	//	Intensity
	////////////////////////////////////////////////

	SFFloat *getIntensityField();

	void setIntensity(float value);
	float getIntensity();

	////////////////////////////////////////////////
	//	Priority
	////////////////////////////////////////////////

	SFFloat *getPriorityField();

	void setPriority(float value);
	float getPriority();

	////////////////////////////////////////////////
	//	Spatialize
	////////////////////////////////////////////////

	SFBool *getSpatializeField();

	void setSpatialize(bool value);
	void setSpatialize(int value);
	bool getSpatialize();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	SoundNode *next();
	SoundNode *nextTraversal();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////

	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	Infomation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif

