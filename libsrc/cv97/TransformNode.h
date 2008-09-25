/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	TransformNode.h
*
******************************************************************/

#ifndef _CV97_TRANSFORM_H_
#define _CV97_TRANSFORM_H_

#include "VRMLField.h"
#include "Node.h"
#include "GroupingNode.h"

class TransformNode : public GroupingNode {

	SFVec3f *translationField;
	SFVec3f *scaleField;
	SFVec3f *centerField;
	SFRotation *rotationField;
	SFRotation *scaleOrientationField;

public:

	TransformNode();
	~TransformNode();

	////////////////////////////////////////////////
	//	Translation
	////////////////////////////////////////////////

	SFVec3f *getTranslationField();

	void setTranslation(float value[]);
	void setTranslation(float x, float y, float z);
	void getTranslation(float value[]);

	////////////////////////////////////////////////
	//	Scale
	////////////////////////////////////////////////

	SFVec3f *getScaleField();

	void setScale(float value[]);
	void setScale(float x, float y, float z);
	void getScale(float value[]);

	////////////////////////////////////////////////
	//	Center
	////////////////////////////////////////////////

	SFVec3f *getCenterField();

	void setCenter(float value[]);
	void setCenter(float x, float y, float z);
	void getCenter(float value[]);

	////////////////////////////////////////////////
	//	Rotation
	////////////////////////////////////////////////

	SFRotation *getRotationField();

	void setRotation(float value[]);
	void setRotation(float x, float y, float z, float w);
	void getRotation(float value[]);

	////////////////////////////////////////////////
	//	ScaleOrientation
	////////////////////////////////////////////////

	SFRotation *getScaleOrientationField();

	void setScaleOrientation(float value[]);
	void setScaleOrientation(float x, float y, float z, float w);
	void getScaleOrientation(float value[]);

	////////////////////////////////////////////////
	//	Matrix
	////////////////////////////////////////////////

	void getSFMatrix(SFMatrix *mOut);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	TransformNode *next();
	TransformNode *nextTraversal();

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

