/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	PixelTextureNode.cpp
*
******************************************************************/

#include "PixelTextureNode.h"

PixelTextureNode::PixelTextureNode()
{
	setHeaderFlag(false);
	setType(pixelTextureNodeString);

	///////////////////////////
	// Exposed Field
	///////////////////////////

	// image field
	imageField = new SFImage();
	addExposedField(imageFieldString, imageField);
}

PixelTextureNode::~PixelTextureNode()
{
}

////////////////////////////////////////////////
// Image
////////////////////////////////////////////////

SFImage *PixelTextureNode::getImageField()
{
	if (isInstanceNode() == false)
		return imageField;
	return (SFImage *)getExposedField(imageFieldString);
}

void PixelTextureNode::addImage(int value)
{
	getImageField()->addValue(value);
}

int PixelTextureNode::getNImages()
{
	return getImageField()->getSize();
}

int PixelTextureNode::getImage(int index)
{
	return getImageField()->get1Value(index);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

PixelTextureNode *PixelTextureNode::next()
{
	return (PixelTextureNode *)Node::next(getType());
}

PixelTextureNode *PixelTextureNode::nextTraversal()
{
	return (PixelTextureNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool PixelTextureNode::isChildNodeType(Node *node)
{
	return false;
}

void PixelTextureNode::initialize()
{
}

void PixelTextureNode::uninitialize()
{
}

void PixelTextureNode::update()
{
}

////////////////////////////////////////////////
//	Imagemation
////////////////////////////////////////////////

void PixelTextureNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *repeatS = getRepeatSField();
	SFBool *repeatT = getRepeatTField();
	if (0 < getNImages()) {
		SFImage *image = getImageField();
		printStream << indentString << "\t" << "image " << endl;
		image->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << endl;
	}

	printStream << indentString << "\t" << "repeatS " << repeatS << endl;
	printStream << indentString << "\t" << "repeatT " << repeatT << endl;
}
