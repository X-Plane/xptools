/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	ElevationGridNode.h
*
******************************************************************/

#ifndef _CV97_ELEVATIONGRID_H_
#define _CV97_ELEVATIONGRID_H_

#include "GeometryNode.h"
#include "ColorNode.h"
#include "NormalNode.h"
#include "TextureCoordinateNode.h"

class ElevationGridNode : public GeometryNode {

	SFFloat *xSpacingField;
	SFFloat *zSpacingField;
	SFInt32 *xDimensionField;
	SFInt32 *zDimensionField;
	SFBool *colorPerVertexField;
	SFBool *normalPerVertexField;
	SFBool *ccwField;
	SFBool *solidField;
	SFFloat *creaseAngleField;
	MFFloat *heightField;

public:

	ElevationGridNode();
	~ElevationGridNode();

	////////////////////////////////////////////////
	//	xSpacing
	////////////////////////////////////////////////

	SFFloat *getXSpacingField();

	void setXSpacing(float value);
	float getXSpacing();

	////////////////////////////////////////////////
	//	zSpacing
	////////////////////////////////////////////////

	SFFloat *getZSpacingField();

	void setZSpacing(float value);
	float getZSpacing();

	////////////////////////////////////////////////
	//	xDimension
	////////////////////////////////////////////////

	SFInt32 *getXDimensionField();

	void setXDimension(int value);
	int getXDimension();

	////////////////////////////////////////////////
	//	zDimension
	////////////////////////////////////////////////

	SFInt32 *getZDimensionField();

	void setZDimension(int value);
	int getZDimension();

	////////////////////////////////////////////////
	//	ColorPerVertex
	////////////////////////////////////////////////

	SFBool *getColorPerVertexField();

	void setColorPerVertex(bool  value);
	void setColorPerVertex(int value);
	bool  getColorPerVertex();

	////////////////////////////////////////////////
	//	NormalPerVertex
	////////////////////////////////////////////////

	SFBool *getNormalPerVertexField();

	void setNormalPerVertex(bool  value);
	void setNormalPerVertex(int value);
	bool  getNormalPerVertex();

	////////////////////////////////////////////////
	//	CCW
	////////////////////////////////////////////////

	SFBool *getCCWField();

	void setCCW(bool  value);
	void setCCW(int value);
	bool  getCCW();

	////////////////////////////////////////////////
	//	Solid
	////////////////////////////////////////////////

	SFBool *getSolidField();

	void setSolid(bool  value);
	void setSolid(int value);
	bool  getSolid();

	////////////////////////////////////////////////
	//	CreaseAngle
	////////////////////////////////////////////////

	SFFloat *getCreaseAngleField();

	void setCreaseAngle(float value);
	float getCreaseAngle();

	////////////////////////////////////////////////
	// height
	////////////////////////////////////////////////

	MFFloat *getHeightField();

	void addHeight(float value);
	int getNHeights();
	float getHeight(int index);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	ElevationGridNode *next();
	ElevationGridNode *nextTraversal();

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

