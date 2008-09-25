/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Event.h
*
******************************************************************/

#ifndef _CV97_EVENT_H_
#define _CV97_EVENT_H_

#include <time.h>
#include "String.h"
#include "Field.h"
#include "SFTime.h"
#include "JavaVM.h"

#ifdef SUPPORT_JSAI
class Event : public JavaVM {
#else
class Event {
#endif

#ifdef SUPPORT_JSAI
	jclass		mEventClass;
#endif

	String		mName;
	double		mTime;
	Field		*mField;

public:

	Event(Field *field);
	Event(char *name, double time, Field *field);

	void InitializeJavaIDs();

	////////////////////////////////////////////////
	//	Name
	////////////////////////////////////////////////

	void setName(char *name);
	char *getName();

	////////////////////////////////////////////////
	//	Time
	////////////////////////////////////////////////

	void setTimeStamp(double time);
	double getTimeStamp();

	////////////////////////////////////////////////
	//	ConstField
	////////////////////////////////////////////////

	void setField(Field *field);
	Field *getField();

	////////////////////////////////////////////////
	//	for Java
	////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

	void setEventClass(jclass eventClass);
	jclass getEventClass();
	jobject toJavaObject();

#endif

};

#endif
