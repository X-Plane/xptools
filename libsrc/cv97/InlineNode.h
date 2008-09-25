/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	InlineNode.h
*
******************************************************************/

#ifndef _CV97_INLINE_H_
#define _CV97_INLINE_H_

#include "GroupingNode.h"

class InlineNode : public GroupingNode {

	MFString *urlField;

public:

	InlineNode();
	~InlineNode();

	////////////////////////////////////////////////
	// Url
	////////////////////////////////////////////////

	MFString *getUrlField();

	void addUrl(char *value);
	int getNUrls();
	char *getUrl(int index);
	void setUrl(int index, char *urlString);

	////////////////////////////////////////////////
	//	List
	////////////////////////////////////////////////

	InlineNode *next();
	InlineNode *nextTraversal();

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

