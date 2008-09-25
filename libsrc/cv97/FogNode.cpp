/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	FogNode.cpp
*
******************************************************************/

#include "FogNode.h"

FogNode::FogNode()
{
	setHeaderFlag(false);
	setType(fogNodeString);

	///////////////////////////
	// Exposed Field
	///////////////////////////

	// color exposed field
	colorField = new SFColor(1.0f, 1.0f, 1.0f);
	addExposedField(colorFieldString, colorField);

	// fogType exposed field
	fogTypeField = new SFString("LINEAR");
	addExposedField(fogTypeFieldString, fogTypeField);

	// visibilityRange exposed field
	visibilityRangeField = new SFFloat(0.0f);
	addExposedField(visibilityRangeFieldString, visibilityRangeField);
}

FogNode::~FogNode()
{
}

////////////////////////////////////////////////
//	Color
////////////////////////////////////////////////

SFColor *FogNode::getColorField()
{
	if (isInstanceNode() == false)
		return colorField;
	return (SFColor *)getExposedField(colorFieldString);
}

void FogNode::setColor(float value[])
{
	getColorField()->setValue(value);
}

void FogNode::setColor(float r, float g, float b)
{
	getColorField()->setValue(r, g, b);
}

void FogNode::getColor(float value[])
{
	getColorField()->getValue(value);
}

////////////////////////////////////////////////
//	FogType
////////////////////////////////////////////////

SFString *FogNode::getFogTypeField()
{
	if (isInstanceNode() == false)
		return fogTypeField;
	return (SFString *)getExposedField(fogTypeFieldString);
}

void FogNode::setFogType(char *value)
{
	getFogTypeField()->setValue(value);
}

char *FogNode::getFogType()
{
	return getFogTypeField()->getValue();
}

////////////////////////////////////////////////
//	VisibilityRange
////////////////////////////////////////////////

SFFloat *FogNode::getVisibilityRangeField()
{
	if (isInstanceNode() == false)
		return visibilityRangeField;
	return (SFFloat *)getExposedField(visibilityRangeFieldString);
}

void FogNode::setVisibilityRange(float value)
{
	getVisibilityRangeField()->setValue(value);
}

float FogNode::getVisibilityRange()
{
	return getVisibilityRangeField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

FogNode *FogNode::next()
{
	return (FogNode *)Node::next(getType());
}

FogNode *FogNode::nextTraversal()
{
	return (FogNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool FogNode::isChildNodeType(Node *node)
{
	return false;
}

void FogNode::initialize()
{
}

void FogNode::uninitialize()
{
}

void FogNode::update()
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void FogNode::outputContext(ostream &printStream, char *indentString)
{
	SFColor *color = getColorField();
	SFString *fogType = getFogTypeField();

	printStream << indentString << "\t" << "color " << color << endl;
	printStream << indentString << "\t" << "fogType " << fogType << endl;
	printStream << indentString << "\t" << "visibilityRange " << getVisibilityRange() << endl;
}
