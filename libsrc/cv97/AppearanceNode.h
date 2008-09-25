/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	AppearanceNode.h
*
******************************************************************/

#ifndef _CV97_APPEARANCE_H_
#define _CV97_APPEARANCE_H_

#include "VRMLField.h"
#include "Node.h"
#include "MaterialNode.h"
#include "TextureTransformNode.h"

class AppearanceNode : public Node {

public:

	AppearanceNode();
	~AppearanceNode();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	AppearanceNode *next();
	AppearanceNode *nextTraversal();

	////////////////////////////////////////////////
	//	virtual functions
	////////////////////////////////////////////////

	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();
	void outputContext(ostream &printStream, char *indentString);
};

#endif
