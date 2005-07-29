/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	OrientationInterpolatorNode.cpp
*
******************************************************************/

#include "VRMLField.h"
#include "OrientationInterpolatorNode.h"

OrientationInterpolatorNode::OrientationInterpolatorNode() 
{
	setHeaderFlag(false);
	setType(orientationInterpolatorNodeString);

	// keyValue exposed field
	keyValueField = new MFRotation();
	addExposedField(keyValueFieldString, keyValueField);

	// value_changed eventOut field
	valueField = new SFRotation(0.0f, 0.0f, 1.0f, 0.0f);
	addEventOut(valueFieldString, valueField);
}

OrientationInterpolatorNode::~OrientationInterpolatorNode() 
{
}

////////////////////////////////////////////////
//	keyValue
////////////////////////////////////////////////
	
MFRotation *OrientationInterpolatorNode::getKeyValueField()
{
	if (isInstanceNode() == false)
		return keyValueField;
	return (MFRotation *)getExposedField(keyValueFieldString);
}

void OrientationInterpolatorNode::addKeyValue(float rotation[]) 
{
	getKeyValueField()->addValue(rotation);
}

int OrientationInterpolatorNode::getNKeyValues() 
{
	return getKeyValueField()->getSize();
}
	
void OrientationInterpolatorNode::getKeyValue(int index, float rotation[]) 
{
	getKeyValueField()->get1Value(index, rotation);
}

////////////////////////////////////////////////
//	value
////////////////////////////////////////////////
	
SFRotation *OrientationInterpolatorNode::getValueField()
{
	if (isInstanceNode() == false)
		return valueField;
	return (SFRotation *)getEventOut(valueFieldString);
}

void OrientationInterpolatorNode::setValue(float rotation[]) 
{
	getValueField()->setValue(rotation);
}

void OrientationInterpolatorNode::getValue(float rotation[]) 
{
	getValueField()->getValue(rotation);
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool OrientationInterpolatorNode::isChildNodeType(Node *node)
{
	return false;
}

void OrientationInterpolatorNode::initialize() 
{
}

void OrientationInterpolatorNode::uninitialize() 
{
}

void OrientationInterpolatorNode::update() 
{
	int	n;

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

	float rotation1[4];
	float rotation2[4];
	float rotationOut[4];

	getKeyValue(index, rotation1);
	getKeyValue(index+1, rotation2);

	for (n=0; n<4; n++)
		rotationOut[n] = rotation1[n] + (rotation2[n] - rotation1[n])*scale;

	setValue(rotationOut);
	sendEvent(getValueField());
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void OrientationInterpolatorNode::outputContext(ostream& printStream, char *indentString) 
{
	if (0 < getNKeys()) {
		MFFloat *key = getKeyField();
		printStream << indentString << "\tkey [" << endl;
		key->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t]" << endl;
	}

	if (0 < getNKeyValues()) {
		MFRotation *keyValue = getKeyValueField();
		printStream << indentString << "\tkeyValue [" << endl;
		keyValue->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t]" << endl;
	}
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

OrientationInterpolatorNode *OrientationInterpolatorNode::next() 
{
	return (OrientationInterpolatorNode *)Node::next(getType());
}

OrientationInterpolatorNode *OrientationInterpolatorNode::nextTraversal() 
{
	return (OrientationInterpolatorNode *)Node::nextTraversalByType(getType());
}

