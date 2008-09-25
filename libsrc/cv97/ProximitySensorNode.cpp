/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ProximitySensorNode.cpp
*
******************************************************************/

#include "SceneGraph.h"

ProximitySensorNode::ProximitySensorNode()
{
	setHeaderFlag(false);
	setType(proximitySensorNodeString);

	// center exposed field
	centerField = new SFVec3f(0.0f, 0.0f, 0.0f);
	addExposedField(centerFieldString, centerField);

	// size exposed field
	sizeField = new SFVec3f(0.0f, 0.0f, 0.0f);
	addExposedField(sizeFieldString, sizeField);

	// position eventOut field
	positionField = new SFVec3f(0.0f, 0.0f, 0.0f);
	addEventOut(positionFieldString, positionField);

	// orientation eventOut field
	orientationField = new SFRotation(0.0f, 0.0f, 1.0f, 0.0f);
	addEventOut(orientationFieldString, orientationField);

	// enterTime eventOut field
	enterTimeField = new SFTime(0.0f);
	addEventOut(enterTimeFieldString, enterTimeField);

	// exitTime eventOut field
	exitTimeField = new SFTime(0.0f);
	addEventOut(exitTimeFieldString, exitTimeField);

	// display list field
	inRegionField = new SFBool(false);
	inRegionField->setName(inRegionPrivateFieldString);
	addPrivateField(inRegionField);
}

ProximitySensorNode::~ProximitySensorNode()
{
}

////////////////////////////////////////////////
//	Center
////////////////////////////////////////////////

SFVec3f *ProximitySensorNode::getCenterField()
{
	if (isInstanceNode() == false)
		return centerField;
	return (SFVec3f *)getExposedField(centerFieldString);
}

void ProximitySensorNode::setCenter(float value[])
{
	getCenterField()->setValue(value);
}

void ProximitySensorNode::setCenter(float x, float y, float z)
{
	getCenterField()->setValue(x, y, z);
}

void ProximitySensorNode::getCenter(float value[])
{
	getCenterField()->getValue(value);
}

////////////////////////////////////////////////
//	Size
////////////////////////////////////////////////

SFVec3f *ProximitySensorNode::getSizeField()
{
	if (isInstanceNode() == false)
		return sizeField;
	return (SFVec3f *)getExposedField(sizeFieldString);
}

void ProximitySensorNode::setSize(float value[])
{
	getSizeField()->setValue(value);
}

void ProximitySensorNode::setSize(float x, float y, float z)
{
	getSizeField()->setValue(x, y, z);
}

void ProximitySensorNode::getSize(float value[])
{
	getSizeField()->getValue(value);
}

////////////////////////////////////////////////
//	Position
////////////////////////////////////////////////

SFVec3f *ProximitySensorNode::getPositionChangedField()
{
	if (isInstanceNode() == false)
		return positionField;
	return (SFVec3f *)getEventOut(positionFieldString);
}

void ProximitySensorNode::setPositionChanged(float value[])
{
	getPositionChangedField()->setValue(value);
}

void ProximitySensorNode::setPositionChanged(float x, float y, float z)
{
	getPositionChangedField()->setValue(x, y, z);
}

void ProximitySensorNode::getPositionChanged(float value[])
{
	getPositionChangedField()->getValue(value);
}

////////////////////////////////////////////////
//	Orientation
////////////////////////////////////////////////

SFRotation *ProximitySensorNode::getOrientationChangedField()
{
	if (isInstanceNode() == false)
		return orientationField;
	return (SFRotation *)getEventOut(orientationFieldString);
}

void ProximitySensorNode::setOrientationChanged(float value[])
{
	getOrientationChangedField()->setValue(value);
}

void ProximitySensorNode::setOrientationChanged(float x, float y, float z, float rot)
{
	getOrientationChangedField()->setValue(x, y, z, rot);
}

void ProximitySensorNode::getOrientationChanged(float value[])
{
	getOrientationChangedField()->getValue(value);
}

////////////////////////////////////////////////
//	EnterTime
////////////////////////////////////////////////

SFTime *ProximitySensorNode::getEnterTimeField()
{
	if (isInstanceNode() == false)
		return enterTimeField;
	return (SFTime *)getEventOut(enterTimeFieldString);
}

void ProximitySensorNode::setEnterTime(double value)
{
	getEnterTimeField()->setValue(value);
}

double ProximitySensorNode::getEnterTime()
{
	return getEnterTimeField()->getValue();
}

////////////////////////////////////////////////
//	ExitTime
////////////////////////////////////////////////

SFTime *ProximitySensorNode::getExitTimeField()
{
	if (isInstanceNode() == false)
		return exitTimeField;
	return (SFTime *)getEventOut(exitTimeFieldString);
}

void ProximitySensorNode::setExitTime(double value)
{
	getExitTimeField()->setValue(value);
}

double ProximitySensorNode::getExitTime()
{
	return getExitTimeField()->getValue();
}

////////////////////////////////////////////////
//	inRegion
////////////////////////////////////////////////

SFBool *ProximitySensorNode::getInRegionField()
{
	if (isInstanceNode() == false)
		return inRegionField;
	return (SFBool *)getPrivateField(inRegionPrivateFieldString);
}

void ProximitySensorNode::setInRegion(bool value)
{
	getInRegionField()->setValue(value);
}

bool ProximitySensorNode::inRegion()
{
	return getInRegionField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

ProximitySensorNode *ProximitySensorNode::next()
{
	return (ProximitySensorNode *)Node::next(getType());
}

ProximitySensorNode *ProximitySensorNode::nextTraversal()
{
	return (ProximitySensorNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool ProximitySensorNode::isChildNodeType(Node *node)
{
	return false;
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void ProximitySensorNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *enabled = getEnabledField();
	SFVec3f *center = getCenterField();
	SFVec3f *size = getSizeField();

	printStream << indentString << "\t" << "enabled " << enabled << endl;
	printStream << indentString << "\t" << "center " << center << endl;
	printStream << indentString << "\t" << "size " << size << endl;
}

////////////////////////////////////////////////
//	ProximitySensorNode::initialize
////////////////////////////////////////////////

void ProximitySensorNode::initialize()
{
	setInRegion(false);
}

////////////////////////////////////////////////
//	ProximitySensorNode::uninitialize
////////////////////////////////////////////////

void ProximitySensorNode::uninitialize()
{
}

////////////////////////////////////////////////
//	ProximitySensorNode::update
////////////////////////////////////////////////

static bool isRegion(float vpos[], float center[], float size[])
{
	for (int n=0; n<3; n++) {
		if (vpos[n] < center[n] - size[n]/2.0f)
			return false;
		if (center[n] + size[n]/2.0f < vpos[n])
			return false;
	}

	return true;
}

void ProximitySensorNode::update()
{
	if (!isEnabled())
		return;

	SceneGraph *sg = getSceneGraph();
	if (!sg)
		return;

	ViewpointNode *vpoint = sg->getViewpointNode();
	if (vpoint == NULL)
		vpoint = sg->getDefaultViewpointNode();

	float vpos[3];
	vpoint->getPosition(vpos);

	float center[3];
	getCenter(center);

	float size[3];
	getSize(size);

	if (inRegion() == false) {
		if (isRegion(vpos, center, size) == true) {
			setInRegion(true);
			double time = GetCurrentSystemTime();
			setEnterTime(time);
			sendEvent(getEventOut(enterTimeFieldString));
			setIsActive(true);
			sendEvent(getEventOut(isActiveFieldString));
		}
	}
	else {
		if (isRegion(vpos, center, size) == false) {
			setInRegion(false);
			double time = GetCurrentSystemTime();
			setExitTime(time);
			sendEvent(getEventOut(exitTimeFieldString));
			setIsActive(false);
			sendEvent(getEventOut(isActiveFieldString));
		}
	}
}

