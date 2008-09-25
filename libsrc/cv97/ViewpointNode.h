/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ViewpointNode.h
*
******************************************************************/

#ifndef _CV97_VIEWPOINT_H_
#define _CV97_VIEWPOINT_H_

#include "BindableNode.h"

class ViewpointNode : public BindableNode {

	SFVec3f *positionField;
	SFRotation *orientationField;
	SFString *descriptionField;
	SFFloat *fovField;
	SFBool *jumpField;

public:

	ViewpointNode();
	~ViewpointNode();

	////////////////////////////////////////////////
	//	Jump
	////////////////////////////////////////////////

	SFBool *getJumpField();

	void setJump(bool value);
	void setJump(int value);
	bool getJump();

	////////////////////////////////////////////////
	//	FieldOfView
	////////////////////////////////////////////////

	SFFloat *getFieldOfViewField();

	void setFieldOfView(float value);
	float getFieldOfView();

	////////////////////////////////////////////////
	//	Description
	////////////////////////////////////////////////

	SFString *getDescriptionField();

	void setDescription(char *value);
	char *getDescription();

	////////////////////////////////////////////////
	//	Position
	////////////////////////////////////////////////

	SFVec3f *getPositionField();

	void setPosition(float value[]);
	void setPosition(float x, float y, float z);
	void getPosition(float value[]);

	////////////////////////////////////////////////
	//	Orientation
	////////////////////////////////////////////////

	SFRotation *getOrientationField();

	void setOrientation(float value[]);
	void setOrientation(float x, float y, float z, float w);
	void getOrientation(float value[]);

	////////////////////////////////////////////////
	//	Add position
	////////////////////////////////////////////////

	void addPosition(float worldTranslation[3]);
	void addPosition(float worldx, float worldy, float worldz);
	void addPosition(float localTranslation[3], float frame[3][3]);
	void addPosition(float x, float y, float z, float frame[3][3]);

	////////////////////////////////////////////////
	//	Add orientation
	////////////////////////////////////////////////

	void addOrientation(SFRotation *rot);
	void addOrientation(float rotationValue[4]);
	void addOrientation(float x, float y, float z, float rot);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	ViewpointNode *next();
	ViewpointNode *nextTraversal();

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

	void outputContext(ostream& printStream, char *indentString);

	////////////////////////////////////////////////
	//	Local frame
	////////////////////////////////////////////////

	void getFrame(float frame[3][3]);
	void translate(float vector[3]);
	void translate(SFVec3f vec);
	void rotate(float rotation[4]);
	void rotate(SFRotation rot);

	////////////////////////////////////////////////
	//	ViewpointNode Matrix
	////////////////////////////////////////////////

	void getMatrix(SFMatrix *matrix);
	void getMatrix(float value[4][4]);
	void getTranslationMatrix(SFMatrix *matrix);
	void getTranslationMatrix(float value[4][4]);
};

#endif

