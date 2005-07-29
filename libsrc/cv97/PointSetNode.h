/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	PointSetNode.h
*
******************************************************************/

#ifndef _CV97_POINTSET_H_
#define _CV97_POINTSET_H_

#include "GeometryNode.h"
#include "ColorNode.h"
#include "CoordinateNode.h"

class PointSetNode : public GeometryNode {

public:

	PointSetNode();
	~PointSetNode();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	PointSetNode *next();
	PointSetNode *nextTraversal();

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

