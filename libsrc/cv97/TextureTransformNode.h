/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	TextureTransformNode.h
*
******************************************************************/

#ifndef _CV97_TEXTURETRANSFORM_H_
#define _CV97_TEXTURETRANSFORM_H_

#include "VRMLField.h"
#include "Node.h"

class TextureTransformNode : public Node {

	SFVec2f *translationField;
	SFVec2f *scaleField;
	SFVec2f *centerField;
	SFFloat *rotationField;

public:

	TextureTransformNode();
	~TextureTransformNode();

	////////////////////////////////////////////////
	//	Translation
	////////////////////////////////////////////////

	SFVec2f *getTranslationField();

	void setTranslation(float value[]);
	void setTranslation(float x, float y);
	void getTranslation(float value[]);

	////////////////////////////////////////////////
	//	Scale
	////////////////////////////////////////////////

	SFVec2f *getScaleField();

	void setScale(float value[]);
	void setScale(float x, float y);
	void getScale(float value[]);

	////////////////////////////////////////////////
	//	Center
	////////////////////////////////////////////////

	SFVec2f *getCenterField();

	void setCenter(float value[]);
	void setCenter(float x, float y);
	void getCenter(float value[]);

	////////////////////////////////////////////////
	//	Rotation
	////////////////////////////////////////////////

	SFFloat *getRotationField();

	void setRotation(float value);
	float getRotation();

	////////////////////////////////////////////////
	//	Texture Matrix
	////////////////////////////////////////////////

	void getSFMatrix(SFMatrix *matrix);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	TextureTransformNode *next();
	TextureTransformNode *nextTraversal();

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

