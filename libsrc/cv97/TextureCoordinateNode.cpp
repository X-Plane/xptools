/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	TextureCoordinateNode.cpp
*
******************************************************************/

#include "TextureCoordinateNode.h"

TextureCoordinateNode::TextureCoordinateNode() 
{
	setHeaderFlag(false);
	setType(textureCoordinateNodeString);

	// point exposed field
	pointField = new MFVec2f();
	pointField->setName(pointFieldString);
	addExposedField(pointField);
}

TextureCoordinateNode::~TextureCoordinateNode() 
{
}

////////////////////////////////////////////////
//	point 
////////////////////////////////////////////////

MFVec2f *TextureCoordinateNode::getPointField()
{
	if (isInstanceNode() == false)
		return pointField;
	return (MFVec2f *)getExposedField(pointFieldString);
}

void TextureCoordinateNode::addPoint(float point[]) 
{
	getPointField()->addValue(point);
}

int TextureCoordinateNode::getNPoints() 
{
	return getPointField()->getSize();
}

void TextureCoordinateNode::getPoint(int index, float point[]) 
{
	getPointField()->get1Value(index, point);
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool TextureCoordinateNode::isChildNodeType(Node *node)
{
	return false;
}

void TextureCoordinateNode::initialize() 
{
}

void TextureCoordinateNode::uninitialize() 
{
}

void TextureCoordinateNode::update() 
{
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void TextureCoordinateNode::outputContext(ostream &printStream, char *indentString) 
{
	if (0 < getNPoints()) {
		MFVec2f *point = getPointField();
		printStream <<  indentString << "\t" << "point ["  << endl;
		point->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

TextureCoordinateNode *TextureCoordinateNode::next() 
{
	return (TextureCoordinateNode *)Node::next(getType());
}

TextureCoordinateNode *TextureCoordinateNode::nextTraversal() 
{
	return (TextureCoordinateNode *)Node::nextTraversalByType(getType());
}
