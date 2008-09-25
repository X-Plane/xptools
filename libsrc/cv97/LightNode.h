/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	LightNode.h
*
******************************************************************/

#ifndef _CV97_LIGHTNODE_H_
#define _CV97_LIGHTNODE_H_

#include "VRMLField.h"
#include "Node.h"

class LightNode : public Node {

	SFBool *bonField;
	SFFloat *intensityField;
	SFColor *colorField;

public:

	LightNode();
	virtual ~LightNode();

	////////////////////////////////////////////////
	//	On
	////////////////////////////////////////////////

	SFBool *getOnField();

	void setOn(bool on);
	void setOn(int value);
	bool isOn();

	////////////////////////////////////////////////
	//	Intensity
	////////////////////////////////////////////////

	SFFloat *getIntensityField();

	void setIntensity(float value);
	float getIntensity();

	////////////////////////////////////////////////
	//	Color
	////////////////////////////////////////////////

	SFColor *getColorField();

	void setColor(float value[]);
	void setColor(float r, float g, float b);
	void getColor(float value[]);

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////

	virtual bool isChildNodeType(Node *node) = 0;

	virtual void initialize() = 0;

	virtual void uninitialize() = 0;

	virtual void update() = 0;

	////////////////////////////////////////////////
	//	Infomation
	////////////////////////////////////////////////

	virtual void outputContext(ostream &printStream, char *indentString) = 0;
};

#endif

