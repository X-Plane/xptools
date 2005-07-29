/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	RouteNode.h
*
******************************************************************/

#ifndef _CV97_ROOTNODE_H_
#define _CV97_ROOTNODE_H_

#include "VRMLField.h"
#include "Node.h"

class RootNode : public Node {

public:

	RootNode();
	~RootNode();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////
	
	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	infomation
	////////////////////////////////////////////////

	void outputContext(ostream& printStream, char * indentString);
};

#endif
