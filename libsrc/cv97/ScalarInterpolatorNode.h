/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ScalarInterpolatorNode.h
*
******************************************************************/

#ifndef _CV97_SCALARINTERPOLATOR_H_
#define _CV97_SCALARINTERPOLATOR_H_

#include "InterpolatorNode.h"

class ScalarInterpolatorNode : public InterpolatorNode {

	MFFloat *keyValueField;
	SFFloat *valueField;

public:

	ScalarInterpolatorNode();
	~ScalarInterpolatorNode();

	////////////////////////////////////////////////
	//	keyValue
	////////////////////////////////////////////////
	
	MFFloat *getKeyValueField();

	void addKeyValue(float value);
	int getNKeyValues();
	float getKeyValue(int index);

	////////////////////////////////////////////////
	//	value
	////////////////////////////////////////////////
	
	SFFloat *getValueField();

	void setValue(float vector);
	float getValue();

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

	ScalarInterpolatorNode *next();
	ScalarInterpolatorNode *nextTraversal();
};

#endif
