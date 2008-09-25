/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SphereSensorNode.h
*
******************************************************************/

#ifndef _CV97_SPHERESENSOR_H_
#define _CV97_SPHERESENSOR_H_

#include "SensorNode.h"

class SphereSensorNode : public SensorNode {

	SFBool *autoOffsetField;
	SFRotation *offsetField;
	SFRotation *rotationField;
	SFVec3f *trackPointField;

public:

	SphereSensorNode();
	~SphereSensorNode();

	////////////////////////////////////////////////
	//	AutoOffset
	////////////////////////////////////////////////

	SFBool *getAutoOffsetField();

	void setAutoOffset(bool value);
	void setAutoOffset(int value);
	bool getAutoOffset();
	bool  isAutoOffset();

	////////////////////////////////////////////////
	//	Offset
	////////////////////////////////////////////////

	SFRotation *getOffsetField();

	void setOffset(float value[]);
	void getOffset(float value[]);

	////////////////////////////////////////////////
	//	Rotation
	////////////////////////////////////////////////

	SFRotation *getRotationChangedField();

	void setRotationChanged(float value[]);
	void setRotationChanged(float x, float y, float z, float rot);
	void getRotationChanged(float value[]);

	////////////////////////////////////////////////
	//	TrackPoint
	////////////////////////////////////////////////

	SFVec3f *getTrackPointChangedField();

	void setTrackPointChanged(float value[]);
	void setTrackPointChanged(float x, float y, float z);
	void getTrackPointChanged(float value[]);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	SphereSensorNode *next();
	SphereSensorNode *nextTraversal();

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

