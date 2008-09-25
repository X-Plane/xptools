/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	CoordinateNode.h
*
******************************************************************/

#ifndef _CV97_COOORDINATE_H_
#define _CV97_COOORDINATE_H_

#include "VRMLField.h"
#include "Node.h"

class CoordinateNode : public Node {

	MFVec3f *pointField;

public:

	CoordinateNode();
	~CoordinateNode();

	////////////////////////////////////////////////
	//	point
	////////////////////////////////////////////////

	MFVec3f *getPointField();

	void addPoint(float point[]);
	void addPoint(float x, float y, float z);
	int getNPoints();
	void getPoint(int index, float point[]);
	void setPoint(int index, float point[]);
	void setPoint(int index, float x, float y, float z);
	void removePoint(int index);
	void removeLastPoint();
	void removeFirstPoint();
	void removeAllPoints();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////

	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	Output
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	CoordinateNode *next();
	CoordinateNode *nextTraversal();
};

#endif

