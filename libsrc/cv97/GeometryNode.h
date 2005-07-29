/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	GeometryNode.h
*
******************************************************************/

#ifndef _CV97_GEOMETRYNODE_H_
#define _CV97_GEOMETRYNODE_H_

#include "VRMLField.h"
#include "Node.h"
#include "BoundingBox.h"

#define	bboxCenterPrivateFieldName		"bboxCenter"
#define	bboxSizePrivateFieldName		"bboxSize"

#ifdef SUPPORT_OPENGL
#define	displayListPrivateFieldString	"oglDisplayList"
#endif

class GeometryNode : public Node 
{

	SFVec3f *bboxCenterField;
	SFVec3f *bboxSizeField;

#ifdef SUPPORT_OPENGL
	SFInt32 *dispListField;
#endif

public:

	GeometryNode();
	virtual ~GeometryNode();

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

	////////////////////////////////////////////////
	//	DisplayList
	////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

	SFInt32 *getDisplayListField();
	void setDisplayList(unsigned int n);
	unsigned int getDisplayList();
	virtual void draw();

#endif
};

#endif

