/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	GroupingNode.cpp
*
******************************************************************/

#include <float.h>
#include "GroupingNode.h"
#include "GeometryNode.h"

GroupingNode::GroupingNode()
{
	setHeaderFlag(false);

/*
	// addChildren eventout field
	MFNode addNodes = new MFNode();
	addEventIn(addChildrenEventIn, addNodes);

	// removeChildren eventout field
	MFNode removeChildren = new MFNode();
	addEventIn(removeChildrenEventIn, removeChildren);
*/

	// bboxCenter field
	bboxCenterField = new SFVec3f(0.0f, 0.0f, 0.0f);
	bboxCenterField->setName(bboxCenterFieldName);
	addField(bboxCenterField);

	// bboxSize field
	bboxSizeField = new SFVec3f(-1.0f, -1.0f, -1.0f);
	bboxSizeField->setName(bboxSizeFieldName);
	addField(bboxSizeField);
}

GroupingNode::~GroupingNode()
{
}

////////////////////////////////////////////////
//	BoundingBoxSize
////////////////////////////////////////////////

SFVec3f *GroupingNode::getBoundingBoxSizeField()
{
	if (isInstanceNode() == false)
		return bboxSizeField;
	return (SFVec3f *)getField(bboxSizeFieldName);
}

void GroupingNode::setBoundingBoxSize(float value[])
{
	getBoundingBoxSizeField()->setValue(value);
}

void GroupingNode::setBoundingBoxSize(float x, float y, float z)
{
	getBoundingBoxSizeField()->setValue(x, y, z);
}

void GroupingNode::getBoundingBoxSize(float value[])
{
	getBoundingBoxSizeField()->getValue(value);
}

////////////////////////////////////////////////
//	BoundingBoxCenter
////////////////////////////////////////////////

SFVec3f *GroupingNode::getBoundingBoxCenterField()
{
	if (isInstanceNode() == false)
		return bboxCenterField;
	return (SFVec3f *)getField(bboxCenterFieldName);
}

void GroupingNode::setBoundingBoxCenter(float value[])
{
	getBoundingBoxCenterField()->setValue(value);
}

void GroupingNode::setBoundingBoxCenter(float x, float y, float z)
{
	getBoundingBoxCenterField()->setValue(x, y, z);
}

void GroupingNode::getBoundingBoxCenter(float value[])
{
	getBoundingBoxCenterField()->getValue(value);
}

////////////////////////////////////////////////
//	BoundingBox
////////////////////////////////////////////////

void GroupingNode::setBoundingBox(BoundingBox *bbox)
{
	float center[3];
	float size[3];
	bbox->getCenter(center);
	bbox->getSize(size);
	setBoundingBoxCenter(center);
	setBoundingBoxSize(size);
}

////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

GroupingNode *GroupingNode::next()
{
	for (Node *node = Node::next(); node != NULL; node = node->next()) {
		if (node->isGroupNode() || node->isTransformNode() || node->isBillboardNode() || node->isCollisionNode() || node->isLodNode() || node->isSwitchNode() || node->isInlineNode())
			return (GroupingNode *)node;
	}
	return NULL;
}

GroupingNode *GroupingNode::nextTraversal()
{
	for (Node *node = Node::nextTraversal(); node != NULL; node = node->nextTraversal()) {
		if (node->isGroupNode() || node->isTransformNode() || node->isBillboardNode() || node->isCollisionNode() || node->isLodNode() || node->isSwitchNode() || node->isInlineNode())
			return (GroupingNode *)node;
	}
	return NULL;
}

////////////////////////////////////////////////
//	GroupingNode::recomputeBoundingBox
////////////////////////////////////////////////

static void RecomputeExtents(
Node		*node,
BoundingBox	*bbox)
{
	if (node->isGeometryNode()) {
		GeometryNode *gnode = (GeometryNode *)node;
		gnode->recomputeBoundingBox();

		float	bboxCenter[3];
		float	bboxSize[3];

		gnode->getBoundingBoxCenter(bboxCenter);
		gnode->getBoundingBoxSize(bboxSize);

		SFMatrix	mx;
		gnode->getTransformMatrix(&mx);

		for (int n=0; n<8; n++) {
			float	point[3];
			point[0] = (n < 4)			? bboxCenter[0] - bboxSize[0] : bboxCenter[0] + bboxSize[0];
			point[1] = (n % 2)			? bboxCenter[1] - bboxSize[1] : bboxCenter[1] + bboxSize[1];
			point[2] = ((n % 4) < 2)	? bboxCenter[2] - bboxSize[2] : bboxCenter[2] + bboxSize[2];
			mx.multi(point);
			bbox->addPoint(point);
		}
	}

	for (Node *cnode=node->getChildNodes(); cnode; cnode=cnode->next())
		RecomputeExtents(cnode, bbox);
}

void GroupingNode::recomputeBoundingBox()
{
	BoundingBox bbox;

	for (Node *node=getChildNodes(); node; node=node->next())
		RecomputeExtents(node, &bbox);

	setBoundingBox(&bbox);
}

