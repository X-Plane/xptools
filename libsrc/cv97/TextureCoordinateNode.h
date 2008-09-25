/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	TextureCoordinateNode.h
*
******************************************************************/

#ifndef _CV97_TEXTURECOORDINATE_H_
#define _CV97_TEXTURECOORDINATE_H_

#include "VRMLField.h"
#include "Node.h"

class TextureCoordinateNode : public Node {

	MFVec2f *pointField;

public:

	TextureCoordinateNode();
	~TextureCoordinateNode();

	////////////////////////////////////////////////
	//	point
	////////////////////////////////////////////////

	MFVec2f *getPointField();

	void addPoint(float point[]);
	int getNPoints();
	void getPoint(int index, float point[]);

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

	TextureCoordinateNode *next();
	TextureCoordinateNode *nextTraversal();

};

#endif

