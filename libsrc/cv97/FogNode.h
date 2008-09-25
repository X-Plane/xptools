/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	FogNode.h
*
******************************************************************/

#ifndef _CV97_FOG_H_
#define _CV97_FOG_H_

#include "BindableNode.h"

class FogNode : public BindableNode {

	SFColor *colorField;
	SFString *fogTypeField;
	SFFloat *visibilityRangeField;

public:

	FogNode();
	~FogNode();

	////////////////////////////////////////////////
	//	Color
	////////////////////////////////////////////////

	SFColor *getColorField();

	void setColor(float value[]);
	void setColor(float r, float g, float b);
	void getColor(float value[]);

	////////////////////////////////////////////////
	//	FogType
	////////////////////////////////////////////////

	SFString *getFogTypeField();

	void setFogType(char *value);
	char *getFogType();

	////////////////////////////////////////////////
	//	VisibilityRange
	////////////////////////////////////////////////

	SFFloat *getVisibilityRangeField();

	void setVisibilityRange(float value);
	float getVisibilityRange();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	FogNode *next();
	FogNode *nextTraversal();

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

