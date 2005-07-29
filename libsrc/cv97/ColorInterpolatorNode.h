/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ColorInterpolatorNode.h
*
******************************************************************/

#ifndef _CV97_COLORINTERPOLATOR_H_
#define _CV97_COLORINTERPOLATOR_H_

#include "InterpolatorNode.h"

class ColorInterpolatorNode : public InterpolatorNode {

	MFColor *keyValueField;
	SFColor *valueField;

public:

	ColorInterpolatorNode();
	~ColorInterpolatorNode();

	////////////////////////////////////////////////
	//	keyValue
	////////////////////////////////////////////////
	
	MFColor *getKeyValueField();

	void addKeyValue(float color[]);
	int getNKeyValues();
	void getKeyValue(int index, float color[]);

	////////////////////////////////////////////////
	//	value
	////////////////////////////////////////////////
	
	SFColor *getValueField();

	void setValue(float color[]);
	void getValue(float color[]);

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

	ColorInterpolatorNode *next();
	ColorInterpolatorNode *nextTraversal();
};

#endif
