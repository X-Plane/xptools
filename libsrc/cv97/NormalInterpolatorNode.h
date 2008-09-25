/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	NormalInterpolatorNode.h
*
******************************************************************/

#ifndef _CV97_NORMALINTERPOLATOR_H_
#define _CV97_NORMALINTERPOLATOR_H_

#include "InterpolatorNode.h"

class NormalInterpolatorNode : public InterpolatorNode {

	MFVec3f *keyValueField;
	SFVec3f *valueField;

public:

	NormalInterpolatorNode();
	~NormalInterpolatorNode();

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
	//	functions
	////////////////////////////////////////////////

	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	Output
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	NormalInterpolatorNode *next();
	NormalInterpolatorNode *nextTraversal();

};

#endif

