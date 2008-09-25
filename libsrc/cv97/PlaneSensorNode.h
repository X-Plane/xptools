/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	PlaneSensorNode.h
*
******************************************************************/

#ifndef _CV97_PLANESENSOR_H_
#define _CV97_PLANESENSOR_H_

#include "SensorNode.h"

class PlaneSensorNode : public SensorNode {

	SFBool *autoOffsetField;
	SFVec2f *minPositionField;
	SFVec2f *maxPositionField;
	SFVec3f *offsetField;
	SFVec3f *translationField;
	SFVec3f *trackPointField;

public:

	PlaneSensorNode();
	~PlaneSensorNode();

	////////////////////////////////////////////////
	//	AutoOffset
	////////////////////////////////////////////////

	SFBool *getAutoOffsetField();

	void setAutoOffset(bool value);
	void setAutoOffset(int value);
	bool getAutoOffset();
	bool  isAutoOffset();

	////////////////////////////////////////////////
	//	MinPosition
	////////////////////////////////////////////////

	SFVec2f *getMinPositionField();

	void setMinPosition(float value[]);
	void setMinPosition(float x, float y);
	void getMinPosition(float value[]);
	void getMinPosition(float *x, float *y);

	////////////////////////////////////////////////
	//	MaxPosition
	////////////////////////////////////////////////

	SFVec2f *getMaxPositionField();

	void setMaxPosition(float value[]);
	void setMaxPosition(float x, float y);
	void getMaxPosition(float value[]);
	void getMaxPosition(float *x, float *y);

	////////////////////////////////////////////////
	//	Offset
	////////////////////////////////////////////////

	SFVec3f *getOffsetField();

	void setOffset(float value[]);
	void getOffset(float value[]);

	////////////////////////////////////////////////
	//	Translation
	////////////////////////////////////////////////

	SFVec3f *getTranslationChangedField();

	void setTranslationChanged(float value[]);
	void setTranslationChanged(float x, float y, float z);
	void getTranslationChanged(float value[]);

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

	PlaneSensorNode *next();
	PlaneSensorNode *nextTraversal();

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

