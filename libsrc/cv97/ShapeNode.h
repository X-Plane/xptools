/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ShapeNode.h
*
******************************************************************/

#ifndef _CV97_SHAPE_H_
#define _CV97_SHAPE_H_

#include "VRMLField.h"
#include "Node.h"
#include "AppearanceNode.h"
#include "GeometryNode.h"

class ShapeNode : public Node {

public:

	ShapeNode();
	~ShapeNode();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	ShapeNode *next();
	ShapeNode *nextTraversal();

	////////////////////////////////////////////////
	//	Geometry
	////////////////////////////////////////////////

	GeometryNode *getGeometry();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////

	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	Infomation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif

