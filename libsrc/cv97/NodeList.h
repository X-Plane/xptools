/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	NodeList.h
*
******************************************************************/

#ifndef _CV97_NODELIST_H_
#define _CV97_NODELIST_H_

#include "LinkedList.h"
#include "RootNode.h"

class NodeList : public LinkedList<Node> {

public:

	NodeList();
	~NodeList();
};

#endif
