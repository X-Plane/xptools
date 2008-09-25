/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	InterpolatorNode.cpp
*
******************************************************************/

#include "VRMLField.h"
#include "InterpolatorNode.h"

InterpolatorNode::InterpolatorNode()
{
	// key exposed field
	keyField = new MFFloat();
	addExposedField(keyFieldString, keyField);

	// set_fraction eventIn field
	fractionField = new SFFloat(0.0f);
	addEventIn(fractionFieldString, fractionField);
}

InterpolatorNode::~InterpolatorNode()
{
}

////////////////////////////////////////////////
//	key
////////////////////////////////////////////////

MFFloat *InterpolatorNode::getKeyField()
{
	if (isInstanceNode() == false)
		return keyField;
	return (MFFloat *)getExposedField(keyFieldString);
}

void InterpolatorNode::addKey(float value)
{
	getKeyField()->addValue(value);
}

int InterpolatorNode::getNKeys()
{
	return getKeyField()->getSize();
}

float InterpolatorNode::getKey(int index)
{
	return getKeyField()->get1Value(index);
}


////////////////////////////////////////////////
//	fraction
////////////////////////////////////////////////

SFFloat *InterpolatorNode::getFractionField()
{
	if (isInstanceNode() == false)
		return fractionField;
	return (SFFloat *)getEventIn(fractionFieldString);
}

void InterpolatorNode::setFraction(float value)
{
	getFractionField()->setValue(value);
}

float InterpolatorNode::getFraction()
{
	return getFractionField()->getValue();
}


