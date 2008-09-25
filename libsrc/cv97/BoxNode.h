/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	BoxNode.h
*
******************************************************************/

#ifndef _CV97_BOX_H_
#define _CV97_BOX_H_

#include "VRMLField.h"
#include "GeometryNode.h"

class BoxNode : public GeometryNode {

	SFVec3f *sizeField;

public:

	BoxNode();
	~BoxNode();

	////////////////////////////////////////////////
	//	size
	////////////////////////////////////////////////

	SFVec3f *getSizeField();

	void setSize(float value[]);
	void setSize(float x, float y, float z);
	void getSize(float value[]);
	float getX();
	float getY();
	float getZ();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	BoxNode *next();
	BoxNode *nextTraversal();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////

	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	BoundingBox
	////////////////////////////////////////////////

	void recomputeBoundingBox();

	////////////////////////////////////////////////
	//	recomputeDisplayList
	////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL
	void recomputeDisplayList();
#endif

	////////////////////////////////////////////////
	//	Infomation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif
