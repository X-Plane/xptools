/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	LodNode.h
*
******************************************************************/

#ifndef _CV97_LOD_H_
#define _CV97_LOD_H_

#include "VRMLField.h"
#include "Node.h"

class LodNode : public Node {

	SFVec3f *centerField;
	MFFloat *rangeField;

public:

	LodNode();
	~LodNode();
	
	////////////////////////////////////////////////
	//	center
	////////////////////////////////////////////////

	SFVec3f *getCenterField();

	void setCenter(float value[]);
	void setCenter(float x, float y, float z);
	void getCenter(float value[]);

	////////////////////////////////////////////////
	//	range 
	////////////////////////////////////////////////

	MFFloat *getRangeField();

	void addRange(float value);
	int getNRanges();
	float getRange(int index);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	LodNode *next();
	LodNode *nextTraversal();

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

void UpdateLod(LodNode *lod);
void InitializeLod(LodNode *lod);
void UninitializeLod(LodNode *lod);

#endif

