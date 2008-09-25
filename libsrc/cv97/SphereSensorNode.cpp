/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SphereSensorNode.cpp
*
******************************************************************/

#include "SphereSensorNode.h"

SphereSensorNode::SphereSensorNode()
{
	setHeaderFlag(false);
	setType(sphereSensorNodeString);

	// autoOffset exposed field
	autoOffsetField = new SFBool(true);
	addExposedField(autoOffsetFieldString, autoOffsetField);

	// offset exposed field
	offsetField = new SFRotation(0.0f, 0.0f, 1.0f, 0.0f);
	addExposedField(offsetFieldString, offsetField);

	// rotation eventOut field
	rotationField = new SFRotation(0.0f, 0.0f, 1.0f, 0.0f);
	addEventOut(rotationFieldString, rotationField);

	// trackPoint eventOut field
	trackPointField = new SFVec3f(0.0f, 0.0f, 0.0f);
	addEventOut(trackPointFieldString, trackPointField);
}

SphereSensorNode::~SphereSensorNode()
{
}

////////////////////////////////////////////////
//	AutoOffset
////////////////////////////////////////////////

SFBool *SphereSensorNode::getAutoOffsetField()
{
	if (isInstanceNode() == false)
		return autoOffsetField;
	return (SFBool *)getExposedField(autoOffsetFieldString);
}

void SphereSensorNode::setAutoOffset(bool value)
{
	getAutoOffsetField()->setValue(value);
}

void SphereSensorNode::setAutoOffset(int value)
{
	setAutoOffset(value ? true : false);
}

bool SphereSensorNode::getAutoOffset()
{
	return getAutoOffsetField()->getValue();
}

bool  SphereSensorNode::isAutoOffset()
{
	return getAutoOffset();
}

////////////////////////////////////////////////
//	Offset
////////////////////////////////////////////////

SFRotation *SphereSensorNode::getOffsetField()
{
	if (isInstanceNode() == false)
		return offsetField;
	return (SFRotation *)getExposedField(offsetFieldString);
}

void SphereSensorNode::setOffset(float value[])
{
	getOffsetField()->setValue(value);
}

void SphereSensorNode::getOffset(float value[])
{
	getOffsetField()->getValue(value);
}

////////////////////////////////////////////////
//	Rotation
////////////////////////////////////////////////

SFRotation *SphereSensorNode::getRotationChangedField()
{
	if (isInstanceNode() == false)
		return rotationField;
	return (SFRotation *)getEventOut(rotationFieldString);
}

void SphereSensorNode::setRotationChanged(float value[])
{
	getRotationChangedField()->setValue(value);
}

void SphereSensorNode::setRotationChanged(float x, float y, float z, float rot)
{
	getRotationChangedField()->setValue(x, y, z, rot);
}

void SphereSensorNode::getRotationChanged(float value[])
{
	getRotationChangedField()->getValue(value);
}

////////////////////////////////////////////////
//	TrackPoint
////////////////////////////////////////////////

SFVec3f *SphereSensorNode::getTrackPointChangedField()
{
	if (isInstanceNode() == false)
		return trackPointField;
	return (SFVec3f *)getEventOut(trackPointFieldString);
}

void SphereSensorNode::setTrackPointChanged(float value[])
{
	getTrackPointChangedField()->setValue(value);
}

void SphereSensorNode::setTrackPointChanged(float x, float y, float z)
{
	getTrackPointChangedField()->setValue(x, y, z);
}

void SphereSensorNode::getTrackPointChanged(float value[])
{
	getTrackPointChangedField()->getValue(value);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

SphereSensorNode *SphereSensorNode::next()
{
	return (SphereSensorNode *)Node::next(getType());
}

SphereSensorNode *SphereSensorNode::nextTraversal()
{
	return (SphereSensorNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool SphereSensorNode::isChildNodeType(Node *node)
{
	return false;
}

void SphereSensorNode::initialize()
{
	setIsActive(false);
}

void SphereSensorNode::uninitialize()
{
}

void SphereSensorNode::update()
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void SphereSensorNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *autoOffset = getAutoOffsetField();
	SFBool *enabled = getEnabledField();
	SFRotation *offset = getOffsetField();

	printStream << indentString << "\t" << "autoOffset " << autoOffset << endl;
	printStream << indentString << "\t" << "enabled " << enabled << endl;
	printStream << indentString << "\t" << "offset " << offset << endl;
}
