/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	PositionInterpolatorNode.h
*
******************************************************************/

#ifndef _CV97_POSITIONINTERPOLATOR_H_
#define _CV97_POSITIONINTERPOLATOR_H_

#include "InterpolatorNode.h"

class PositionInterpolatorNode : public InterpolatorNode {

	MFVec3f *keyValueField;
	SFVec3f *valueField;

public:

	PositionInterpolatorNode();
	~PositionInterpolatorNode();

	////////////////////////////////////////////////
	//	keyValue
	////////////////////////////////////////////////
	
	MFVec3f *getKeyValueField();

	void addKeyValue(float vector[]);
	int getNKeyValues();
	void getKeyValue(int index, float vector[]);

	////////////////////////////////////////////////
	//	value
	////////////////////////////////////////////////
	
	SFVec3f *getValueField();

	void setValue(float vector[]);
	void getValue(float vector[]);

	////////////////////////////////////////////////
	//	Virtual functions
	////////////////////////////////////////////////
	
	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	void outputContext(ostream &printStream, char *indentString);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	PositionInterpolatorNode *next();
	PositionInterpolatorNode *nextTraversal();
};

#endif

