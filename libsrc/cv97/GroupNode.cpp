/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	GroupNode.cpp
*
******************************************************************/

#include "GroupNode.h"

GroupNode::GroupNode()
{
	setHeaderFlag(false);
	setType(groupNodeString);
}

GroupNode::~GroupNode()
{
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void GroupNode::outputContext(ostream &printStream, char *indentString)
{
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool GroupNode::isChildNodeType(Node *node)
{
	if (node->isCommonNode() || node->isBindableNode() ||node->isInterpolatorNode() || node->isSensorNode() || node->isGroupingNode() || node->isSpecialGroupNode())
		return true;
	else
		return false;
}

void GroupNode::initialize()
{
	recomputeBoundingBox();
}

void GroupNode::uninitialize()
{
}

void GroupNode::update()
{
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

GroupNode *GroupNode::next()
{
	return (GroupNode *)Node::next(getType());
}

GroupNode *GroupNode::nextTraversal()
{
	return (GroupNode *)Node::nextTraversalByType(getType());
}

