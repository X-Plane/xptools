/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	WorldInfoNode.h
*
******************************************************************/

#ifndef _CV97_WORLDINFO_H_
#define _CV97_WORLDINFO_H_

#include "VRMLField.h"
#include "Node.h"

class WorldInfoNode : public Node {

	SFString *titleField;
	MFString *infoField;

public:

	WorldInfoNode();
	~WorldInfoNode();

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	WorldInfoNode *next();
	WorldInfoNode *nextTraversal();

	////////////////////////////////////////////////
	//	Title
	////////////////////////////////////////////////

	SFString *getTitleField();

	void setTitle(char *value);
	char *getTitle();

	////////////////////////////////////////////////
	// Info
	////////////////////////////////////////////////

	MFString *getInfoField();

	void addInfo(char *value);
	int getNInfos();
	char *getInfo(int index);

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

	void outputContext(ostream& printStream, char *indentString);
};

#endif
