/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	WorldInfoNode.cpp
*
******************************************************************/

#include "WorldInfoNode.h"

WorldInfoNode::WorldInfoNode()
{
	setHeaderFlag(false);
	setType(worldInfoNodeString);

	// title exposed field
	titleField = new SFString("");
	addField(titleFieldString, titleField);

	// info exposed field
	infoField = new MFString();
	addField(infoFieldString, infoField);
}

WorldInfoNode::~WorldInfoNode()
{
}

////////////////////////////////////////////////
//	Title
////////////////////////////////////////////////

SFString *WorldInfoNode::getTitleField()
{
	if (isInstanceNode() == false)
		return titleField;
	return (SFString *)getField(titleFieldString);
}

void WorldInfoNode::setTitle(char *value)
{
	getTitleField()->setValue(value);
}

char *WorldInfoNode::getTitle()
{
	return getTitleField()->getValue();
}

////////////////////////////////////////////////
// Info
////////////////////////////////////////////////

MFString *WorldInfoNode::getInfoField()
{
	if (isInstanceNode() == false)
		return infoField;
	return (MFString *)getField(infoFieldString);
}

void WorldInfoNode::addInfo(char *value)
{
	getInfoField()->addValue(value);
}

int WorldInfoNode::getNInfos()
{
	return getInfoField()->getSize();
}

char *WorldInfoNode::getInfo(int index)
{
	return getInfoField()->get1Value(index);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

WorldInfoNode *WorldInfoNode::next()
{
	return (WorldInfoNode *)Node::next(getType());
}

WorldInfoNode *WorldInfoNode::nextTraversal()
{
	return (WorldInfoNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool WorldInfoNode::isChildNodeType(Node *node)
{
	return false;
}

void WorldInfoNode::initialize()
{
}

void WorldInfoNode::uninitialize()
{
}

void WorldInfoNode::update()
{
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void WorldInfoNode::outputContext(ostream& printStream, char *indentString)
{
	SFString *title = getTitleField();
	printStream << indentString << "\t" << "title " << title << endl;

	if (0 < getNInfos()) {
		MFString *info = getInfoField();
		printStream <<  indentString << "\t" << "info ["  << endl;
		info->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}
