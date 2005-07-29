/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	PointLightNode.h
*
******************************************************************/

#ifndef _CV97_POINTLIGHT_H_
#define _CV97_POINTLIGHT_H_

#include "LightNode.h"

class PointLightNode : public LightNode {

	SFFloat *ambientIntensityField;
	SFVec3f *locationField;
	SFFloat *radiusField;
	SFVec3f *attenuationField;

public:

	PointLightNode();
	~PointLightNode();

	////////////////////////////////////////////////
	//	AmbientIntensity
	////////////////////////////////////////////////

	SFFloat *getAmbientIntensityField();
	
	void setAmbientIntensity(float value);
	float getAmbientIntensity();

	////////////////////////////////////////////////
	//	Location
	////////////////////////////////////////////////

	SFVec3f *getLocationField();

	void setLocation(float value[]);
	void setLocation(float x, float y, float z);
	void getLocation(float value[]);

	////////////////////////////////////////////////
	//	Radius
	////////////////////////////////////////////////

	SFFloat *getRadiusField();
	
	void setRadius(float value);
	float getRadius();

	////////////////////////////////////////////////
	//	Attenuation
	////////////////////////////////////////////////

	SFVec3f *getAttenuationField();

	void setAttenuation(float value[]);
	void setAttenuation(float x, float y, float z);
	void getAttenuation(float value[]);

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

	PointLightNode *next();
	PointLightNode *nextTraversal();

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
