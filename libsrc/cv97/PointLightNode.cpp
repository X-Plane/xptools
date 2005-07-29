/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	PointLightNode.h
*
******************************************************************/

#include "PointLightNode.h"

PointLightNode::PointLightNode() 
{
	setHeaderFlag(false);
	setType(pointLightNodeString);

	// ambient intensity exposed field
	ambientIntensityField = new SFFloat(0.0f);
	ambientIntensityField->setName(ambientIntensityFieldString);
	addExposedField(ambientIntensityField);

	// location exposed field
	locationField = new SFVec3f(0.0f, 0.0f, 0.0f);
	locationField->setName(locationFieldString);
	addExposedField(locationField);

	// radius exposed field
	radiusField = new SFFloat(100.0f);
	radiusField->setName(radiusFieldString);
	addExposedField(radiusField);

	// attenuation exposed field
	attenuationField = new SFVec3f(1.0f, 0.0f, 0.0f);
	attenuationField->setName(attenuationFieldString);
	addExposedField(attenuationField);
}

PointLightNode::~PointLightNode() 
{
}

////////////////////////////////////////////////
//	AmbientIntensity
////////////////////////////////////////////////

SFFloat *PointLightNode::getAmbientIntensityField()
{
	if (isInstanceNode() == false)
		return ambientIntensityField;
	return (SFFloat *)getExposedField(ambientIntensityFieldString);
}
	
void PointLightNode::setAmbientIntensity(float value) 
{
	getAmbientIntensityField()->setValue(value);
}

float PointLightNode::getAmbientIntensity() 
{
	return getAmbientIntensityField()->getValue();
}

////////////////////////////////////////////////
//	Location
////////////////////////////////////////////////

SFVec3f *PointLightNode::getLocationField()
{
	if (isInstanceNode() == false)
		return locationField;
	return (SFVec3f *)getExposedField(locationFieldString);
}

void PointLightNode::setLocation(float value[]) 
{
	getLocationField()->setValue(value);
}

void PointLightNode::setLocation(float x, float y, float z) 
{
	getLocationField()->setValue(x, y, z);
}

void PointLightNode::getLocation(float value[]) 
{
	getLocationField()->getValue(value);
}

////////////////////////////////////////////////
//	Radius
////////////////////////////////////////////////

SFFloat *PointLightNode::getRadiusField()
{
	if (isInstanceNode() == false)
		return radiusField;
	return (SFFloat *)getExposedField(radiusFieldString);
}
	
void PointLightNode::setRadius(float value) 
{
	getRadiusField()->setValue(value);
}

float PointLightNode::getRadius() 
{
	return getRadiusField()->getValue();
}

////////////////////////////////////////////////
//	Attenuation
////////////////////////////////////////////////

SFVec3f *PointLightNode::getAttenuationField()
{
	if (isInstanceNode() == false)
		return attenuationField;
	return (SFVec3f *)getExposedField(attenuationFieldString);
}

void PointLightNode::setAttenuation(float value[]) 
{
	getAttenuationField()->setValue(value);
}

void PointLightNode::setAttenuation(float x, float y, float z) 
{
	getAttenuationField()->setValue(x, y, z);
}

void PointLightNode::getAttenuation(float value[]) 
{
	getAttenuationField()->getValue(value);
}

////////////////////////////////////////////////
//	Diffuse Color
////////////////////////////////////////////////

void PointLightNode::getDiffuseColor(float value[]) 
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

void PointLightNode::getAmbientColor(float value[]) 
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

PointLightNode *PointLightNode::next() 
{
	return (PointLightNode *)Node::next(getType());
}

PointLightNode *PointLightNode::nextTraversal() 
{
	return (PointLightNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool PointLightNode::isChildNodeType(Node *node)
{
	return false;
}

void PointLightNode::initialize() 
{
}

void PointLightNode::uninitialize() 
{
}

void PointLightNode::update() 
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void PointLightNode::outputContext(ostream &printStream, char *indentString) 
{
	SFColor *color = getColorField();
	SFVec3f *attenuation = getAttenuationField();
	SFVec3f *location = getLocationField();
	SFBool *bon = getOnField();

	printStream << indentString << "\t" << "on " << bon  << endl;
	printStream << indentString << "\t" << "intensity " << getIntensity()  << endl;
	printStream << indentString << "\t" << "ambientIntensity " << getAmbientIntensity()  << endl;
	printStream << indentString << "\t" << "color " << color  << endl;
	printStream << indentString << "\t" << "location " << location  << endl;
	printStream << indentString << "\t" << "radius " << getRadius()  << endl;
	printStream << indentString << "\t" << "attenuation " << attenuation  << endl;
}
