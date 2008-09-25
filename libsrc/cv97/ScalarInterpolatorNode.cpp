/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ScalarInterpolatorNode.cpp
*
******************************************************************/

#include "VRMLField.h"
#include "ScalarInterpolatorNode.h"

ScalarInterpolatorNode::ScalarInterpolatorNode()
{
	setHeaderFlag(false);
	setType(scalarInterpolatorNodeString);

	// keyValue exposed field
	keyValueField = new MFFloat();
	addExposedField(keyValueFieldString, keyValueField);

	// value_changed eventOut field
	valueField = new SFFloat(0.0f);
	addEventOut(valueFieldString, valueField);
}

ScalarInterpolatorNode::~ScalarInterpolatorNode()
{
}

////////////////////////////////////////////////
//	keyValue
////////////////////////////////////////////////

MFFloat *ScalarInterpolatorNode::getKeyValueField()
{
	if (isInstanceNode() == false)
		return keyValueField;
	return (MFFloat *)getExposedField(keyValueFieldString);
}

void ScalarInterpolatorNode::addKeyValue(float value)
{
	getKeyValueField()->addValue(value);
}

int ScalarInterpolatorNode::getNKeyValues()
{
	return getKeyValueField()->getSize();
}

float ScalarInterpolatorNode::getKeyValue(int index)
{
	return getKeyValueField()->get1Value(index);
}

////////////////////////////////////////////////
//	value
////////////////////////////////////////////////

SFFloat *ScalarInterpolatorNode::getValueField()
{
	if (isInstanceNode() == false)
		return valueField;
	return (SFFloat *)getEventOut(valueFieldString);
}

void ScalarInterpolatorNode::setValue(float vector)
{
	getValueField()->setValue(vector);
}

float ScalarInterpolatorNode::getValue()
{
	return getValueField()->getValue();
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool ScalarInterpolatorNode::isChildNodeType(Node *node)
{
	return false;
}

void ScalarInterpolatorNode::initialize()
{
}

void ScalarInterpolatorNode::uninitialize()
{
}

void ScalarInterpolatorNode::update()
{

	float fraction = getFraction();
	int index = -1;
	int nKey = getNKeys();
	for (int n=0; n<(nKey-1); n++) {
		if (getKey(n) <= fraction && fraction <= getKey(n+1)) {
			index = n;
			break;
		}
	}
	if (index == -1)
		return;

	float scale = (fraction - getKey(index)) / (getKey(index+1) - getKey(index));

	float value1 = getKeyValue(index);
	float value2 = getKeyValue(index+1);
	float valueOut = value1 + (value2 - value1)*scale;

	setValue(valueOut);
	sendEvent(getValueField());
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void ScalarInterpolatorNode::outputContext(ostream &printStream, char *indentString)
{
	if (0 < getNKeys()) {
		MFFloat *key = getKeyField();
		printStream << indentString << "\tkey [" << endl;
		key->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t]" << endl;
	}

	if (0 < getNKeyValues()) {
		MFFloat *keyValue = getKeyValueField();
		printStream << indentString << "\tkeyValue [" << endl;
		keyValue->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t]" << endl;
	}
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

ScalarInterpolatorNode *ScalarInterpolatorNode::next()
{
	return (ScalarInterpolatorNode *)Node::next(getType());
}

ScalarInterpolatorNode *ScalarInterpolatorNode::nextTraversal()
{
	return (ScalarInterpolatorNode *)Node::nextTraversalByType(getType());
}
