/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SpotLightNode.cpp
*
******************************************************************/

#include "SpotLightNode.h"

SpotLightNode::SpotLightNode()
{
	setType(spotLightNodeString);

	// ambient intensity exposed field
	ambientIntensityField = new SFFloat(0.0f);
	ambientIntensityField->setName(ambientIntensityFieldString);
	addExposedField(ambientIntensityField);

	// location exposed field
	locationField = new SFVec3f(0.0f, 0.0f, 0.0f);
	locationField->setName(locationFieldString);
	addExposedField(locationField);

	// direction exposed field
	directionField = new SFVec3f(0.0f, 0.0f, -1.0f);
	directionField->setName(directionFieldString);
	addExposedField(directionField);

	// radius exposed field
	radiusField = new SFFloat(100.0f);
	radiusField->setName(radiusFieldString);
	addExposedField(radiusField);

	// attenuation exposed field
	attenuationField = new SFVec3f(1.0f, 0.0f, 0.0f);
	attenuationField->setName(attenuationFieldString);
	addExposedField(attenuationField);

	// beamWidth exposed field
	beamWidthField = new SFFloat(1.570796f);
	beamWidthField->setName(beamWidthFieldString);
	addExposedField(beamWidthField);

	// cutOffAngle exposed field
	cutOffAngleField = new SFFloat(0.785398f);
	cutOffAngleField->setName(cutOffAngleFieldString);
	addExposedField(cutOffAngleField);
}

SpotLightNode::~SpotLightNode()
{
}

////////////////////////////////////////////////
//	AmbientIntensity
////////////////////////////////////////////////

SFFloat *SpotLightNode::getAmbientIntensityField()
{
	if (isInstanceNode() == false)
		return ambientIntensityField;
	return (SFFloat *)getExposedField(ambientIntensityFieldString);
}

void SpotLightNode::setAmbientIntensity(float value)
{
	getAmbientIntensityField()->setValue(value);
}

float SpotLightNode::getAmbientIntensity()
{
	return getAmbientIntensityField()->getValue();
}

////////////////////////////////////////////////
//	Location
////////////////////////////////////////////////

SFVec3f *SpotLightNode::getLocationField()
{
	if (isInstanceNode() == false)
		return locationField;
	return (SFVec3f *)getExposedField(locationFieldString);
}

void SpotLightNode::setLocation(float value[])
{
	getLocationField()->setValue(value);
}

void SpotLightNode::setLocation(float x, float y, float z)
{
	getLocationField()->setValue(x, y, z);
}

void SpotLightNode::getLocation(float value[])
{
	getLocationField()->getValue(value);
}

////////////////////////////////////////////////
//	Direction
////////////////////////////////////////////////

SFVec3f *SpotLightNode::getDirectionField()
{
	if (isInstanceNode() == false)
		return directionField;
	return (SFVec3f *)getExposedField(directionFieldString);
}

void SpotLightNode::setDirection(float value[])
{
	getDirectionField()->setValue(value);
}

void SpotLightNode::setDirection(float x, float y, float z)
{
	getDirectionField()->setValue(x, y, z);
}

void SpotLightNode::getDirection(float value[])
{
	getDirectionField()->getValue(value);
}

////////////////////////////////////////////////
//	Radius
////////////////////////////////////////////////

SFFloat *SpotLightNode::getRadiusField()
{
	if (isInstanceNode() == false)
		return radiusField;
	return (SFFloat *)getExposedField(radiusFieldString);
}

void SpotLightNode::setRadius(float value)
{
	getRadiusField()->setValue(value);
}

float SpotLightNode::getRadius()
{
	return getRadiusField()->getValue();
}

////////////////////////////////////////////////
//	Attenuation
////////////////////////////////////////////////

SFVec3f *SpotLightNode::getAttenuationField()
{
	if (isInstanceNode() == false)
		return attenuationField;
	return (SFVec3f *)getExposedField(attenuationFieldString);
}

void SpotLightNode::setAttenuation(float value[])
{
	getAttenuationField()->setValue(value);
}

void SpotLightNode::setAttenuation(float x, float y, float z)
{
	getAttenuationField()->setValue(x, y, z);
}

void SpotLightNode::getAttenuation(float value[])
{
	getAttenuationField()->getValue(value);
}

////////////////////////////////////////////////
//	BeamWidth
////////////////////////////////////////////////

SFFloat *SpotLightNode::getBeamWidthField()
{
	if (isInstanceNode() == false)
		return beamWidthField;
	return (SFFloat *)getExposedField(beamWidthFieldString);
}

void SpotLightNode::setBeamWidth(float value)
{
	getBeamWidthField()->setValue(value);
}

float SpotLightNode::getBeamWidth()
{
	return getBeamWidthField()->getValue();
}

////////////////////////////////////////////////
//	CutOffAngle
////////////////////////////////////////////////

SFFloat *SpotLightNode::getCutOffAngleField()
{
	if (isInstanceNode() == false)
		return cutOffAngleField;
	return (SFFloat *)getExposedField(cutOffAngleFieldString);
}

void SpotLightNode::setCutOffAngle(float value)
{
	getCutOffAngleField()->setValue(value);
}

float SpotLightNode::getCutOffAngle()
{
	return getCutOffAngleField()->getValue();
}

////////////////////////////////////////////////
//	Diffuse Color
////////////////////////////////////////////////

void SpotLightNode::getDiffuseColor(float value[])
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

void SpotLightNode::getAmbientColor(float value[])
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

SpotLightNode *SpotLightNode::next()
{
	return (SpotLightNode *)Node::next(getType());
}

SpotLightNode *SpotLightNode::nextTraversal()
{
	return (SpotLightNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool SpotLightNode::isChildNodeType(Node *node)
{
	return false;
}

void SpotLightNode::initialize()
{
}

void SpotLightNode::uninitialize()
{
}

void SpotLightNode::update()
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void SpotLightNode::outputContext(ostream &printStream, char *indentString)
{
	SFBool *bon = getOnField();
	SFColor *color = getColorField();
	SFVec3f *direction = getDirectionField();
	SFVec3f *location = getLocationField();
	SFVec3f *attenuation = getAttenuationField();

	printStream << indentString << "\t" << "on " << bon << endl;
	printStream << indentString << "\t" << "intensity " << getIntensity() << endl;
	printStream << indentString << "\t" << "ambientIntensity " << getAmbientIntensity() << endl;
	printStream << indentString << "\t" << "color " << color << endl;
	printStream << indentString << "\t" << "direction " << direction << endl;
	printStream << indentString << "\t" << "location " << location << endl;
	printStream << indentString << "\t" << "beamWidth " << getBeamWidth() << endl;
	printStream << indentString << "\t" << "cutOffAngle " << getCutOffAngle() << endl;
	printStream << indentString << "\t" << "radius " << getRadius() << endl;
	printStream << indentString << "\t" << "attenuation " << attenuation << endl;
}
