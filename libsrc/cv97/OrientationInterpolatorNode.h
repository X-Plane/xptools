/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	OrientationInterpolatorNode.h
*
******************************************************************/

#ifndef _CV97_ORIENTATIONINTERPOLATOR_H_
#define _CV97_ORIENTATIONINTERPOLATOR_H_

#include "InterpolatorNode.h"

class OrientationInterpolatorNode : public InterpolatorNode {

	MFRotation *keyValueField;
	SFRotation *valueField;

public:

	OrientationInterpolatorNode();
	~OrientationInterpolatorNode();

	////////////////////////////////////////////////
	//	keyValue
	////////////////////////////////////////////////
	
	MFRotation *getKeyValueField();

	void addKeyValue(float rotation[]);
	int getNKeyValues();
	void getKeyValue(int index, float rotation[]);

	////////////////////////////////////////////////
	//	value
	////////////////////////////////////////////////
	
	SFRotation *getValueField();

	void setValue(float rotation[]);
	void getValue(float rotation[]);

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

	void outputContext(ostream& printStream, char *indentString);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	OrientationInterpolatorNode *next();
	OrientationInterpolatorNode *nextTraversal();
};

#endif

