/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	InterpolatorNode.h
*
******************************************************************/

#ifndef _CV97_INTERPOLATOR_H_
#define _CV97_INTERPOLATOR_H_

#include "Node.h"

class InterpolatorNode : public Node {

	MFFloat *keyField;
	SFFloat *fractionField;

public:

	InterpolatorNode();
	~InterpolatorNode();

	////////////////////////////////////////////////
	//	key
	////////////////////////////////////////////////

	MFFloat *getKeyField();
	
	void addKey(float value);
	int getNKeys();
	float getKey(int index);

	////////////////////////////////////////////////
	//	fraction
	////////////////////////////////////////////////
	
	SFFloat *getFractionField();

	void setFraction(float value);
	float getFraction();
};

#endif
