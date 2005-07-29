/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ExtrusionNode.h
*
******************************************************************/

#ifndef _CV97_EXTRUSION_H_
#define _CV97_EXTRUSION_H_

#include "GeometryNode.h"

class ExtrusionNode : public GeometryNode {

	SFBool *beginCapField;
	SFBool *endCapField;
	SFBool *ccwField;
	SFBool *convexField;
	SFFloat *creaseAngleField;
	SFBool *solidField;
	MFRotation *orientationField;
	MFVec2f *scaleField;
	MFVec2f *crossSectionField;
	MFVec3f *spineField;

public:

	ExtrusionNode();
	~ExtrusionNode();

	////////////////////////////////////////////////
	//	BeginCap
	////////////////////////////////////////////////
	
	SFBool *getBeginCapField();

	void setBeginCap(bool value);
	void setBeginCap(int value);
	bool getBeginCap();

	////////////////////////////////////////////////
	//	EndCap
	////////////////////////////////////////////////

	SFBool *getEndCapField();
	
	void setEndCap(bool value);
	void setEndCap(int value);
	bool getEndCap();

	////////////////////////////////////////////////
	//	CCW
	////////////////////////////////////////////////

	SFBool *getCCWField();
	
	void setCCW(bool value);
	void setCCW(int value);
	bool getCCW();

	////////////////////////////////////////////////
	//	Convex
	////////////////////////////////////////////////

	SFBool *getConvexField();
	
	void setConvex(bool value);
	void setConvex(int value);
	bool getConvex();

	////////////////////////////////////////////////
	//	CreaseAngle
	////////////////////////////////////////////////

	SFFloat *getCreaseAngleField();
	
	void setCreaseAngle(float value);
	float getCreaseAngle();

	////////////////////////////////////////////////
	//	Solid
	////////////////////////////////////////////////

	SFBool *getSolidField();
	
	void setSolid(bool value);
	void setSolid(int value);
	bool getSolid();

	////////////////////////////////////////////////
	// orientation
	////////////////////////////////////////////////

	MFRotation *getOrientationField();

	void addOrientation(float value[]);
	void addOrientation(float x, float y, float z, float angle);
	int getNOrientations();
	void getOrientation(int index, float value[]);

	////////////////////////////////////////////////
	// scale
	////////////////////////////////////////////////

	MFVec2f *getScaleField();

	void addScale(float value[]);
	void addScale(float x, float z);
	int getNScales();
	void getScale(int index, float value[]);

	////////////////////////////////////////////////
	// crossSection
	////////////////////////////////////////////////

	MFVec2f *getCrossSectionField();

	void addCrossSection(float value[]);
	void addCrossSection(float x, float z);
	int getNCrossSections();
	void getCrossSection(int index, float value[]);

	////////////////////////////////////////////////
	// spine
	////////////////////////////////////////////////

	MFVec3f *getSpineField();

	void addSpine(float value[]);
	void addSpine(float x, float y, float z);
	int getNSpines();
	void getSpine(int index, float value[]);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	ExtrusionNode *next();
	ExtrusionNode *nextTraversal();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////
	
	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	BoundingBox
	////////////////////////////////////////////////

	void recomputeBoundingBox();

	////////////////////////////////////////////////
	//	recomputeDisplayList
	////////////////////////////////////////////////

	void recomputeDisplayList();

	////////////////////////////////////////////////
	//	Infomation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif

