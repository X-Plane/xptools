/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	DEF.h
*
******************************************************************/

#ifndef _CV97_DEF_H_
#define _CV97_DEF_H_

#include "String.h"
#include "LinkedListNode.h"

class DEF : public LinkedListNode<DEF> {

	String		mName;
	String		mString;

public:

	DEF (char *name, char *string);
	~DEF();

	////////////////////////////////////////////////
	//	Name
	////////////////////////////////////////////////

	void setName(char *name);
	char *getName();

	////////////////////////////////////////////////
	//	Name
	////////////////////////////////////////////////

	void setString(char *string);
	char *getString();
};

#endif


