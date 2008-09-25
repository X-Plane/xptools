/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	DirectionalLightNode.h
*
******************************************************************/

#include "DirectionalLightNode.h"

DirectionalLightNode::DirectionalLightNode()
{
	setType(directionalLightNodeString);

	// ambient intensity exposed field
	ambientIntensityField = new SFFloat(0.0f);
	ambientIntensityField->setName(ambientIntensityFieldString);
	addExposedField(ambientIntensityField);

	// direction exposed field
	directionField = new SFVec3f(0.0f, 0.0f, -1.0f);
	directionField->setName(directionFieldString);
	addExposedField(directionField);
}

DirectionalLightNode::~DirectionalLightNode()
{
}

////////////////////////////////////////////////
//	AmbientIntensity
////////////////////////////////////////////////

SFFloat *DirectionalLightNode::getAmbientIntensityField()
{
	if (isInstanceNode() == false)
		return ambientIntensityField;
	return (SFFloat *)getExposedField(ambientIntensityFieldString);
}

void DirectionalLightNode::setAmbientIntensity(float value)
{
	getAmbientIntensityField()->setValue(value);
}

float DirectionalLightNode::getAmbientIntensity()
{
	return getAmbientIntensityField()->getValue();
}

////////////////////////////////////////////////
//	Direction
////////////////////////////////////////////////

SFVec3f *DirectionalLightNode::getDirectionField()
{
	if (isInstanceNode() == false)
		return directionField;
	return (SFVec3f *)getExposedField(directionFieldString);
}

void DirectionalLightNode::setDirection(float value[])
{
	getDirectionField()->setValue(value);
}

void DirectionalLightNode::setDirection(float x, float y, float z)
{
	getDirectionField()->setValue(x, y, z);
}

void DirectionalLightNode::getDirection(float value[])
{
	getDirectionField()->getValue(value);
}

////////////////////////////////////////////////
//	Diffuse Color
////////////////////////////////////////////////

void DirectionalLightNode::getDiffuseColor(float value[])
{
	getColor(value);
	float	intensity = getIntensity();
	value[0] *= intensity;
	value[1] *= intensity;
	value[2] *= intensity;
}

////////////////////////////////////////////////
//	Ambient Color
////////////////////////////////////////////////

void DirectionalLightNode::getAmbientColor(float value[])
{
	getColor(value);
	float	intensity = getIntensity();
	float	ambientIntensity = getAmbientIntensity();
	value[0] *= intensity * ambientIntensity;
	value[1] *= intensity * ambientIntensity;
	value[2] *= intensity * ambientIntensity;
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

DirectionalLightNode *DirectionalLightNode::next()
{
	return (DirectionalLightNode *)Node::next(getType());
}

DirectionalLightNode *DirectionalLightNode::nextTraversal()
{
	return (DirectionalLightNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool DirectionalLightNode::isChildNodeType(Node *node)
{
	return false;
}

void DirectionalLightNode::initialize()
{
}

void DirectionalLightNode::uninitialize()
{
}

void DirectionalLightNode::update()
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void DirectionalLightNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *bon = getOnField();
	SFVec3f *direction = getDirectionField();
	SFColor *color = getColorField();

	printStream << indentString << "\t" << "on " << bon  << endl;
	printStream << indentString << "\t" << "intensity " << getIntensity()  << endl;
	printStream << indentString << "\t" << "ambientIntensity " << getAmbientIntensity()  << endl;
	printStream << indentString << "\t" << "color " << color  << endl;
	printStream << indentString << "\t" << "direction " << direction  << endl;
}
