/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	CollisionNode.cpp
*
******************************************************************/

#include "VRMLField.h"
#include "CollisionNode.h"

CollisionNode::CollisionNode()
{
	setHeaderFlag(false);
	setType(collisionNodeString);

	// collide exposed field
	collideField = new SFBool(true);
	addExposedField(collideFieldString, collideField);

	// collide event out
	collideTimeField = new SFTime(-1.0);
	addEventOut(collideTimeFieldString, collideTimeField);
}

CollisionNode::~CollisionNode()
{
}

////////////////////////////////////////////////
//	collide
////////////////////////////////////////////////

SFBool *CollisionNode::getCollideField()
{
	if (isInstanceNode() == false)
		return collideField;
	return (SFBool *)getExposedField(collideFieldString);
}

void CollisionNode::setCollide(bool  value)
{
	getCollideField()->setValue(value);
}

void CollisionNode::setCollide(int value)
{
	setCollide(value ? true : false);
}

bool CollisionNode::getCollide()
{
	return getCollideField()->getValue();
}

////////////////////////////////////////////////
//	collideTime
////////////////////////////////////////////////

SFTime *CollisionNode::getCollideTimeField()
{
	if (isInstanceNode() == false)
		return collideTimeField;
	return (SFTime *)getEventOut(collideTimeFieldString);
}

void CollisionNode::setCollideTime(double value)
{
	getCollideTimeField()->setValue(value);
}

double CollisionNode::getCollideTime()
{
	return getCollideTimeField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

CollisionNode *CollisionNode::next()
{
	return (CollisionNode *)Node::next(getType());
}

CollisionNode *CollisionNode::nextTraversal()
{
	return (CollisionNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool CollisionNode::isChildNodeType(Node *node)
{
	if (node->isCommonNode() || node->isBindableNode() ||node->isInterpolatorNode() || node->isSensorNode() || node->isGroupingNode() || node->isSpecialGroupNode())
		return true;
	else
		return false;
}

void CollisionNode::initialize()
{
	recomputeBoundingBox();
}

void CollisionNode::uninitialize()
{
}

void CollisionNode::update()
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void CollisionNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *collide = getCollideField();
	printStream << indentString << "\t" << "collide " << collide << endl;
}
