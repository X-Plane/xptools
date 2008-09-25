/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SensorNode.cpp
*
******************************************************************/

#include "VRMLField.h"
#include "SensorNode.h"

SensorNode::SensorNode()
{
	// enabled exposed field
	enabledField = new SFBool(true);
	addExposedField(enabledFieldString, enabledField);

	// isActive eventOut field
	isActiveField = new SFBool(false);
	addEventOut(isActiveFieldString, isActiveField);
}

SensorNode::~SensorNode()
{
}

////////////////////////////////////////////////
//	Enabled
////////////////////////////////////////////////

SFBool *SensorNode::getEnabledField()
{
	if (isInstanceNode() == false)
		return enabledField;
	return (SFBool *)getExposedField(enabledFieldString);
}

void SensorNode::setEnabled(bool  value)
{
	getEnabledField()->setValue(value);
}

void SensorNode::setEnabled(int value)
{
	setEnabled(value ? true : false);
}

bool  SensorNode::getEnabled()
{
	return getEnabledField()->getValue();
}

bool  SensorNode::isEnabled()
{
	return getEnabled();
}

////////////////////////////////////////////////
//	isActive
////////////////////////////////////////////////

SFBool *SensorNode::getIsActiveField()
{
	if (isInstanceNode() == false)
		return isActiveField;
	return (SFBool *)getEventOut(isActiveFieldString);
}

void SensorNode::setIsActive(bool  value)
{
	getIsActiveField()->setValue(value);
}

void SensorNode::setIsActive(int value)
{
	setIsActive(value ? true : false);
}

bool  SensorNode::getIsActive()
{
	return getIsActiveField()->getValue();
}

bool SensorNode::isActive()
{
	return getIsActive();
}

