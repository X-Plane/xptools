/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	PlaneSensorNode.cpp
*
******************************************************************/

#include "PlaneSensorNode.h"

PlaneSensorNode::PlaneSensorNode()
{
	setHeaderFlag(false);
	setType(planeSensorNodeString);

	// autoOffset exposed field
	autoOffsetField = new SFBool(true);
	addExposedField(autoOffsetFieldString, autoOffsetField);

	// minPosition exposed field
	minPositionField = new SFVec2f(0.0f, 0.0f);
	addExposedField(minPositionFieldString, minPositionField);

	// maxAngle exposed field
	maxPositionField = new SFVec2f(-1.0f, -1.0f);
	addExposedField(maxPositionFieldString, maxPositionField);

	// offset exposed field
	offsetField = new SFVec3f(0.0f, 0.0f, 0.0f);
	addExposedField(offsetFieldString, offsetField);

	// translation eventOut field
	translationField = new SFVec3f(0.0f, 0.0f, 0.0f);
	addEventOut(translationFieldString, translationField);

	// trackPoint eventOut field
	trackPointField = new SFVec3f(0.0f, 0.0f, 0.0f);
	addEventOut(trackPointFieldString, trackPointField);
}

PlaneSensorNode::~PlaneSensorNode()
{
}

////////////////////////////////////////////////
//	AutoOffset
////////////////////////////////////////////////

SFBool *PlaneSensorNode::getAutoOffsetField()
{
	if (isInstanceNode() == false)
		return autoOffsetField;
	return (SFBool *)getExposedField(autoOffsetFieldString);
}

void PlaneSensorNode::setAutoOffset(bool value)
{
	getAutoOffsetField()->setValue(value);
}

void PlaneSensorNode::setAutoOffset(int value)
{
	setAutoOffset(value ? true : false);
}

bool PlaneSensorNode::getAutoOffset()
{
	return getAutoOffsetField()->getValue();
}

bool PlaneSensorNode::isAutoOffset()
{
	return getAutoOffset();
}

////////////////////////////////////////////////
//	MinPosition
////////////////////////////////////////////////

SFVec2f *PlaneSensorNode::getMinPositionField()
{
	if (isInstanceNode() == false)
		return minPositionField;
	return (SFVec2f *)getExposedField(minPositionFieldString);
}

void PlaneSensorNode::setMinPosition(float value[])
{
	getMinPositionField()->setValue(value);
}

void PlaneSensorNode::setMinPosition(float x, float y)
{
	getMinPositionField()->setValue(x, y);
}

void PlaneSensorNode::getMinPosition(float value[])
{
	getMinPositionField()->getValue(value);
}

void PlaneSensorNode::getMinPosition(float *x, float *y)
{
	SFVec2f *sfvec2f = getMinPositionField();
	*x = sfvec2f->getX();
	*y = sfvec2f->getY();
}

////////////////////////////////////////////////
//	MaxPosition
////////////////////////////////////////////////

SFVec2f *PlaneSensorNode::getMaxPositionField()
{
	if (isInstanceNode() == false)
		return maxPositionField;
	return (SFVec2f *)getExposedField(maxPositionFieldString);
}

void PlaneSensorNode::setMaxPosition(float value[])
{
	getMaxPositionField()->setValue(value);
}

void PlaneSensorNode::setMaxPosition(float x, float y)
{
	getMaxPositionField()->setValue(x, y);
}

void PlaneSensorNode::getMaxPosition(float value[])
{
	getMaxPositionField()->getValue(value);
}

void PlaneSensorNode::getMaxPosition(float *x, float *y)
{
	SFVec2f *sfvec2f = getMaxPositionField();
	*x = sfvec2f->getX();
	*y = sfvec2f->getY();
}

////////////////////////////////////////////////
//	Offset
////////////////////////////////////////////////

SFVec3f *PlaneSensorNode::getOffsetField()
{
	if (isInstanceNode() == false)
		return offsetField;
	return (SFVec3f *)getExposedField(offsetFieldString);
}

void PlaneSensorNode::setOffset(float value[])
{
	getOffsetField()->setValue(value);
}

void PlaneSensorNode::getOffset(float value[])
{
	getOffsetField()->getValue(value);
}

////////////////////////////////////////////////
//	Translation
////////////////////////////////////////////////

SFVec3f *PlaneSensorNode::getTranslationChangedField()
{
	if (isInstanceNode() == false)
		return translationField;
	return (SFVec3f *)getEventOut(translationFieldString);
}

void PlaneSensorNode::setTranslationChanged(float value[])
{
	getTranslationChangedField()->setValue(value);
}

void PlaneSensorNode::setTranslationChanged(float x, float y, float z)
{
	getTranslationChangedField()->setValue(x, y, z);
}

void PlaneSensorNode::getTranslationChanged(float value[])
{
	getTranslationChangedField()->getValue(value);
}

////////////////////////////////////////////////
//	TrackPoint
////////////////////////////////////////////////

SFVec3f *PlaneSensorNode::getTrackPointChangedField()
{
	if (isInstanceNode() == false)
		return trackPointField;
	return (SFVec3f *)getEventOut(trackPointFieldString);
}

void PlaneSensorNode::setTrackPointChanged(float value[])
{
	getTrackPointChangedField()->setValue(value);
}

void PlaneSensorNode::setTrackPointChanged(float x, float y, float z)
{
	getTrackPointChangedField()->setValue(x, y, z);
}

void PlaneSensorNode::getTrackPointChanged(float value[])
{
	getTrackPointChangedField()->getValue(value);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

PlaneSensorNode *PlaneSensorNode::next()
{
	return (PlaneSensorNode *)Node::next(getType());
}

PlaneSensorNode *PlaneSensorNode::nextTraversal()
{
	return (PlaneSensorNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool PlaneSensorNode::isChildNodeType(Node *node)
{
	return false;
}

void PlaneSensorNode::initialize()
{
	setIsActive(false);
}

void PlaneSensorNode::uninitialize()
{
}

void PlaneSensorNode::update()
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void PlaneSensorNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *autoOffset = getAutoOffsetField();
	SFBool *enabled = getEnabledField();
	SFVec2f *maxpos = getMaxPositionField();
	SFVec2f *minpos = getMinPositionField();
	SFVec3f *offset = getOffsetField();

	printStream << indentString << "\t" << "autoOffset " << autoOffset  << endl;
	printStream << indentString << "\t" << "enabled " << enabled  << endl;
	printStream << indentString << "\t" << "maxPosition " << maxpos  << endl;
	printStream << indentString << "\t" << "minPosition " << minpos  << endl;
	printStream << indentString << "\t" << "offset " << offset << endl;
}
