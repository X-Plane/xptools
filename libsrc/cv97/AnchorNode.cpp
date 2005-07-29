/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	AnchorNode.cpp
*
******************************************************************/

#include "AnchorNode.h"

AnchorNode::AnchorNode() 
{
	setHeaderFlag(false);
	setType(anchorNodeString);

	///////////////////////////
	// Exposed Field 
	///////////////////////////

	// description exposed field
	descriptionField = new SFString("");
	addExposedField(descriptionFieldString, descriptionField);

	// parameter exposed field
	parameterField = new MFString();
	addExposedField(parameterFieldString, parameterField);

	// url exposed field
	urlField = new MFString();
	addExposedField(urlFieldString, urlField);
}

AnchorNode::~AnchorNode() 
{
}

////////////////////////////////////////////////
//	Description
////////////////////////////////////////////////

SFString *AnchorNode::getDescriptionField() 
{
	if (isInstanceNode() == false)
		return descriptionField;
	return (SFString *)getExposedField(descriptionFieldString);
}

void AnchorNode::setDescription(char *value) 
{
	getDescriptionField()->setValue(value);
}

char *AnchorNode::getDescription() 
{
	return getDescriptionField()->getValue();
}

////////////////////////////////////////////////
// Parameter
////////////////////////////////////////////////

MFString *AnchorNode::getParameterField() 
{
	if (isInstanceNode() == false)
		return parameterField;
	return (MFString *)getExposedField(parameterFieldString);
}

void AnchorNode::addParameter(char *value) 
{
	getParameterField()->addValue(value);
}

int AnchorNode::getNParameters() 
{
	return getParameterField()->getSize();
}

char *AnchorNode::getParameter(int index) 
{
	return getParameterField()->get1Value(index);
}

////////////////////////////////////////////////
// Url
////////////////////////////////////////////////

MFString *AnchorNode::getUrlField() 
{
	if (isInstanceNode() == false)
		return urlField;
	return (MFString *)getExposedField(urlFieldString);
}

void AnchorNode::addUrl(char *value) 
{
	getUrlField()->addValue(value);
}

int AnchorNode::getNUrls() 
{
	return getUrlField()->getSize();
}

char *AnchorNode::getUrl(int index) 
{
	return getUrlField()->get1Value(index);
}

void AnchorNode::setUrl(int index, char *urlString) 
{
	getUrlField()->set1Value(index, urlString);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

AnchorNode *AnchorNode::next() 
{
	return (AnchorNode *)Node::next(getType());
}

AnchorNode *AnchorNode::nextTraversal() 
{
	return (AnchorNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	virtual functions
////////////////////////////////////////////////

bool AnchorNode::isChildNodeType(Node *node)
{
	if (node->isCommonNode() || node->isBindableNode() ||node->isInterpolatorNode() || node->isSensorNode() || node->isGroupingNode() || node->isSpecialGroupNode())
		return true;
	else
		return false;
}

void AnchorNode::initialize() 
{
	recomputeBoundingBox();
}

void AnchorNode::uninitialize() {
}

void AnchorNode::update() 
{
}

void AnchorNode::outputContext(ostream &printStream, char *indentString) 
{
	SFString *description = getDescriptionField();
	printStream << indentString << "\t" << "description " << description << endl;
	
	if (0 < getNParameters()) {
		MFString *parameter = getParameterField();
		printStream << indentString << "\t" << "parameter ["  << endl;
		parameter->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}

	if (0 < getNUrls()) {
		MFString *url = getUrlField();
		printStream << indentString << "\t" << "url [" << endl;
		url->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}
