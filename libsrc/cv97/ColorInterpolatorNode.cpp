/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ColorInterpolatorNode.cpp
*
******************************************************************/

#include "VRMLField.h"
#include "ColorInterpolatorNode.h"

ColorInterpolatorNode::ColorInterpolatorNode() 
{
	setHeaderFlag(false);
	setType(colorInterpolatorNodeString);

	// keyValue exposed field
	keyValueField = new MFColor();
	addExposedField(keyValueFieldString, keyValueField);

	// value_changed eventOut field
	valueField = new SFColor(0.0f, 0.0f, 0.0f);
	addEventOut(valueFieldString, valueField);
}

ColorInterpolatorNode::~ColorInterpolatorNode() 
{
}

////////////////////////////////////////////////
//	keyValue
////////////////////////////////////////////////

MFColor *ColorInterpolatorNode::getKeyValueField()
{
	if (isInstanceNode() == false)
		return keyValueField;
	return (MFColor *)getExposedField(keyValueFieldString);
}
	
void ColorInterpolatorNode::addKeyValue(float color[]) 
{
	getKeyValueField()->addValue(color);
}

int ColorInterpolatorNode::getNKeyValues() 
{
	return getKeyValueField()->getSize();
}
	
void ColorInterpolatorNode::getKeyValue(int index, float color[]) 
{
	getKeyValueField()->get1Value(index, color);
}

////////////////////////////////////////////////
//	value
////////////////////////////////////////////////

SFColor *ColorInterpolatorNode::getValueField()
{
	if (isInstanceNode() == false)
		return valueField;
	return (SFColor *)getEventOut(valueFieldString);
}
	
void ColorInterpolatorNode::setValue(float color[]) 
{
	getValueField()->setValue(color);
}

void ColorInterpolatorNode::getValue(float color[]) 
{
	getValueField()->getValue(color);
}


////////////////////////////////////////////////
//	Virtual functions
////////////////////////////////////////////////
	
bool ColorInterpolatorNode::isChildNodeType(Node *node)
{
	return false;
}

void ColorInterpolatorNode::initialize() 
{
}

void ColorInterpolatorNode::uninitialize() 
{
}

void ColorInterpolatorNode::update() 
{
	int n;

	float fraction = getFraction();
	int index = -1;
	int nKey = getNKeys();
	for (n=0; n<(nKey-1); n++) {
		if (getKey(n) <= fraction && fraction <= getKey(n+1)) {
			index = n;
			break;
		}
	}
	if (index == -1)
		return;

	float scale = (fraction - getKey(index)) / (getKey(index+1) - getKey(index));	
	float color1[3];
	float color2[3];
	float colorOut[3];

	getKeyValue(index, color1);
	getKeyValue(index+1, color2);
	for (n=0; n<3; n++)
		colorOut[n] = color1[n] + (color2[n] - color1[n])*scale;

	setValue(colorOut);
	sendEvent(getValueField());
}

void ColorInterpolatorNode::outputContext(ostream &printStream, char *indentString) 
{
	if (0 < getNKeys()) {
		MFFloat *key = getKeyField();	
		printStream << indentString << "\tkey [" << endl;
		key->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t]" << endl;
	}

	if (0 < getNKeyValues()) {
		MFColor *keyValue = getKeyValueField();
		printStream << indentString << "\tkeyValue [" << endl;
		keyValue->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t]" << endl;
	}
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

ColorInterpolatorNode *ColorInterpolatorNode::next() 
{
	return (ColorInterpolatorNode *)Node::next(getType());
}

ColorInterpolatorNode *ColorInterpolatorNode::nextTraversal() 
{
	return (ColorInterpolatorNode *)Node::nextTraversalByType(getType());
}

