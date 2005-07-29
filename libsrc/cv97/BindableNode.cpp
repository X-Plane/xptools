/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	BindableNode.cpp
*
******************************************************************/

#include "BindableNode.h"

BindableNode::BindableNode() 
{
	// set_bind
	setBindField = new SFBool(true);
	addEventIn(setBindFieldString, setBindField);

	// cybleInterval exposed field
	bindTimeField = new SFTime(1.0);
	addEventOut(bindTimeFieldString, bindTimeField);

	// isBind
	isBoundField = new SFBool(true);
	addEventOut(isBoundFieldString, isBoundField);
}

BindableNode::~BindableNode() 
{
}

////////////////////////////////////////////////
//	bind
////////////////////////////////////////////////

SFBool *BindableNode::getBindField() 
{
	if (isInstanceNode() == false)
		return setBindField;
	return (SFBool *)getEventIn(setBindFieldString);
}

void BindableNode::setBind(bool value) 
{
	getBindField()->setValue(value);
}

bool BindableNode::getBind() 
{
	return getBindField()->getValue();
}

bool BindableNode::isBind() 
{
	return getBind();
}

////////////////////////////////////////////////
//	bindTime
////////////////////////////////////////////////

SFTime *BindableNode::getBindTimeField()
{
	if (isInstanceNode() == false)
		return bindTimeField;
	return (SFTime *)getEventOut(bindTimeFieldString);
}

void BindableNode::setBindTime(double value) 
{
	getBindTimeField()->setValue(value);
}

double BindableNode::getBindTime() 
{
	return getBindTimeField()->getValue();
}

////////////////////////////////////////////////
//	isBound
////////////////////////////////////////////////

SFBool *BindableNode::getIsBoundField()
{
	if (isInstanceNode() == false)
		return isBoundField;
	return (SFBool *)getEventOut(isBoundFieldString);
}

void BindableNode::setIsBound(bool value) 
{
	getIsBoundField()->setValue(value);
}

bool BindableNode::getIsBound() 
{
	return getIsBoundField()->getValue();
}

bool BindableNode::isBound() 
{
	return getIsBound();
}

