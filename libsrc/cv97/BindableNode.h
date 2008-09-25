/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	BindableNode.h
*
******************************************************************/

#ifndef _CV97_BINDABLENODE_H_
#define _CV97_BINDABLENODE_H_

#include "VRMLField.h"
#include "Node.h"

class BindableNode : public Node {

	SFBool *setBindField;
	SFTime *bindTimeField;
	SFBool *isBoundField;

public:

	BindableNode();
	virtual ~BindableNode();

	////////////////////////////////////////////////
	//	bind
	////////////////////////////////////////////////

	SFBool *getBindField();

	void setBind(bool value);
	bool  getBind();
	bool  isBind();

	////////////////////////////////////////////////
	//	bindTime
	////////////////////////////////////////////////

	SFTime *getBindTimeField();

	void setBindTime(double value);
	double getBindTime();

	////////////////////////////////////////////////
	//	isBound
	////////////////////////////////////////////////

	SFBool *getIsBoundField();

	void setIsBound(bool  value);
	bool  getIsBound();
	bool  isBound();
};

#endif

