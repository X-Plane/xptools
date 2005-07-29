/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	GroupingNode.h
*
******************************************************************/

#ifndef _CV97_GROUPINGNODE_H_
#define _CV97_GROUPINGNODE_H_

#include "Node.h"
#include "BoundingBox.h"

#define	addChildrenEventIn			"addChildren"
#define	removeChildrenEventIn		"removeChildren"
#define	bboxCenterFieldName			"bboxCenter"
#define	bboxSizeFieldName			"bboxSize"

class GroupingNode : public Node {

	SFVec3f *bboxCenterField;
	SFVec3f *bboxSizeField;

public:

	GroupingNode();
	virtual ~GroupingNode();

	////////////////////////////////////////////////
	//	BoundingBoxSize
	////////////////////////////////////////////////

	SFVec3f *getBoundingBoxSizeField();

	void setBoundingBoxSize(float value[]);
	void setBoundingBoxSize(float x, float y, float z);
	void getBoundingBoxSize(float value[]);

	////////////////////////////////////////////////
	//	BoundingBoxCenter
	////////////////////////////////////////////////

	SFVec3f *getBoundingBoxCenterField();

	void setBoundingBoxCenter(float value[]);
	void setBoundingBoxCenter(float x, float y, float z);
	void getBoundingBoxCenter(float value[]);

	////////////////////////////////////////////////
	//	BoundingBox
	////////////////////////////////////////////////

	void setBoundingBox(BoundingBox *bbox);
	void recomputeBoundingBox();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	GroupingNode *next();
	GroupingNode *nextTraversal();
};

#endif

