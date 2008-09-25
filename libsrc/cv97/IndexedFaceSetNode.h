/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	IndexedFaceSetNode.h
*
******************************************************************/

#ifndef _CV97_INDEXEDFACESET_H_
#define _CV97_INDEXEDFACESET_H_

#include "GeometryNode.h"
#include "NormalNode.h"
#include "ColorNode.h"
#include "CoordinateNode.h"
#include "TextureCoordinateNode.h"

class IndexedFaceSetNode : public GeometryNode {

	SFBool *ccwField;
	SFBool *colorPerVertexField;
	SFBool *normalPerVertexField;
	SFBool *solidField;
	SFBool *convexField;
	SFFloat *creaseAngleField;
	MFInt32 *coordIdxField;
	MFInt32 *texCoordIndexField;
	MFInt32 *colorIndexField;
	MFInt32 *normalIndexField;

public:

	IndexedFaceSetNode();
	~IndexedFaceSetNode();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	IndexedFaceSetNode *next();
	IndexedFaceSetNode *nextTraversal();

	////////////////////////////////////////////////
	//	CCW
	////////////////////////////////////////////////

	SFBool *getCCWField();

	void setCCW(bool value);
	void setCCW(int value);
	bool getCCW();

	////////////////////////////////////////////////
	//	ColorPerVertex
	////////////////////////////////////////////////

	SFBool *getColorPerVertexField();

	void setColorPerVertex(bool value);
	void setColorPerVertex(int value);
	bool getColorPerVertex();

	////////////////////////////////////////////////
	//	NormalPerVertex
	////////////////////////////////////////////////

	SFBool *getNormalPerVertexField();

	void setNormalPerVertex(bool value);
	void setNormalPerVertex(int value);
	bool getNormalPerVertex();

	////////////////////////////////////////////////
	//	Solid
	////////////////////////////////////////////////

	SFBool *getSolidField();

	void setSolid(bool value);
	void setSolid(int value);
	bool getSolid();

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
	// CoordIndex
	////////////////////////////////////////////////

	MFInt32 *getCoordIndexField();

	void addCoordIndex(int value);
	int getNCoordIndexes();
	int getCoordIndex(int index);

	////////////////////////////////////////////////
	// TexCoordIndex
	////////////////////////////////////////////////

	MFInt32 *getTexCoordIndexField();

	void addTexCoordIndex(int value);
	int getNTexCoordIndexes();
	int getTexCoordIndex(int index);

	////////////////////////////////////////////////
	// ColorIndex
	////////////////////////////////////////////////

	MFInt32 *getColorIndexField();

	void addColorIndex(int value);
	int getNColorIndexes();
	int getColorIndex(int index);

	////////////////////////////////////////////////
	// NormalIndex
	////////////////////////////////////////////////

	MFInt32 *getNormalIndexField();

	void addNormalIndex(int value);
	int getNNormalIndexes();
	int getNormalIndex(int index);

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

#ifdef SUPPORT_OPENGL
	void recomputeDisplayList();
#endif

	////////////////////////////////////////////////
	//	Infomation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);

	////////////////////////////////////////////////
	//	Polygon
	////////////////////////////////////////////////

	int		getNPolygons();

	////////////////////////////////////////////////
	//	Normal
	////////////////////////////////////////////////

	bool generateNormals();

	////////////////////////////////////////////////
	//	TextureCoordinate
	////////////////////////////////////////////////

	bool generateTextureCoordinate();

};

#endif

