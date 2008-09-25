/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ColorNode.cpp
*
******************************************************************/

#include "VRMLField.h"
#include "ColorNode.h"

ColorNode::ColorNode()
{
	setHeaderFlag(false);
	setType(colorNodeString);

	// color exposed field
	colorField = new MFColor();
	colorField->setName(colorFieldString);
	addExposedField(colorField);
}

ColorNode::~ColorNode()
{
}

////////////////////////////////////////////////
//	color
////////////////////////////////////////////////

MFColor *ColorNode::getColorField()
{
	if (isInstanceNode() == false)
		return colorField;
	return (MFColor *)getExposedField(colorFieldString);
}

void ColorNode::addColor(float color[])
{
	getColorField()->addValue(color);
}

int ColorNode::getNColors()
{
	return getColorField()->getSize();
}

void ColorNode::getColor(int index, float color[])
{
	getColorField()->get1Value(index, color);
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool ColorNode::isChildNodeType(Node *node)
{
	return false;
}

void ColorNode::initialize()
{
}

void ColorNode::uninitialize()
{
}

void ColorNode::update()
{
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void ColorNode::outputContext(ostream &printStream, char *indentString)
{
	if (0 < getNColors()) {
		MFColor *color = getColorField();
		printStream <<  indentString << "\t" << "color ["  << endl;
		color->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

ColorNode *ColorNode::next()
{
	return (ColorNode *)Node::next(getType());
}

ColorNode *ColorNode::nextTraversal()
{
	return (ColorNode *)Node::nextTraversalByType(getType());
}

