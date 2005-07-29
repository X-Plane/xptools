/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	BackgroundNode.cpp
*
******************************************************************/

#include "BackgroundNode.h"

BackgroundNode::BackgroundNode()
{
	setHeaderFlag(false);
	setType(backgroundNodeString);

	// groundColor exposed field
	groundColorField = new MFColor();
	addExposedField(groundColorFieldString, groundColorField);

	// skyColor exposed field
	skyColorField = new MFColor();
	addExposedField(skyColorFieldString, skyColorField);

	// groundAngle exposed field
	groundAngleField = new MFFloat();
	addExposedField(groundAngleFieldString, groundAngleField);

	// skyAngle exposed field
	skyAngleField = new MFFloat();
	addExposedField(skyAngleFieldString, skyAngleField);

	// url exposed field
	frontUrlField = new MFString();
	addExposedField(frontUrlFieldString, frontUrlField);

	// url exposed field
	backUrlField = new MFString();
	addExposedField(backUrlFieldString, backUrlField);

	// url exposed field
	leftUrlField = new MFString();
	addExposedField(leftUrlFieldString, leftUrlField);

	// url exposed field
	rightUrlField = new MFString();
	addExposedField(rightUrlFieldString, rightUrlField);

	// url exposed field
	topUrlField = new MFString();
	addExposedField(topUrlFieldString, topUrlField);

	// url exposed field
	bottomUrlField = new MFString();
	addExposedField(bottomUrlFieldString, bottomUrlField);
}

BackgroundNode::~BackgroundNode()
{
}

////////////////////////////////////////////////
// groundColor
////////////////////////////////////////////////

MFColor *BackgroundNode::getGroundColorField()
{
	if (isInstanceNode() == false)
		return groundColorField;
	return (MFColor *)getExposedField(groundColorFieldString);
}

void BackgroundNode::addGroundColor(float value[])
{
	getGroundColorField()->addValue(value);
}

int BackgroundNode::getNGroundColors()
{
	return getGroundColorField()->getSize();
}

void BackgroundNode::getGroundColor(int index, float value[])
{
	getGroundColorField()->get1Value(index, value);
}

////////////////////////////////////////////////
// skyColor
////////////////////////////////////////////////

MFColor *BackgroundNode::getSkyColorField()
{
	if (isInstanceNode() == false)
		return skyColorField;
	return (MFColor *)getExposedField(skyColorFieldString);
}

void BackgroundNode::addSkyColor(float value[])
{
	getSkyColorField()->addValue(value);
}

int BackgroundNode::getNSkyColors()
{
	return getSkyColorField()->getSize();
}

void BackgroundNode::getSkyColor(int index, float value[])
{
	getSkyColorField()->get1Value(index, value);
}

////////////////////////////////////////////////
// groundAngle
////////////////////////////////////////////////

MFFloat *BackgroundNode::getGroundAngleField()
{
	if (isInstanceNode() == false)
		return groundAngleField;
	return (MFFloat *)getExposedField(groundAngleFieldString);
}

void BackgroundNode::addGroundAngle(float value)
{
	getGroundAngleField()->addValue(value);
}

int BackgroundNode::getNGroundAngles()
{
	return getGroundAngleField()->getSize();
}

float BackgroundNode::getGroundAngle(int index)
{
	return getGroundAngleField()->get1Value(index);
}

////////////////////////////////////////////////
// skyAngle
////////////////////////////////////////////////

MFFloat *BackgroundNode::getSkyAngleField()
{
	if (isInstanceNode() == false)
		return skyAngleField;
	return (MFFloat *)getExposedField(skyAngleFieldString);
}

void BackgroundNode::addSkyAngle(float value)
{
	getSkyAngleField()->addValue(value);
}

int BackgroundNode::getNSkyAngles()
{
	return getSkyAngleField()->getSize();
}

float BackgroundNode::getSkyAngle(int index)
{
	return getSkyAngleField()->get1Value(index);
}

////////////////////////////////////////////////
// frontUrl
////////////////////////////////////////////////

MFString *BackgroundNode::getFrontUrlField()
{
	if (isInstanceNode() == false)
		return frontUrlField;
	return (MFString *)getExposedField(frontUrlFieldString);
}

void BackgroundNode::addFrontUrl(char * value)
{
	getFrontUrlField()->addValue(value);
}

int BackgroundNode::getNFrontUrls()
{
	return getFrontUrlField()->getSize();
}

char * BackgroundNode::getFrontUrl(int index)
{
	return getFrontUrlField()->get1Value(index);
}

////////////////////////////////////////////////
// backUrl
////////////////////////////////////////////////

MFString *BackgroundNode::getBackUrlField()
{
	if (isInstanceNode() == false)
		return backUrlField;
	return (MFString *)getExposedField(backUrlFieldString);
}

void BackgroundNode::addBackUrl(char * value)
{
	getBackUrlField()->addValue(value);
}

int BackgroundNode::getNBackUrls()
{
	return getBackUrlField()->getSize();
}

char * BackgroundNode::getBackUrl(int index)
{
	return getBackUrlField()->get1Value(index);
}

////////////////////////////////////////////////
// leftUrl
////////////////////////////////////////////////

MFString *BackgroundNode::getLeftUrlField()
{
	if (isInstanceNode() == false)
		return leftUrlField;
	return (MFString *)getExposedField(leftUrlFieldString);
}

void BackgroundNode::addLeftUrl(char * value)
{
	getLeftUrlField()->addValue(value);
}

int BackgroundNode::getNLeftUrls()
{
	return getLeftUrlField()->getSize();
}

char * BackgroundNode::getLeftUrl(int index)
{
	return getLeftUrlField()->get1Value(index);
}

////////////////////////////////////////////////
// rightUrl
////////////////////////////////////////////////

MFString *BackgroundNode::getRightUrlField()
{
	if (isInstanceNode() == false)
		return rightUrlField;
	return (MFString *)getExposedField(rightUrlFieldString);
}

void BackgroundNode::addRightUrl(char * value)
{
	getRightUrlField()->addValue(value);
}

int BackgroundNode::getNRightUrls()
{
	return getRightUrlField()->getSize();
}

char * BackgroundNode::getRightUrl(int index)
{
	return getRightUrlField()->get1Value(index);
}

////////////////////////////////////////////////
// topUrl
////////////////////////////////////////////////

MFString *BackgroundNode::getTopUrlField()
{
	if (isInstanceNode() == false)
		return topUrlField;
	return (MFString *)getExposedField(topUrlFieldString);
}

void BackgroundNode::addTopUrl(char * value)
{
	getTopUrlField()->addValue(value);
}

int BackgroundNode::getNTopUrls()
{
	return getTopUrlField()->getSize();
}

char * BackgroundNode::getTopUrl(int index)
{
	return getTopUrlField()->get1Value(index);
}

////////////////////////////////////////////////
// bottomUrl
////////////////////////////////////////////////

MFString *BackgroundNode::getBottomUrlField()
{
	if (isInstanceNode() == false)
		return bottomUrlField;
	return (MFString *)getExposedField(bottomUrlFieldString);
}

void BackgroundNode::addBottomUrl(char * value)
{
	getBottomUrlField()->addValue(value);
}

int BackgroundNode::getNBottomUrls()
{
	return getBottomUrlField()->getSize();
}

char * BackgroundNode::getBottomUrl(int index)
{
	return getBottomUrlField()->get1Value(index);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

BackgroundNode *BackgroundNode::next()
{
	return (BackgroundNode *)Node::next(getType());
}

BackgroundNode *BackgroundNode::nextTraversal()
{
	return (BackgroundNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	Virtual functions
////////////////////////////////////////////////
	
bool BackgroundNode::isChildNodeType(Node *node)
{
	return false;
}

void BackgroundNode::initialize()
{
}

void BackgroundNode::uninitialize()
{
}

void BackgroundNode::update()
{
}

void BackgroundNode::outputContext(ostream &printStream, char *indentString)
{
	if (0 < getNGroundColors()){
		MFColor *groundColor = getGroundColorField();
		printStream << indentString << "\t" << "groundColor [" << endl;
		groundColor->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNSkyColors()){
		MFColor *skyColor = getSkyColorField();
		printStream << indentString << "\t" << "skyColor [" << endl;
		skyColor->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNGroundAngles()){
		MFFloat *groundAngle = getGroundAngleField();
		printStream << indentString << "\t" << "groundAngle [" << endl;
		groundAngle->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNSkyAngles()){
		MFFloat *skyAngle = getSkyAngleField();
		printStream << indentString << "\t" << "skyAngle [" << endl;
		skyAngle->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNFrontUrls()){
		MFString *frontUrl = getFrontUrlField();
		printStream << indentString << "\t" << "frontUrl [" << endl;
		frontUrl->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNBackUrls()){
		MFString *backUrl = getBackUrlField();
		printStream << indentString << "\t" << "backUrl [" << endl;
		backUrl->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNLeftUrls()){
		MFString *leftUrl = getLeftUrlField();
		printStream << indentString << "\t" << "leftUrl [" << endl;
		leftUrl->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNRightUrls()){
		MFString *rightUrl = getRightUrlField();
		printStream << indentString << "\t" << "rightUrl [" << endl;
		rightUrl->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNTopUrls()){
		MFString *topUrl = getTopUrlField();
		printStream << indentString << "\t" << "topUrl [" << endl;
		topUrl->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNBottomUrls()){
		MFString *bottomUrl = getBottomUrlField();
		printStream << indentString << "\t" << "bottomUrl [" << endl;
		bottomUrl->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}
