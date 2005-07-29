/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	TouchSensorNode.h
*
******************************************************************/

#ifndef _CV97_TOUCHSENSOR_H_
#define _CV97_TOUCHSENSOR_H_

#include "SensorNode.h"

class TouchSensorNode : public SensorNode {

	SFVec3f *hitNormalField;
	SFVec2f *hitTexCoordField;
	SFVec3f *hitPointField;
	SFBool *isOverField;
	SFTime *touchTimeField;
	
public:

	TouchSensorNode();
	~TouchSensorNode();

	////////////////////////////////////////////////
	//	isOver
	////////////////////////////////////////////////
	
	SFBool *getIsOverField();

	void setIsOver(bool  value);
	void setIsOver(int value);
	bool  getIsOver();
	bool  isOver();

	////////////////////////////////////////////////
	//	hitNormal
	////////////////////////////////////////////////
	
	SFVec3f *getHitNormalChangedField();

	void setHitNormalChanged(float value[]);
	void setHitNormalChanged(float x, float y, float z);
	void getHitNormalChanged(float value[]);

	////////////////////////////////////////////////
	//	hitPoint
	////////////////////////////////////////////////
	
	SFVec3f *getHitPointChangedField();

	void setHitPointChanged(float value[]);
	void setHitPointChanged(float x, float y, float z);
	void getHitPointChanged(float value[]);

	////////////////////////////////////////////////
	//	hitTexCoord
	////////////////////////////////////////////////
	
	SFVec2f *getHitTexCoordField();

	void setHitTexCoord(float value[]);
	void setHitTexCoord(float x, float y);
	void getHitTexCoord(float value[]);

	////////////////////////////////////////////////
	//	ExitTime
	////////////////////////////////////////////////
	
	SFTime *getTouchTimeField();

	void setTouchTime(double value);
	double getTouchTime();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	TouchSensorNode *next();
	TouchSensorNode *nextTraversal();

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
