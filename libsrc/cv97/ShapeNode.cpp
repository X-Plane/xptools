/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ShapeNode.cpp
*
******************************************************************/

#include "ShapeNode.h"

ShapeNode::ShapeNode() 
{
	setHeaderFlag(false);
	setType(shapeNodeString);
}

ShapeNode::~ShapeNode() 
{
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

ShapeNode *ShapeNode::next() 
{
	return (ShapeNode *)Node::next(getType());
}

ShapeNode *ShapeNode::nextTraversal() 
{
	return (ShapeNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	Geometry
////////////////////////////////////////////////

GeometryNode *ShapeNode::getGeometry() 
{
	for (Node *node=getChildNodes(); node; node=node->next()) {
		if (node->isGeometryNode())
			return (GeometryNode *)node;
	}
	return NULL;
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////
	
bool ShapeNode::isChildNodeType(Node *node)
{
	if (node->isAppearanceNode() || node->isGeometryNode())
		return true;
	else
		return false;
}

void ShapeNode::initialize() 
{
}

void ShapeNode::uninitialize() 
{
}

void ShapeNode::update() 
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void ShapeNode::outputContext(ostream &printStream, char *indentString) 
{
	AppearanceNode *appearance = getAppearanceNodes();
	if (appearance != NULL) {
		if (appearance->isInstanceNode() == false) {
			if (appearance->getName() != NULL && strlen(appearance->getName()))
				printStream << indentString << "\t" << "appearance " << "DEF " << appearance->getName() << " Appearance {" << endl;
			else
				printStream << indentString << "\t" << "appearance Appearance {" << endl;
			appearance->Node::outputContext(printStream, indentString, "\t");
			printStream << indentString << "\t" << "}" << endl;
		}
		else 
			printStream << indentString << "\t" << "appearance USE " << appearance->getName() << endl;
	}
	
	Node *node = getGeometryNode();
	if (node != NULL) {
		if (node->isInstanceNode() == false) {
			if (node->getName() != NULL && strlen(node->getName()))
				printStream << indentString << "\t" << "geometry " << "DEF " << node->getName() << " " << node->Node::getType() << " {" << endl;
			else
				printStream << indentString << "\t" << "geometry " << node->getType() << " {" << endl;
			node->Node::outputContext(printStream, indentString, "\t");
			printStream << indentString << "\t" << "}" << endl;
		}
		else 
			printStream << indentString << "\t" << "geometry USE " << node->getName() << endl;
	}
}

