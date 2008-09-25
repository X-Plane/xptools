/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	BackgroundNode.h
*
******************************************************************/

#ifndef _CV97_BACKGROUND_H_
#define _CV97_BACKGROUND_H_

#include "BindableNode.h"

class BackgroundNode : public BindableNode {

	MFColor *groundColorField;
	MFColor *skyColorField;
	MFFloat *groundAngleField;
	MFFloat *skyAngleField;
	MFString *frontUrlField;
	MFString *backUrlField;
	MFString *leftUrlField;
	MFString *rightUrlField;
	MFString *topUrlField;
	MFString *bottomUrlField;

public:

	BackgroundNode();
	~BackgroundNode();

	////////////////////////////////////////////////
	// groundColor
	////////////////////////////////////////////////

	MFColor *getGroundColorField();

	void addGroundColor(float value[]);
	int getNGroundColors();
	void getGroundColor(int index, float value[]);

	////////////////////////////////////////////////
	// skyColor
	////////////////////////////////////////////////

	MFColor *getSkyColorField();

	void addSkyColor(float value[]);
	int getNSkyColors();
	void getSkyColor(int index, float value[]);

	////////////////////////////////////////////////
	// groundAngle
	////////////////////////////////////////////////

	MFFloat *getGroundAngleField();

	void addGroundAngle(float value);
	int getNGroundAngles();
	float getGroundAngle(int index);

	////////////////////////////////////////////////
	// skyAngle
	////////////////////////////////////////////////

	MFFloat *getSkyAngleField();

	void addSkyAngle(float value);
	int getNSkyAngles();
	float getSkyAngle(int index);

	////////////////////////////////////////////////
	// frontUrl
	////////////////////////////////////////////////

	MFString *getFrontUrlField();

	void addFrontUrl(char *value);
	int getNFrontUrls();
	char *getFrontUrl(int index);

	////////////////////////////////////////////////
	// backUrl
	////////////////////////////////////////////////

	MFString *getBackUrlField();

	void addBackUrl(char *value);
	int getNBackUrls();
	char *getBackUrl(int index);

	////////////////////////////////////////////////
	// leftUrl
	////////////////////////////////////////////////

	MFString *getLeftUrlField();

	void addLeftUrl(char *value);
	int getNLeftUrls();
	char *getLeftUrl(int index);

	////////////////////////////////////////////////
	// rightUrl
	////////////////////////////////////////////////

	MFString *getRightUrlField();

	void addRightUrl(char *value);
	int getNRightUrls();
	char *getRightUrl(int index);

	////////////////////////////////////////////////
	// topUrl
	////////////////////////////////////////////////

	MFString *getTopUrlField();

	void addTopUrl(char *value);
	int getNTopUrls();
	char *getTopUrl(int index);

	////////////////////////////////////////////////
	// bottomUrl
	////////////////////////////////////////////////

	MFString *getBottomUrlField();

	void addBottomUrl(char *value);
	int getNBottomUrls();
	char *getBottomUrl(int index);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	BackgroundNode *next();
	BackgroundNode *nextTraversal();

	////////////////////////////////////////////////
	//	Virtual functions
	////////////////////////////////////////////////

	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();
	void outputContext(ostream &printStream, char *indentString);
};

#endif

