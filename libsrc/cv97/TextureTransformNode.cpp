/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Node.cpp
*
******************************************************************/

#include "TextureTransformNode.h"

TextureTransformNode::TextureTransformNode()
{
	setHeaderFlag(false);
	setType(textureTransformNodeString);

	// translation exposed field
	translationField = new SFVec2f(0.0f, 0.0f);
	translationField->setName(translationFieldString);
	addExposedField(translationField);

	// scale exposed field
	scaleField = new SFVec2f(1.0f, 1.0f);
	scaleField->setName(scaleFieldString);
	addExposedField(scaleField);

	// center exposed field
	centerField = new SFVec2f(0.0f, 0.0f);
	centerField->setName(centerFieldString);
	addExposedField(centerField);

	// rotation exposed field
	rotationField = new SFFloat(0.0f);
	rotationField->setName(rotationFieldString);
	addExposedField(rotationField);
}

TextureTransformNode::~TextureTransformNode()
{
}

////////////////////////////////////////////////
//	Translation
////////////////////////////////////////////////

SFVec2f *TextureTransformNode::getTranslationField()
{
	if (isInstanceNode() == false)
		return translationField;
	return (SFVec2f *)getExposedField(translationFieldString);
}

void TextureTransformNode::setTranslation(float value[])
{
	getTranslationField()->setValue(value);
}

void TextureTransformNode::setTranslation(float x, float y)
{
	getTranslationField()->setValue(x, y);
}

void TextureTransformNode::getTranslation(float value[])
{
	getTranslationField()->getValue(value);
}

////////////////////////////////////////////////
//	Scale
////////////////////////////////////////////////

SFVec2f *TextureTransformNode::getScaleField()
{
	if (isInstanceNode() == false)
		return scaleField;
	return (SFVec2f *)getExposedField(scaleFieldString);
}

void TextureTransformNode::setScale(float value[])
{
	getScaleField()->setValue(value);
}

void TextureTransformNode::setScale(float x, float y)
{
	getScaleField()->setValue(x, y);
}

void TextureTransformNode::getScale(float value[])
{
	getScaleField()->getValue(value);
}

////////////////////////////////////////////////
//	Center
////////////////////////////////////////////////

SFVec2f *TextureTransformNode::getCenterField()
{
	if (isInstanceNode() == false)
		return centerField;
	return (SFVec2f *)getExposedField(centerFieldString);
}

void TextureTransformNode::setCenter(float value[])
{
	getCenterField()->setValue(value);
}

void TextureTransformNode::setCenter(float x, float y)
{
	getCenterField()->setValue(x, y);
}

void TextureTransformNode::getCenter(float value[])
{
	getCenterField()->getValue(value);
}

////////////////////////////////////////////////
//	Rotation
////////////////////////////////////////////////

SFFloat *TextureTransformNode::getRotationField()
{
	if (isInstanceNode() == false)
		return rotationField;
	return (SFFloat *)getExposedField(rotationFieldString);
}

void TextureTransformNode::setRotation(float value)
{
	getRotationField()->setValue(value);
}

float TextureTransformNode::getRotation()
{
	return getRotationField()->getValue();
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

TextureTransformNode *TextureTransformNode::next()
{
	return (TextureTransformNode *)Node::next(getType());
}

TextureTransformNode *TextureTransformNode::nextTraversal()
{
	return (TextureTransformNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool TextureTransformNode::isChildNodeType(Node *node)
{
	return false;
}

void TextureTransformNode::initialize()
{
}

void TextureTransformNode::uninitialize()
{
}

void TextureTransformNode::update()
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void TextureTransformNode::outputContext(ostream &printStream, char *indentString)
{
	SFVec2f *translation = getTranslationField();
	SFVec2f *center = getCenterField();
	SFVec2f *scale = getScaleField();
	printStream << indentString  <<  "\t"  <<  "translation " << translation << endl;
	printStream << indentString  <<  "\t"  <<  "rotation "  << getRotation() << endl;
	printStream << indentString  <<  "\t"  <<  "scale "  << scale << endl;
	printStream << indentString  <<  "\t"  <<  "center "  << center << endl;
}

////////////////////////////////////////////////
//	Node::getTransformMatrix(SFMatrix *matrix)
////////////////////////////////////////////////

void TextureTransformNode::getSFMatrix(SFMatrix *mOut)
{
	float		center[3];
	float		rotation[4];
	float		scale[3];
	float		translation[3];

	SFMatrix	mSRI;
	SFMatrix	mSR;
	SFMatrix	mCI;
	SFMatrix	mC;
	SFMatrix	mT;
	SFMatrix	mR;
	SFMatrix	mS;

	getTranslation(translation);
	translation[2] = 0.0f;
	mT.setTranslation(translation);

	getCenter(center);
	center[2] = 0.0f;
	mC.setTranslation(center);

	rotation[0] = 0.0f;
	rotation[1] = 0.0f;
	rotation[2] = 1.0f;
	rotation[3] = getRotation();
	mR.setRotation(rotation);

	getScale(scale);
	scale[2] = 1.0f;
	mS.setScaling(scale);

	getCenter(center);
	center[0] = -center[0];
	center[1] = -center[1];
	center[2] = 0.0f;
	mCI.setTranslation(center);

	mOut->init();
	mOut->add(&mCI);
	mOut->add(&mS);
	mOut->add(&mR);
	mOut->add(&mC);
	mOut->add(&mT);
}
