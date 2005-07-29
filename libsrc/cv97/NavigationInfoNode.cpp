/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	NavigationInfoNode.cpp
*
******************************************************************/

#include "NavigationInfoNode.h"

NavigationInfoNode::NavigationInfoNode() 
{
	setHeaderFlag(false);
	setType(navigationInfoNodeString);

	///////////////////////////
	// Exposed Field 
	///////////////////////////

	// visibilityLimit exposed field
	visibilityLimitField = new SFFloat(0.0f);
	addExposedField(visibilityLimitFieldString, visibilityLimitField);

	// avatarSize exposed field
	avatarSizeField = new MFFloat();
	addExposedField(avatarSizeFieldString, avatarSizeField);

	// type exposed field
	typeField = new MFString();
	addExposedField(typeFieldString, typeField);

	// headlight exposed field
	headlightField = new SFBool(false);
	addExposedField(headlightFieldString, headlightField);

	// speed exposed field
	speedField = new SFFloat(1.0f);
	addExposedField(speedFieldString, speedField);
}

NavigationInfoNode::~NavigationInfoNode() 
{
}

////////////////////////////////////////////////
// Type
////////////////////////////////////////////////

MFString *NavigationInfoNode::getTypeField()
{
	if (isInstanceNode() == false)
		return typeField;
	return (MFString *)getExposedField(typeFieldString);
}

void NavigationInfoNode::addType(char *value) 
{
	getTypeField()->addValue(value);
}

int NavigationInfoNode::getNTypes() 
{
	return getTypeField()->getSize();
}

char *NavigationInfoNode::getType(int index) 
{
	return getTypeField()->get1Value(index);
}

////////////////////////////////////////////////
// avatarSize
////////////////////////////////////////////////

MFFloat *NavigationInfoNode::getAvatarSizeField()
{
	if (isInstanceNode() == false)
		return avatarSizeField;
	return (MFFloat *)getExposedField(avatarSizeFieldString);
}

void NavigationInfoNode::addAvatarSize(float value) 
{
	getAvatarSizeField()->addValue(value);
}

int NavigationInfoNode::getNAvatarSizes() 
{
	return getAvatarSizeField()->getSize();
}

float NavigationInfoNode::getAvatarSize(int index) 
{
	return getAvatarSizeField()->get1Value(index);
}

////////////////////////////////////////////////
//	Headlight
////////////////////////////////////////////////

SFBool *NavigationInfoNode::getHeadlightField()
{
	if (isInstanceNode() == false)
		return headlightField;
	return (SFBool *)getExposedField(headlightFieldString);
}
	
void NavigationInfoNode::setHeadlight(bool value) 
{
	getHeadlightField()->setValue(value);
}

void NavigationInfoNode::setHeadlight(int value) 
{
	setHeadlight(value ? true : false);
}

bool NavigationInfoNode::getHeadlight() 
{
	return getHeadlightField()->getValue();
}

////////////////////////////////////////////////
//	VisibilityLimit
////////////////////////////////////////////////

SFFloat *NavigationInfoNode::getVisibilityLimitField()
{
	if (isInstanceNode() == false)
		return visibilityLimitField;
	return (SFFloat *)getExposedField(visibilityLimitFieldString);
}

void NavigationInfoNode::setVisibilityLimit(float value) 
{
	getVisibilityLimitField()->setValue(value);
}

float NavigationInfoNode::getVisibilityLimit() 
{
	return getVisibilityLimitField()->getValue();
}

////////////////////////////////////////////////
//	Speed
////////////////////////////////////////////////

SFFloat *NavigationInfoNode::getSpeedField()
{
	if (isInstanceNode() == false)
		return speedField;
	return (SFFloat *)getExposedField(speedFieldString);
}
	
void NavigationInfoNode::setSpeed(float value) 
{
	getSpeedField()->setValue(value);
}

float NavigationInfoNode::getSpeed() 
{
	return getSpeedField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

bool NavigationInfoNode::isChildNodeType(Node *node)
{
	return false;
}

NavigationInfoNode *NavigationInfoNode::next() 
{
	return (NavigationInfoNode *)Node::next(Node::getType());
}

NavigationInfoNode *NavigationInfoNode::nextTraversal() 
{
	return (NavigationInfoNode *)Node::nextTraversalByType(Node::getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
void NavigationInfoNode::initialize() 
{
}

void NavigationInfoNode::uninitialize() 
{
}

void NavigationInfoNode::update() 
{
}

////////////////////////////////////////////////
//	infomation
////////////////////////////////////////////////

void NavigationInfoNode::outputContext(ostream &printStream, char *indentString) 
{
	SFBool *headlight = getHeadlightField();

	printStream << indentString << "\t" << "visibilityLimit " << getVisibilityLimit() << endl;
	printStream << indentString << "\t" << "headlight " << headlight << endl;
	printStream << indentString << "\t" << "speed " << getSpeed() << endl;

	if (0 < getNTypes()) {
		MFString *type = getTypeField();
		printStream << indentString << "\t" << "type [" << endl;
		type->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNAvatarSizes()) {
		MFFloat *avatarSize = getAvatarSizeField();
		printStream << indentString << "\t" << "avatarSize [" << endl;
		avatarSize->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}
