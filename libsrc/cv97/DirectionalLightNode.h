/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	DirectionalLightNode.h
*
******************************************************************/

#ifndef _CV97_DIRECTIONALLIGHT_H_
#define _CV97_DIRECTIONALLIGHT_H_

#include "VRMLField.h"
#include "LightNode.h"

class DirectionalLightNode : public LightNode {

	SFFloat *ambientIntensityField;
	SFVec3f *directionField;
	
public:

	DirectionalLightNode();
	~DirectionalLightNode();

	////////////////////////////////////////////////
	//	AmbientIntensity
	////////////////////////////////////////////////
	
	SFFloat *getAmbientIntensityField();

	void setAmbientIntensity(float value);
	float getAmbientIntensity();

	////////////////////////////////////////////////
	//	Direction
	////////////////////////////////////////////////

	SFVec3f *getDirectionField();

	void setDirection(float value[]);
	void setDirection(float x, float y, float z);
	void getDirection(float value[]);

	////////////////////////////////////////////////
	//	Diffuse Color
	////////////////////////////////////////////////

	void getDiffuseColor(float value[]);

	////////////////////////////////////////////////
	//	Ambient Color
	////////////////////////////////////////////////

	void getAmbientColor(float value[]);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	DirectionalLightNode *next();
	DirectionalLightNode *nextTraversal();

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

