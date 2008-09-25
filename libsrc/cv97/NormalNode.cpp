/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	NormalNode.cpp
*
******************************************************************/

#include "NormalNode.h"

NormalNode::NormalNode ()
{
	setHeaderFlag(false);
	setType(normalNodeString);

	// vector exposed field
	vectorField = new MFVec3f();
	vectorField->setName(vectorFieldString);
	addExposedField(vectorField);
}

NormalNode::~NormalNode ()
{
}

////////////////////////////////////////////////
//	vector
////////////////////////////////////////////////

MFVec3f *NormalNode::getVectorField()
{
	if (isInstanceNode() == false)
		return vectorField;
	return (MFVec3f *)getExposedField(vectorFieldString);
}

void NormalNode::addVector(float value[])
{
	getVectorField()->addValue(value);
}

int NormalNode::getNVectors()
{
	return getVectorField()->getSize();
}

void NormalNode::getVector(int index, float value[])
{
	getVectorField()->get1Value(index, value);
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool NormalNode::isChildNodeType(Node *node)
{
	return false;
}

void NormalNode::initialize()
{
}

void NormalNode::uninitialize()
{
}

void NormalNode::update()
{
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void NormalNode::outputContext(ostream &printStream, char *indentString)
{
	if (0 < getNVectors()) {
		MFVec3f *vector = getVectorField();
		printStream <<  indentString << "\t" << "vector ["  << endl;
		vector->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

NormalNode *NormalNode::next()
{
	return (NormalNode *)Node::next(getType());
}

NormalNode *NormalNode::nextTraversal()
{
	return (NormalNode *)Node::nextTraversalByType(getType());
}

