/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ProximitySensorNode.h
*
******************************************************************/

#ifndef _CV97_PROXIMITYSENSOR_H_
#define _CV97_PROXIMITYSENSOR_H_

#include "SensorNode.h"

class ProximitySensorNode : public SensorNode {

	SFVec3f *centerField;
	SFVec3f *sizeField;
	SFVec3f *positionField;
	SFRotation *orientationField;
	SFTime *enterTimeField;
	SFTime *exitTimeField;
	SFBool *inRegionField;
	
public:

	ProximitySensorNode();
	~ProximitySensorNode();

	////////////////////////////////////////////////
	//	Center
	////////////////////////////////////////////////

	SFVec3f *getCenterField();
	
	void setCenter(float value[]);
	void setCenter(float x, float y, float z);
	void getCenter(float value[]);

	////////////////////////////////////////////////
	//	Size
	////////////////////////////////////////////////

	SFVec3f *getSizeField();
	
	void setSize(float value[]);
	void setSize(float x, float y, float z);
	void getSize(float value[]);

	////////////////////////////////////////////////
	//	Position
	////////////////////////////////////////////////

	SFVec3f *getPositionChangedField();
	
	void setPositionChanged(float value[]);
	void setPositionChanged(float x, float y, float z);
	void getPositionChanged(float value[]);

	////////////////////////////////////////////////
	//	Orientation
	////////////////////////////////////////////////

	SFRotation *getOrientationChangedField();
	
	void setOrientationChanged(float value[]);
	void setOrientationChanged(float x, float y, float z, float rot);
	void getOrientationChanged(float value[]);

	////////////////////////////////////////////////
	//	EnterTime
	////////////////////////////////////////////////
	
	SFTime *getEnterTimeField();

	void setEnterTime(double value);
	double getEnterTime();

	////////////////////////////////////////////////
	//	ExitTime
	////////////////////////////////////////////////

	SFTime *getExitTimeField();
	
	void setExitTime(double value);
	double getExitTime();

	////////////////////////////////////////////////
	//	inRegion
	////////////////////////////////////////////////

	SFBool *getInRegionField();

	void setInRegion(bool value);
	bool inRegion();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	ProximitySensorNode *next();
	ProximitySensorNode *nextTraversal();

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

