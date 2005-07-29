/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	AppearanceNode.cpp
*
******************************************************************/

#include "AppearanceNode.h"
#include "TextureNode.h"

AppearanceNode::AppearanceNode() 
{
	setHeaderFlag(false);
	setType(appearanceNodeString);
}

AppearanceNode::~AppearanceNode() 
{
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

AppearanceNode *AppearanceNode::next() 
{
	return (AppearanceNode *)Node::next(getType());
}

AppearanceNode *AppearanceNode::nextTraversal() 
{
	return (AppearanceNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	virtual functions
////////////////////////////////////////////////
	
bool AppearanceNode::isChildNodeType(Node *node)
{
	if (node->isMaterialNode() || node->isTextureNode() || node->isTextureTransformNode())
		return true;
	else
		return false;
}

void AppearanceNode::initialize() 
{
}

void AppearanceNode::uninitialize() 
{
}

void AppearanceNode::update() 
{
}

void AppearanceNode::outputContext(ostream &printStream, char *indentString) 
{
	MaterialNode *material = getMaterialNodes();
	if (material != NULL) {
		if (material->isInstanceNode() == false) {
			if (material->getName() != NULL && strlen(material->getName()))
				printStream << indentString << "\t" << "material " << "DEF " << material->getName() << " Material {" << endl;
			else
				printStream << indentString << "\t" << "material Material {" << endl;
			material->Node::outputContext(printStream, indentString, "\t");
			printStream << indentString << "\t" << "}" << endl;
		}
		else 
			printStream << indentString << "\t" << "material USE " << material->getName() << endl;
	}

	TextureNode *texture = getTextureNode();
	if (texture != NULL) {
		if (texture->isInstanceNode() == false) {
			if (texture->getName() != NULL && strlen(texture->getName()))
				printStream << indentString << "\t" << "texture " << "DEF " << texture->getName() << " " << texture->Node::getType() << " {" << endl;
			else
				printStream << indentString << "\t" << "texture " << texture->Node::getType() << " {" << endl;
			texture->Node::outputContext(printStream, indentString, "\t");
			printStream << indentString << "\t" << "}" << endl;
		}
		else 
			printStream << indentString << "\t" << "texture USE " << texture->getName() << endl;
	}

	TextureTransformNode *textureTransform = getTextureTransformNodes();
	if (textureTransform != NULL) {
		if (textureTransform->isInstanceNode() == false) {
			if (textureTransform->getName() != NULL && strlen(textureTransform->getName()))
				printStream << indentString << "\t" << "textureTransform " << "DEF " << textureTransform->getName() << " TextureTransform {" << endl;
			else
				printStream << indentString << "\t" << "textureTransform TextureTransform {" << endl;
			textureTransform->Node::outputContext(printStream, indentString, "\t");
			printStream << indentString << "\t" << "}" << endl;
		}
		else 
			printStream << indentString << "\t" << "textureTransform USE " << textureTransform->getName() << endl;
	}
}
