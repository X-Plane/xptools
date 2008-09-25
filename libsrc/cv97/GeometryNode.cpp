/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	GeometryNode.cpp
*
******************************************************************/

#ifdef SUPPORT_OPENGL
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#endif

#include "GeometryNode.h"

GeometryNode::GeometryNode()
{
	// bboxCenter field
	bboxCenterField = new SFVec3f(0.0f, 0.0f, 0.0f);
	bboxCenterField->setName(bboxCenterPrivateFieldName);
	addPrivateField(bboxCenterField);

	// bboxSize field
	bboxSizeField = new SFVec3f(-1.0f, -1.0f, -1.0f);
	bboxSizeField->setName(bboxSizePrivateFieldName);
	addPrivateField(bboxSizeField);

	setBoundingBoxCenter(0.0f, 0.0f, 0.0f);
	setBoundingBoxSize(-1.0f, -1.0f, -1.0f);

#ifdef SUPPORT_OPENGL
	// display list field
	dispListField = new SFInt32(0);
	dispListField->setName(displayListPrivateFieldString);
	addPrivateField(dispListField);

	setDisplayList(0);
#endif
}

GeometryNode::~GeometryNode()
{
}

////////////////////////////////////////////////
//	BoundingBoxSize
////////////////////////////////////////////////

SFVec3f *GeometryNode::getBoundingBoxSizeField()
{
	if (isInstanceNode() == false)
		return bboxSizeField;
	return (SFVec3f *)getPrivateField(bboxSizePrivateFieldName);
}

void GeometryNode::setBoundingBoxSize(float value[])
{
	getBoundingBoxSizeField()->setValue(value);
}

void GeometryNode::setBoundingBoxSize(float x, float y, float z)
{
	getBoundingBoxSizeField()->setValue(x, y, z);
}

void GeometryNode::getBoundingBoxSize(float value[])
{
	getBoundingBoxSizeField()->getValue(value);
}

////////////////////////////////////////////////
//	BoundingBoxCenter
////////////////////////////////////////////////

SFVec3f *GeometryNode::getBoundingBoxCenterField()
{
	if (isInstanceNode() == false)
		return bboxCenterField;
	return (SFVec3f *)getPrivateField(bboxCenterPrivateFieldName);
}

void GeometryNode::setBoundingBoxCenter(float value[])
{
	getBoundingBoxCenterField()->setValue(value);
}

void GeometryNode::setBoundingBoxCenter(float x, float y, float z)
{
	getBoundingBoxCenterField()->setValue(x, y, z);
}

void GeometryNode::getBoundingBoxCenter(float value[])
{
	getBoundingBoxCenterField()->getValue(value);
}

////////////////////////////////////////////////
//	BoundingBox
////////////////////////////////////////////////

void GeometryNode::setBoundingBox(BoundingBox *bbox)
{
	float center[3];
	float size[3];
	bbox->getCenter(center);
	bbox->getSize(size);
	setBoundingBoxCenter(center);
	setBoundingBoxSize(size);
}

////////////////////////////////////////////////
//	OpenGL
////////////////////////////////////////////////

#ifdef SUPPORT_OPENGL

SFInt32 *GeometryNode::getDisplayListField()
{
	if (isInstanceNode() == false)
		return dispListField;
	return (SFInt32 *)getPrivateField(displayListPrivateFieldString);
}

void GeometryNode::setDisplayList(unsigned int n)
{
	getDisplayListField()->setValue((int)n);
}

unsigned int GeometryNode::getDisplayList()
{
	return (unsigned int)getDisplayListField()->getValue();
}

void GeometryNode::draw()
{
	unsigned int nDisplayList = getDisplayList();
	if (0 < nDisplayList)
		glCallList(nDisplayList);
}

#endif