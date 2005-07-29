/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MaterialNode.h
*
******************************************************************/

#include "MaterialNode.h"

MaterialNode::MaterialNode() 
{
	setHeaderFlag(false);
	setType(materialNodeString);

	// tranparency exposed field
	transparencyField = new SFFloat(0.0f);
	transparencyField->setName(transparencyFieldString);
	addExposedField(transparencyField);

	// ambientIntesity exposed field
	ambientIntensityField = new SFFloat(0.2f);
	ambientIntensityField->setName(ambientIntensityFieldString);
	addExposedField(ambientIntensityField);

	// shininess exposed field
	shininessField = new SFFloat(0.2f);
	shininessField->setName(shininessFieldString);
	addExposedField(shininessField);

	// diffuseColor exposed field
	diffuseColorField = new SFColor(0.8f, 0.8f, 0.8f);
	diffuseColorField->setName(diffuseColorFieldString);
	addExposedField(diffuseColorField);

	// specularColor exposed field
	specularColorField = new SFColor(0.0f, 0.0f, 0.0f);
	specularColorField->setName(specularColorFieldString);
	addExposedField(specularColorField);

	// emissiveColor exposed field
	emissiveColorField = new SFColor(0.0f, 0.0f, 0.0f);
	emissiveColorField->setName(emissiveColorFieldString);
	addExposedField(emissiveColorField);
}

MaterialNode::~MaterialNode() 
{
}

////////////////////////////////////////////////
//	Transparency
////////////////////////////////////////////////

SFFloat *MaterialNode::getTransparencyField()
{
	if (isInstanceNode() == false)
		return transparencyField;
	return (SFFloat *)getExposedField(transparencyFieldString);
}
	
void MaterialNode::setTransparency(float value) 
{
	getTransparencyField()->setValue(value);
}

float MaterialNode::getTransparency() 
{
	return getTransparencyField()->getValue();
}

////////////////////////////////////////////////
//	AmbientIntensity
////////////////////////////////////////////////

SFFloat *MaterialNode::getAmbientIntensityField()
{
	if (isInstanceNode() == false)
		return ambientIntensityField;
	return (SFFloat *)getExposedField(ambientIntensityFieldString);
}
	
void MaterialNode::setAmbientIntensity(float intensity) 
{
	getAmbientIntensityField()->setValue(intensity);
}

float MaterialNode::getAmbientIntensity() 
{
	return getAmbientIntensityField()->getValue();
}

////////////////////////////////////////////////
//	Shininess
////////////////////////////////////////////////

SFFloat *MaterialNode::getShininessField()
{
	if (isInstanceNode() == false)
		return shininessField;
	return (SFFloat *)getExposedField(shininessFieldString);
}
	
void MaterialNode::setShininess(float value) 
{
	getShininessField()->setValue(value);
}

float MaterialNode::getShininess() 
{
	return getShininessField()->getValue();
}

////////////////////////////////////////////////
//	DiffuseColor
////////////////////////////////////////////////

SFColor *MaterialNode::getDiffuseColorField()
{
	if (isInstanceNode() == false)
		return diffuseColorField;
	return (SFColor *)getExposedField(diffuseColorFieldString);
}

void MaterialNode::setDiffuseColor(float value[]) 
{
	getDiffuseColorField()->setValue(value);
}

void MaterialNode::setDiffuseColor(float r, float g, float b) 
{
	getDiffuseColorField()->setValue(r, g, b);
}

void MaterialNode::getDiffuseColor(float value[]) 
{
	getDiffuseColorField()->getValue(value);
}

////////////////////////////////////////////////
//	SpecularColor
////////////////////////////////////////////////

SFColor *MaterialNode::getSpecularColorField()
{
	if (isInstanceNode() == false)
		return specularColorField;
	return (SFColor *)getExposedField(specularColorFieldString);
}

void MaterialNode::setSpecularColor(float value[]) 
{
	getSpecularColorField()->setValue(value);
}

void MaterialNode::setSpecularColor(float r, float g, float b) 
{
	getSpecularColorField()->setValue(r, g, b);
}

void MaterialNode::getSpecularColor(float value[]) 
{
	getSpecularColorField()->getValue(value);
}

////////////////////////////////////////////////
//	EmissiveColor
////////////////////////////////////////////////

SFColor *MaterialNode::getEmissiveColorField()
{
	if (isInstanceNode() == false)
		return emissiveColorField;
	return (SFColor *)getExposedField(emissiveColorFieldString);
}

void MaterialNode::setEmissiveColor(float value[]) 
{
	getEmissiveColorField()->setValue(value);
}

void MaterialNode::setEmissiveColor(float r, float g, float b) 
{
	getEmissiveColorField()->setValue(r, g, b);
}

void MaterialNode::getEmissiveColor(float value[]) 
{
	getEmissiveColorField()->getValue(value);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

MaterialNode *MaterialNode::next() 
{
	return (MaterialNode *)Node::next(getType());
}

MaterialNode *MaterialNode::nextTraversal() 
{
	return (MaterialNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool MaterialNode::isChildNodeType(Node *node)
{
	return false;
}

void MaterialNode::initialize() 
{
}

void MaterialNode::uninitialize() 
{
}

void MaterialNode::update() 
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void MaterialNode::outputContext(ostream &printStream, char *indentString) 
{
	SFColor *dcolor = getDiffuseColorField();
	SFColor *scolor = getSpecularColorField();
	SFColor *ecolor = getEmissiveColorField();
	printStream << indentString << "\t" << "diffuseColor " << dcolor << endl;
	printStream << indentString << "\t" << "ambientIntensity " << getAmbientIntensity() << endl;
	printStream << indentString << "\t" << "specularColor " << scolor << endl;
	printStream << indentString << "\t" << "emissiveColor " << ecolor << endl;
	printStream << indentString << "\t" << "shininess " << getShininess() << endl;
	printStream << indentString << "\t" << "transparency " << getTransparency() << endl;
}
