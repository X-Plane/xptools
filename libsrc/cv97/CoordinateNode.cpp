/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	CoordinateNode.cpp
*
******************************************************************/

#include "CoordinateNode.h"

CoordinateNode::CoordinateNode() 
{
	setHeaderFlag(false);
	setType(coordinateNodeString);

	// point exposed field
	pointField = new MFVec3f();
	pointField->setName(pointFieldString);
	addExposedField(pointField);
}

CoordinateNode::~CoordinateNode() {
}

////////////////////////////////////////////////
//	point 
////////////////////////////////////////////////

MFVec3f *CoordinateNode::getPointField()
{
	if (isInstanceNode() == false)
		return pointField;
	return 	(MFVec3f *)getExposedField(pointFieldString);
}

void CoordinateNode::addPoint(float point[]) 
{
	getPointField()->addValue(point);
}

void CoordinateNode::addPoint(float x, float y, float z) 
{
	getPointField()->addValue(x, y, z);
}

int CoordinateNode::getNPoints() 
{
	return getPointField()->getSize();
}

void CoordinateNode::getPoint(int index, float point[]) 
{
	getPointField()->get1Value(index, point);
}

void CoordinateNode::setPoint(int index, float point[]) 
{
	getPointField()->set1Value(index, point);
}

void CoordinateNode::setPoint(int index, float x, float y, float z) 
{
	getPointField()->set1Value(index, x, y, z);
}

void CoordinateNode::removePoint(int index) 
{
	getPointField()->remove(index);
}

void CoordinateNode::removeLastPoint() 
{
	getPointField()->removeLastObject();
}

void CoordinateNode::removeFirstPoint() 
{
	getPointField()->removeFirstObject();
}

void CoordinateNode::removeAllPoints() 
{
	getPointField()->clear();
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool CoordinateNode::isChildNodeType(Node *node)
{
	return false;
}

void CoordinateNode::initialize() 
{
}

void CoordinateNode::uninitialize() 
{
}

void CoordinateNode::update() 
{
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void CoordinateNode::outputContext(ostream &printStream, char *indentString) 
{
	if (0 < getNPoints()) {
		MFVec3f *point = getPointField();
		printStream <<  indentString << "\t" << "point ["  << endl;
		point->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

CoordinateNode *CoordinateNode::next() 
{
	return (CoordinateNode *)Node::next(getType());
}

CoordinateNode *CoordinateNode::nextTraversal() 
{
	return (CoordinateNode *)Node::nextTraversalByType(getType());
}
