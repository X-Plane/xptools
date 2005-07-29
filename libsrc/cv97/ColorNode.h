/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ColorNode.h
*
******************************************************************/

#ifndef _CV97_COLOR_H_
#define _CV97_COLOR_H_

#include "Node.h"

class ColorNode : public Node {

	MFColor *colorField;

public:

	ColorNode();
	~ColorNode();

	////////////////////////////////////////////////
	//	color
	////////////////////////////////////////////////
	
	MFColor *getColorField();

	void addColor(float color[]);
	int getNColors();
	void getColor(int index, float color[]);

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

	ColorNode *next();
	ColorNode *nextTraversal();
};

#endif
