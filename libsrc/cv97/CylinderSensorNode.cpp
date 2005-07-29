/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	CylinderSensorNode.cpp
*
******************************************************************/

#include "CylinderSensorNode.h"

CylinderSensorNode::CylinderSensorNode() 
{
	setHeaderFlag(false);
	setType(cylinderSensorNodeString);

	// autoOffset exposed field
	autoOffsetField = new SFBool(true);
	addExposedField(autoOffsetFieldString, autoOffsetField);

	// diskAngle exposed field
	diskAngleField = new SFFloat(0.262f);
	addExposedField(diskAngleFieldString, diskAngleField);

	// minAngle exposed field
	minAngleField = new SFFloat(0.0f);
	addExposedField(minAngleFieldString, minAngleField);

	// maxAngle exposed field
	maxAngleField = new SFFloat(-1.0f);
	addExposedField(maxAngleFieldString, maxAngleField);

	// offset exposed field
	offsetField = new SFFloat(0.0f);
	addExposedField(offsetFieldString, offsetField);

	// rotation eventOut field
	rotationField = new SFRotation(0.0f, 0.0f, 1.0f, 0.0f);
	addEventOut(rotationFieldString, rotationField);

	// trackPoint eventOut field
	trackPointField = new SFVec3f(0.0f, 0.0f, 0.0f);
	addEventOut(trackPointFieldString, trackPointField);
}

CylinderSensorNode::~CylinderSensorNode() 
{
}

////////////////////////////////////////////////
//	AutoOffset
////////////////////////////////////////////////

SFBool *CylinderSensorNode::getAutoOffsetField()
{
	if (isInstanceNode() == false)
		return autoOffsetField;
	return (SFBool *)getExposedField(autoOffsetFieldString);
}
	
void CylinderSensorNode::setAutoOffset(bool  value) 
{
	getAutoOffsetField()->setValue(value);
}

void CylinderSensorNode::setAutoOffset(int value) 
{
	setAutoOffset(value ? true : false);
}

bool  CylinderSensorNode::getAutoOffset()
{
	return getAutoOffsetField()->getValue();
}

bool  CylinderSensorNode::isAutoOffset() 
{
	return getAutoOffset();
}

////////////////////////////////////////////////
//	DiskAngle
////////////////////////////////////////////////

SFFloat *CylinderSensorNode::getDiskAngleField()
{
	if (isInstanceNode() == false)
		return diskAngleField;
	return (SFFloat *)getExposedField(diskAngleFieldString);
}
	
void CylinderSensorNode::setDiskAngle(float value)
{
	getDiskAngleField()->setValue(value);
}

float CylinderSensorNode::getDiskAngle() 
{
	return getDiskAngleField()->getValue();
}

////////////////////////////////////////////////
//	MinAngle
////////////////////////////////////////////////

SFFloat *CylinderSensorNode::getMinAngleField()
{
	if (isInstanceNode() == false)
		return minAngleField;
	return (SFFloat *)getExposedField(minAngleFieldString);
}
	
void CylinderSensorNode::setMinAngle(float value) 
{
	getMinAngleField()->setValue(value);
}

float CylinderSensorNode::getMinAngle() 
{
	return getMinAngleField()->getValue();
}

////////////////////////////////////////////////
//	MaxAngle
////////////////////////////////////////////////

SFFloat *CylinderSensorNode::getMaxAngleField()
{
	if (isInstanceNode() == false)
		return maxAngleField;
	return (SFFloat *)getExposedField(maxAngleFieldString);
}
	
void CylinderSensorNode::setMaxAngle(float value) 
{
	getMaxAngleField()->setValue(value);
}

float CylinderSensorNode::getMaxAngle() 
{
	return getMaxAngleField()->getValue();
}

////////////////////////////////////////////////
//	Offset
////////////////////////////////////////////////

SFFloat *CylinderSensorNode::getOffsetField()
{
	if (isInstanceNode() == false)
		return offsetField;
	return (SFFloat *)getExposedField(offsetFieldString);
}
	
void CylinderSensorNode::setOffset(float value) 
{
	getOffsetField()->setValue(value);
}

float CylinderSensorNode::getOffset() 
{
	return getOffsetField()->getValue();
}

////////////////////////////////////////////////
//	Rotation
////////////////////////////////////////////////

SFRotation *CylinderSensorNode::getRotationChangedField()
{
	if (isInstanceNode() == false)
		return rotationField;
	return (SFRotation *)getEventOut(rotationFieldString);
}
	
void CylinderSensorNode::setRotationChanged(float value[]) 
{
	getRotationChangedField()->setValue(value);
}

void CylinderSensorNode::setRotationChanged(float x, float y, float z, float rot) 
{
	getRotationChangedField()->setValue(x, y, z, rot);
}

void CylinderSensorNode::getRotationChanged(float value[]) 
{
	getRotationChangedField()->getValue(value);
}

////////////////////////////////////////////////
//	TrackPoint
////////////////////////////////////////////////

SFVec3f *CylinderSensorNode::getTrackPointChangedField()
{
	if (isInstanceNode() == false)
		return trackPointField;
	return (SFVec3f *)getEventOut(trackPointFieldString);
}
	
void CylinderSensorNode::setTrackPointChanged(float value[]) 
{
	getTrackPointChangedField()->setValue(value);
}

void CylinderSensorNode::setTrackPointChanged(float x, float y, float z) 
{
	getTrackPointChangedField()->setValue(x, y, z);
}

void CylinderSensorNode::getTrackPointChanged(float value[]) 
{
	getTrackPointChangedField()->getValue(value);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

CylinderSensorNode *CylinderSensorNode::next() 
{
	return (CylinderSensorNode *)Node::next(getType());
}

CylinderSensorNode *CylinderSensorNode::nextTraversal() 
{
	return (CylinderSensorNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool CylinderSensorNode::isChildNodeType(Node *node)
{
	return false;
}

void CylinderSensorNode::initialize() 
{
	setIsActive(false);
}

void CylinderSensorNode::uninitialize()
{
}

void CylinderSensorNode::update() 
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void CylinderSensorNode::outputContext(ostream &printStream, char *indentString) 
{
	SFBool *autoOffset = getAutoOffsetField();
	SFBool *enabled = getEnabledField();

	printStream << indentString << "\t" << "autoOffset " << autoOffset << endl;
	printStream << indentString << "\t" << "diskAngle " << getDiskAngle() << endl;
	printStream << indentString << "\t" << "enabled " << enabled << endl;
	printStream << indentString << "\t" << "maxAngle " << getMaxAngle() << endl;
	printStream << indentString << "\t" << "minAngle " << getMinAngle() << endl;
	printStream << indentString << "\t" << "offset " << getOffset() << endl;
}
