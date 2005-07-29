/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SwitchNode.h
*
******************************************************************/

#ifndef _CV97_SWITCH_H_
#define _CV97_SWITCH_H_

#include "VRMLField.h"
#include "Node.h"

class SwitchNode : public Node {

	SFInt32 *whichChoiceField;

public:

	SwitchNode();
	~SwitchNode();

	////////////////////////////////////////////////
	//	whichChoice
	////////////////////////////////////////////////

	SFInt32 *getWhichChoiceField();

	void setWhichChoice(int value);
	int getWhichChoice();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	SwitchNode *next();
	SwitchNode *nextTraversal();

	////////////////////////////////////////////////
	//	functions
	////////////////////////////////////////////////
	
	bool isChildNodeType(Node *node);
	void initialize();
	void uninitialize();
	void update();

	////////////////////////////////////////////////
	//	Infomation
	////////////////////////////////////////////////

	void outputContext(ostream &printStream, char *indentString);
};

#endif

