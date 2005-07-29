/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	BillboardNode.h
*
******************************************************************/

#ifndef _CV97_BILLBOARD_H_
#define _CV97_BILLBOARD_H_

#include "VRMLField.h"
#include "Node.h"
	
class BillboardNode : public GroupingNode {

	SFVec3f *axisOfRotationField;

public:

	BillboardNode();
	~BillboardNode();

	////////////////////////////////////////////////
	//	axisOfRotation
	////////////////////////////////////////////////

	SFVec3f *getAxisOfRotationField();

	void setAxisOfRotation(float value[]);
	void setAxisOfRotation(float x, float y, float z);
	void getAxisOfRotation(float value[]);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	BillboardNode *next();
	BillboardNode *nextTraversal();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////
	
	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	actions
	////////////////////////////////////////////////

	void	getBillboardToViewerVector(float vector[3]);
	void	getViewerToBillboardVector(float vector[3]);
	void	getPlaneVectorOfAxisOfRotationAndBillboardToViewer(float vector[3]);
	void	getZAxisVectorOnPlaneOfAxisOfRotationAndBillboardToViewer(float vector[3]);
	float	getRotationAngleOfZAxis();
	void	getRotationZAxisRotation(float rotation[4]);
	void	getSFMatrix(SFMatrix *mOut);

	////////////////////////////////////////////////
	//	Infomation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif

