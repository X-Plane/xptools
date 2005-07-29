/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	CollisionNode.h
*
******************************************************************/

#ifndef _CV97_COLLISION_H_
#define _CV97_COLLISION_H_

#include "GroupingNode.h"

class CollisionNode : public GroupingNode {

	SFBool *collideField;
	SFTime *collideTimeField;

public:

	CollisionNode();
	~CollisionNode();

	////////////////////////////////////////////////
	//	collide
	////////////////////////////////////////////////

	SFBool *getCollideField();

	void setCollide(bool  value);
	void setCollide(int value);
	bool getCollide();

	////////////////////////////////////////////////
	//	collideTime
	////////////////////////////////////////////////

	SFTime *getCollideTimeField();

	void setCollideTime(double value);
	double getCollideTime();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	CollisionNode *next();
	CollisionNode *nextTraversal();

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

