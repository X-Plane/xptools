/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	CylinderSensorNode.h
*
******************************************************************/

#ifndef _CV97_CYLINDERSENSOR_H_
#define _CV97_CYLINDERSENSOR_H_

#include "SensorNode.h"

class CylinderSensorNode : public SensorNode {

	SFBool *autoOffsetField;
	SFFloat *diskAngleField;
	SFFloat *minAngleField;
	SFFloat *maxAngleField;
	SFFloat *offsetField;
	SFRotation *rotationField;
	SFVec3f *trackPointField;

public:

	CylinderSensorNode();
	~CylinderSensorNode();

	////////////////////////////////////////////////
	//	AutoOffset
	////////////////////////////////////////////////

	SFBool *getAutoOffsetField();
	
	void setAutoOffset(bool  value);
	void setAutoOffset(int value);
	bool  getAutoOffset();
	bool  isAutoOffset();

	////////////////////////////////////////////////
	//	DiskAngle
	////////////////////////////////////////////////

	SFFloat *getDiskAngleField();
	
	void setDiskAngle(float value);
	float getDiskAngle();

	////////////////////////////////////////////////
	//	MinAngle
	////////////////////////////////////////////////

	SFFloat *getMinAngleField();
	
	void setMinAngle(float value);
	float getMinAngle();

	////////////////////////////////////////////////
	//	MaxAngle
	////////////////////////////////////////////////

	SFFloat *getMaxAngleField();
	
	void setMaxAngle(float value);
	float getMaxAngle();

	////////////////////////////////////////////////
	//	Offset
	////////////////////////////////////////////////

	SFFloat *getOffsetField();
	
	void setOffset(float value);
	float getOffset();

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

	CylinderSensorNode *next();
	CylinderSensorNode *nextTraversal();

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

