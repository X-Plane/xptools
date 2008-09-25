/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Node.cpp
*
******************************************************************/

#include "TransformNode.h"

TransformNode::TransformNode()
{
	setHeaderFlag(false);
	setType(transformNodeString);

	// translation exposed field
	translationField = new SFVec3f(0.0f, 0.0f, 0.0f);
	translationField->setName(translationFieldString);
	addExposedField(translationField);

	// scale exposed field
	scaleField = new SFVec3f(1.0f, 1.0f, 1.0f);
	scaleField->setName(scaleFieldString);
	addExposedField(scaleField);

	// center exposed field
	centerField = new SFVec3f(0.0f, 0.0f, 0.0f);
	centerField->setName(centerFieldString);
	addExposedField(centerField);

	// rotation exposed field
	rotationField = new SFRotation(0.0f, 0.0f, 1.0f, 0.0f);
	rotationField->setName(rotationFieldString);
	addExposedField(rotationField);

	// scaleOrientation exposed field
	scaleOrientationField = new SFRotation(0.0f, 0.0f, 1.0f, 0.0f);
	scaleOrientationField->setName(scaleOrientationFieldString);
	addExposedField(scaleOrientationField);
}

TransformNode::~TransformNode()
{
}

////////////////////////////////////////////////
//	Translation
////////////////////////////////////////////////

SFVec3f *TransformNode::getTranslationField()
{
	if (isInstanceNode() == false)
		return translationField;
	return (SFVec3f *)getExposedField(translationFieldString);
}

void TransformNode::setTranslation(float value[])
{
	getTranslationField()->setValue(value);
}

void TransformNode::setTranslation(float x, float y, float z)
{
	getTranslationField()->setValue(x, y, z);
}

void TransformNode::getTranslation(float value[])
{
	getTranslationField()->getValue(value);
}

////////////////////////////////////////////////
//	Scale
////////////////////////////////////////////////

SFVec3f *TransformNode::getScaleField()
{
	if (isInstanceNode() == false)
		return scaleField;
	return (SFVec3f *)getExposedField(scaleFieldString);
}

void TransformNode::setScale(float value[])
{
	getScaleField()->setValue(value);
}

void TransformNode::setScale(float x, float y, float z)
{
	getScaleField()->setValue(x, y, z);
}

void TransformNode::getScale(float value[])
{
	getScaleField()->getValue(value);
}

////////////////////////////////////////////////
//	Center
////////////////////////////////////////////////

SFVec3f *TransformNode::getCenterField()
{
	if (isInstanceNode() == false)
		return centerField;
	return (SFVec3f *)getExposedField(centerFieldString);
}

void TransformNode::setCenter(float value[])
{
	getCenterField()->setValue(value);
}

void TransformNode::setCenter(float x, float y, float z)
{
	getCenterField()->setValue(x, y, z);
}

void TransformNode::getCenter(float value[])
{
	getCenterField()->getValue(value);
}

////////////////////////////////////////////////
//	Rotation
////////////////////////////////////////////////

SFRotation *TransformNode::getRotationField()
{
	if (isInstanceNode() == false)
		return rotationField;
	return (SFRotation *)getExposedField(rotationFieldString);
}

void TransformNode::setRotation(float value[])
{
	getRotationField()->setValue(value);
}

void TransformNode::setRotation(float x, float y, float z, float w)
{
	getRotationField()->setValue(x, y, z, w);
}

void TransformNode::getRotation(float value[])
{
	getRotationField()->getValue(value);
}

////////////////////////////////////////////////
//	ScaleOrientation
////////////////////////////////////////////////

SFRotation *TransformNode::getScaleOrientationField()
{
	if (isInstanceNode() == false)
		return scaleOrientationField;
	return (SFRotation *)getExposedField(scaleOrientationFieldString);
}

void TransformNode::setScaleOrientation(float value[])
{
	getScaleOrientationField()->setValue(value);
}

void TransformNode::setScaleOrientation(float x, float y, float z, float w)
{
	getScaleOrientationField()->setValue(x, y, z, w);
}

void TransformNode::getScaleOrientation(float value[])
{
	getScaleOrientationField()->getValue(value);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

TransformNode *TransformNode::next()
{
	return (TransformNode *)Node::next(getType());
}

TransformNode *TransformNode::nextTraversal()
{
	return (TransformNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool TransformNode::isChildNodeType(Node *node)
{
	if (node->isCommonNode() || node->isBindableNode() ||node->isInterpolatorNode() || node->isSensorNode() || node->isGroupingNode() || node->isSpecialGroupNode())
		return true;
	else
		return false;
}

void TransformNode::initialize()
{
	recomputeBoundingBox();
}

void TransformNode::uninitialize()
{
}

void TransformNode::update()
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void TransformNode::outputContext(ostream &printStream, char *indentString)
{
	float vec[3];
	float rot[4];
	getTranslation(vec);		printStream << indentString << "\t" << "translation " << vec[0] << " "<< vec[1] << " " << vec[2] << endl;
	getRotation(rot);			printStream << indentString << "\t" << "rotation " << rot[0] << " "<< rot[1] << " " << rot[2] << " " << rot[3] << endl;
	getScale(vec);				printStream << indentString << "\t" << "scale " << vec[0] << " "<< vec[1] << " " << vec[2] << endl;
	getScaleOrientation(rot);	printStream << indentString << "\t" << "scaleOrientation " << rot[0] << " "<< rot[1] << " " << rot[2] << " " << rot[3] << endl;
	getCenter(vec);				printStream << indentString << "\t" << "center " << vec[0] << " "<< vec[1] << " " << vec[2] << endl;
}

////////////////////////////////////////////////
//	Matrix
////////////////////////////////////////////////

void TransformNode::getSFMatrix(SFMatrix *mOut)
{
	float	center[3];
	float	rotation[4];
	float	scale[3];
	float	scaleOri[4];
	float	trans[3];
	SFMatrix	mSRI;
	SFMatrix	mSR;
	SFMatrix	mCI;
	SFMatrix	mC;
	SFMatrix	mT;
	SFMatrix	mR;
	SFMatrix	mS;

	getTranslation(trans);
	mT.setTranslation(trans);

	getCenter(center);
	mC.setTranslation(center);

	getRotation(rotation);
	mR.setRotation(rotation);

	getScaleOrientation(scaleOri);
	mSR.setRotation(scaleOri);

	getScale(scale);
	mS.setScaling(scale);

	getScaleOrientation(scaleOri);
	scaleOri[3] = -scaleOri[3];
	mSRI.setRotation(scaleOri);

	getCenter(center);
	center[0] = -center[0];
	center[1] = -center[1];
	center[2] = -center[2];
	mCI.setTranslation(center);

	mOut->init();
	mOut->add(&mT);
	mOut->add(&mC);
	mOut->add(&mR);
	mOut->add(&mSR);
	mOut->add(&mS);
	mOut->add(&mSRI);
	mOut->add(&mCI);
}
