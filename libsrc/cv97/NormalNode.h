/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	NormalNode.h
*
******************************************************************/

#ifndef _CV97_NORMAL_H_
#define _CV97_NORMAL_H_

#include "VRMLField.h"
#include "Node.h"

class NormalNode : public Node {

	MFVec3f *vectorField;

public:

	NormalNode();
	~NormalNode();

	////////////////////////////////////////////////
	//	vector
	////////////////////////////////////////////////
	
	MFVec3f *getVectorField();

	void addVector(float value[]);
	int getNVectors();
	void getVector(int index, float value[]);

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

	NormalNode *next();
	NormalNode *nextTraversal();
};

#endif

