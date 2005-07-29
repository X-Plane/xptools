/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	NodeList.cpp
*
******************************************************************/

#include "NodeList.h"

NodeList::NodeList() 
{
	RootNode *rootNode = new RootNode();
	setRootNode(rootNode);
}

NodeList::~NodeList() 
{
}
