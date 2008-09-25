/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SphereNode.h
*
******************************************************************/

#ifndef _CV97_SPHERE_H_
#define _CV97_SPHERE_H_

#include "GeometryNode.h"

class SphereNode : public GeometryNode {

	SFFloat *radiusField;

public:

	SphereNode();
	~SphereNode();

	////////////////////////////////////////////////
	//	Radius
	////////////////////////////////////////////////

	SFFloat *getRadiusField();

	void setRadius(float value);
	float getRadius();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	SphereNode *next();
	SphereNode *nextTraversal();

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

