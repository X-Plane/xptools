/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	RouteNode.cpp
*
******************************************************************/

#include "RootNode.h"

RootNode::RootNode()
{
	setHeaderFlag(true);
	setType(rootNodeString);
}

RootNode::~RootNode()
{
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool RootNode::isChildNodeType(Node *node)
{
	if (node->isCommonNode() || node->isBindableNode() ||node->isInterpolatorNode() || node->isSensorNode() || node->isGroupingNode() || node->isSpecialGroupNode())
		return true;
	else
		return false;
}

void RootNode::initialize()
{
}

void RootNode::uninitialize()
{
}

void RootNode::update()
{
}

////////////////////////////////////////////////
//	infomation
////////////////////////////////////////////////

void RootNode::outputContext(ostream& printStream, char * indentString)
{
}
