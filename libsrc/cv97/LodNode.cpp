/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Node.cpp
*
******************************************************************/

#include "SceneGraph.h"

LodNode::LodNode()
{
	setHeaderFlag(false);
	setType(lodNodeString);

	// center field
	centerField = new SFVec3f(0.0f, 0.0f, 0.0f);
	addField(centerFieldString, centerField);

	// range field
	rangeField = new MFFloat();
	addField(rangeFieldString, rangeField);
}

LodNode::~LodNode()
{
}

////////////////////////////////////////////////
//	center
////////////////////////////////////////////////

SFVec3f *LodNode::getCenterField()
{
	if (isInstanceNode() == false)
		return centerField;
	return (SFVec3f *)getField(centerFieldString);
}

void LodNode::setCenter(float value[])
{
	getCenterField()->setValue(value);
}

void LodNode::setCenter(float x, float y, float z)
{
	getCenterField()->setValue(x, y, z);
}

void LodNode::getCenter(float value[])
{
	getCenterField()->getValue(value);
}

////////////////////////////////////////////////
//	range
////////////////////////////////////////////////

MFFloat *LodNode::getRangeField()
{
	if (isInstanceNode() == false)
		return rangeField;
	return (MFFloat *)getField(rangeFieldString);
}

void LodNode::addRange(float value)
{
	getRangeField()->addValue(value);
}

int LodNode::getNRanges()
{
	return getRangeField()->getSize();
}

float LodNode::getRange(int index)
{
	return getRangeField()->get1Value(index);
}


////////////////////////////////////////////////
//	List
////////////////////////////////////////////////

LodNode *LodNode::next()
{
	return (LodNode *)Node::next(getType());
}

LodNode *LodNode::nextTraversal()
{
	return (LodNode *)Node::nextTraversalByType(getType());
}

////////////////////////////////////////////////
//	functions
////////////////////////////////////////////////

bool LodNode::isChildNodeType(Node *node)
{
	if (node->isCommonNode() || node->isBindableNode() ||node->isInterpolatorNode() || node->isSensorNode() || node->isGroupingNode() || node->isSpecialGroupNode())
		return true;
	else
		return false;
}

////////////////////////////////////////////////
//	Infomation
////////////////////////////////////////////////

void LodNode::outputContext(ostream &printStream, char *indentString)
{
	SFVec3f *center = getCenterField();
	printStream << indentString << "\t" << "center " << center << endl;

	if (0 < getNRanges()) {
		MFFloat *range = getRangeField();
		printStream << indentString << "\t" << "range [" << endl;
		range->MField::outputContext(printStream, indentString, "\t\t");
		printStream << indentString << "\t" << "]" << endl;
	}
}

////////////////////////////////////////////////
//	LodNode::update
////////////////////////////////////////////////

void UpdateLod(LodNode *lod)
{
	int nNodes = lod->getNPrivateNodeElements();
	for (int n=0; n<nNodes; n++) {
		Node *node = lod->getPrivateNodeElementAt(n);
		node->remove();
	}

	SceneGraph *sg = lod->getSceneGraph();

	ViewpointNode *vpoint = sg->getViewpointNode();
	if (vpoint == NULL)
		vpoint = sg->getDefaultViewpointNode();

	if (vpoint) {
		SFMatrix	viewMatrix;
		float		viewPosition[3];
		vpoint->getTransformMatrix(&viewMatrix);
		vpoint->getPosition(viewPosition);
		viewMatrix.multi(viewPosition);

		SFMatrix	lodMatrix;
		float		lodCenter[3];
		lod->getTransformMatrix(&lodMatrix);
		lod->getCenter(lodCenter);
		lodMatrix.multi(lodCenter);

		float lx = lodCenter[0] - viewPosition[0];
		float ly = lodCenter[1] - viewPosition[1];
		float lz = lodCenter[2] - viewPosition[2];
		float distance = (float)sqrt(lx*lx + ly*ly + lz*lz);


		int numRange = lod->getNRanges();
		int nRange = 0;
		for (nRange=0; nRange<numRange; nRange++) {
			if (distance < lod->getRange(nRange))
				break;
		}

		Node *node = lod->getPrivateNodeElementAt(nRange);
		if (!node)
			node = lod->getPrivateNodeElementAt(lod->getNPrivateNodeElements() - 1);
		assert(node);
		lod->addChildNode(node);
	}
}

void LodNode::update()
{
	UpdateLod(this);
}

////////////////////////////////////////////////
//	LodNode::initialize
////////////////////////////////////////////////

void InitializeLod(LodNode *lod)
{
	lod->uninitialize();

	Node *node = lod->getChildNodes();
	while (node) {
		Node *nextNode = node->next();
//		node->remove();
		lod->addPrivateNodeElement(node);
		node = nextNode;
	}
/*
	Node *firstNode = lod->getPrivateNodeElementAt(0);
	if (firstNode)
		lod->addChildNode(firstNode);
*/
}

void LodNode::initialize()
{
	if (isInitialized() == false) {
		InitializeLod(this);
		setInitialized(true);
	}
}

////////////////////////////////////////////////
//	LodNode::uninitialize
////////////////////////////////////////////////

void UninitializeLod(LodNode *lod)
{
	int nNodes = lod->getNPrivateNodeElements();
	for (int n=0; n<nNodes; n++) {
		Node *node = lod->getPrivateNodeElementAt(n);
		node->remove();
		lod->addChildNode(node);
	}
	lod->removeAllNodeElement();
}

void LodNode::uninitialize()
{
	if (isInitialized() == true) {
		UninitializeLod(this);
		setInitialized(false);
	}
}

