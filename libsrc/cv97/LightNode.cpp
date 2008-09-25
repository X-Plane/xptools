/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	LightNode.cpp
*
******************************************************************/

#include "LightNode.h"

LightNode::LightNode()
{
	setHeaderFlag(false);

	// on exposed field
	bonField = new SFBool(true);
	bonField->setName(onFieldString);
	addExposedField(bonField);

	// intensity exposed field
	intensityField = new SFFloat(1.0f);
	intensityField->setName(intensityFieldString);
	addExposedField(intensityField);

	// color exposed field
	colorField = new SFColor(1.0f, 1.0f, 1.0f);
	colorField->setName(colorFieldString);
	addExposedField(colorField);
}

LightNode::~LightNode()
{
}

////////////////////////////////////////////////
//	On
////////////////////////////////////////////////

SFBool *LightNode::getOnField()
{
	if (isInstanceNode() == false)
		return bonField;
	return (SFBool *)getExposedField(onFieldString);
}

void LightNode::setOn(bool on)
{
	getOnField()->setValue(on);
}

void LightNode::setOn(int value)
{
	setOn(value ? true : false);
}

bool LightNode::isOn()
{
	return getOnField()->getValue();
}

////////////////////////////////////////////////
//	Intensity
////////////////////////////////////////////////

SFFloat *LightNode::getIntensityField()
{
	if (isInstanceNode() == false)
		return intensityField;
	return (SFFloat *)getExposedField(intensityFieldString);
}

void LightNode::setIntensity(float value)
{
	getIntensityField()->setValue(value);
}

float LightNode::getIntensity()
{
	return getIntensityField()->getValue();
}

////////////////////////////////////////////////
//	Color
////////////////////////////////////////////////

SFColor *LightNode::getColorField()
{
	if (isInstanceNode() == false)
		return colorField;
	return (SFColor *)getExposedField(colorFieldString);
}

void LightNode::setColor(float value[])
{
	getColorField()->setValue(value);
}

void LightNode::setColor(float r, float g, float b)
{
	getColorField()->setValue(r, g, b);
}

void LightNode::getColor(float value[])
{
	getColorField()->getValue(value);
}

