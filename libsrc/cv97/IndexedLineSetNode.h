/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	IndexedLinSet.h
*
******************************************************************/

#ifndef _CV97_INDEXEDLINESET_H_
#define _CV97_INDEXEDLINESET_H_

#include "GeometryNode.h"
#include "ColorNode.h"
#include "CoordinateNode.h"

class IndexedLineSetNode : public GeometryNode {

	SFBool *colorPerVertexField;
	MFInt32 *coordIndexField;
	MFInt32 *colorIndexField;

public:

	IndexedLineSetNode();
	~IndexedLineSetNode();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	IndexedLineSetNode *next();
	IndexedLineSetNode *nextTraversal();

	////////////////////////////////////////////////
	//	ColorPerVertex
	////////////////////////////////////////////////

	SFBool *getColorPerVertexField();

	void setColorPerVertex(bool value);
	void setColorPerVertex(int value);
	bool getColorPerVertex();

	////////////////////////////////////////////////
	// CoordIndex
	////////////////////////////////////////////////

	MFInt32 *getCoordIndexField();

	void addCoordIndex(int value);
	int getNCoordIndexes();
	int getCoordIndex(int index);
	void clearCoordIndex();

	////////////////////////////////////////////////
	// ColorIndex
	////////////////////////////////////////////////

	MFInt32 *getColorIndexField();

	void addColorIndex(int value);
	int getNColorIndexes();
	int getColorIndex(int index);
	void clearColorIndex();

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
};

#endif