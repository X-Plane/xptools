/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	VisibilitySensorNode.cpp
*
******************************************************************/

#include "VisibilitySensorNode.h"

VisibilitySensorNode::VisibilitySensorNode() 
{
	setHeaderFlag(false);
	setType(visibilitySensorNodeString);

	// center exposed field
	centerField = new SFVec3f(0.0f, 0.0f, 0.0f);
	addExposedField(centerFieldString, centerField);

	// size exposed field
	sizeField = new SFVec3f(0.0f, 0.0f, 0.0f);
	addExposedField(sizeFieldString, sizeField);

	// enterTime eventOut field
	enterTimeField = new SFTime(0.0f);
	addEventOut(enterTimeFieldString, enterTimeField);

	// exitTime eventOut field
	exitTimeField = new SFTime(0.0f);
	addEventOut(exitTimeFieldString, exitTimeField);
}

VisibilitySensorNode::~VisibilitySensorNode() 
{
}

////////////////////////////////////////////////
//	Center
////////////////////////////////////////////////

SFVec3f *VisibilitySensorNode::getCenterField()
{
	if (isInstanceNode() == false)
		return centerField;
	return (SFVec3f *)getExposedField(centerFieldString);
}
	
void VisibilitySensorNode::setCenter(float value[]) 
{
	getCenterField()->setValue(value);
}

void VisibilitySensorNode::setCenter(float x, float y, float z) 
{
	getCenterField()->setValue(x, y, z);
}

void VisibilitySensorNode::getCenter(float value[]) 
{
	getCenterField()->getValue();
}

////////////////////////////////////////////////
//	Size
////////////////////////////////////////////////

SFVec3f *VisibilitySensorNode::getSizeField()
{
	if (isInstanceNode() == false)
		return sizeField;
	return (SFVec3f *)getExposedField(sizeFieldString);
}
	
void VisibilitySensorNode::setSize(float value[]) 
{
	getSizeField()->setValue(value);
}

void VisibilitySensorNode::setSize(float x, float y, float z) 
{
	getSizeField()->setValue(x, y, z);
}

void VisibilitySensorNode::getSize(float value[]) 
{
	getSizeField()->getValue();
}

////////////////////////////////////////////////
//	EnterTime
////////////////////////////////////////////////

SFTime *VisibilitySensorNode::getEnterTimeField()
{
	if (isInstanceNode() == false)
		return enterTimeField;
	return (SFTime *)getEventOut(enterTimeFieldString);
}
	
void VisibilitySensorNode::setEnterTime(double value) 
{
	getEnterTimeField()->setValue(value);
}

double VisibilitySensorNode::getEnterTime() 
{
	return getEnterTimeField()->getValue();
}

////////////////////////////////////////////////
//	ExitTime
////////////////////////////////////////////////

SFTime *VisibilitySensorNode::getExitTimeField()
{
	if (isInstanceNode() == false)
		return exitTimeField;
	return (SFTime *)getEventOut(exitTimeFieldString);
}
	
void VisibilitySensorNode::setExitTime(double value) 
{
	getExitTimeField()->setValue(value);
}

double VisibilitySensorNode::getExitTime() 
{
	return getExitTimeField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

VisibilitySensorNode *VisibilitySensorNode::next() 
{
	return (VisibilitySensorNode *)Node::next(getType());
}

VisibilitySensorNode *VisibilitySensorNode::nextTraversal() 
{
	return (VisibilitySensorNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool VisibilitySensorNode::isChildNodeType(Node *node)
{
	return false;
}

void VisibilitySensorNode::initialize() 
{
}

void VisibilitySensorNode::uninitialize() 
{
}

void VisibilitySensorNode::update() 
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void VisibilitySensorNode::outputContext(ostream &printStream, char *indentString) 
{
	SFBool *enabled = getEnabledField();
	SFVec3f *center = getCenterField();
	SFVec3f *size = getSizeField();
	printStream << indentString << "\t" << "enabled " << enabled << endl;
	printStream << indentString << "\t" << "center " << center << endl;
	printStream << indentString << "\t" << "size " << size << endl;
}


