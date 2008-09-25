/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	VisibilitySensorNode.h
*
******************************************************************/

#ifndef _CV97_VISIBILITYSENSOR_H_
#define _CV97_VISIBILITYSENSOR_H_

#include "SensorNode.h"

class VisibilitySensorNode : public SensorNode {

	SFVec3f *centerField;
	SFVec3f *sizeField;
	SFTime *enterTimeField;
	SFTime *exitTimeField;

public:

	VisibilitySensorNode();
	~VisibilitySensorNode();

	////////////////////////////////////////////////
	//	Center
	////////////////////////////////////////////////

	SFVec3f *getCenterField();

	void	setCenter(float value[]);
	void	setCenter(float x, float y, float z);
	void	getCenter(float value[]);

	////////////////////////////////////////////////
	//	Size
	////////////////////////////////////////////////

	SFVec3f *getSizeField();

	void	setSize(float value[]);
	void	setSize(float x, float y, float z);
	void	getSize(float value[]);

	////////////////////////////////////////////////
	//	EnterTime
	////////////////////////////////////////////////

	SFTime *getEnterTimeField();

	void	setEnterTime(double value);
	double	getEnterTime();

	////////////////////////////////////////////////
	//	ExitTime
	////////////////////////////////////////////////

	SFTime *getExitTimeField();

	void	setExitTime(double value);
	double	getExitTime();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	VisibilitySensorNode *next();
	VisibilitySensorNode *nextTraversal();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////

	bool	isChildNodeType(Node *node);
	void	initialize();
	void	uninitialize();
	void	update();

	////////////////////////////////////////////////
	//	Infomation
	////////////////////////////////////////////////

	void	outputContext(ostream &printStream, char *indentString);
};

#endif

