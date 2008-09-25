/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	DEF.cpp
*
******************************************************************/

#include "DEF.h"

DEF::DEF(char *name, char *string)
{
	setName(name);
	setString(string);
}

DEF::~DEF()
{
	remove();
}

////////////////////////////////////////////////
//	Name
////////////////////////////////////////////////

void DEF::setName(char *name)
{
	mName.setValue(name);
}

char *DEF::getName()
{
	return mName.getValue();
}

////////////////////////////////////////////////
//	String
////////////////////////////////////////////////

void DEF::setString(char *string)
{
	mString.setValue(string);
}

char *DEF::getString()
{
	return mString.getValue();
}
