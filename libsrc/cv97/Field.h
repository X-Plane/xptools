/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Field.h
*
******************************************************************/

#ifndef _CV97_FIELD_H_
#define _CV97_FIELD_H_

#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <assert.h>
#include "String.h"
#include "JavaVM.h"

#ifdef SUPPORT_OLDCPP
#include "OldCpp.h"
#endif

enum {
fieldTypeNone,
fieldTypeSFBool,
fieldTypeSFFloat,
fieldTypeSFInt32,
fieldTypeSFVec2f,
fieldTypeSFVec3f,
fieldTypeSFString,
fieldTypeSFColor,
fieldTypeSFTime,
fieldTypeSFRotation,
fieldTypeSFImage,
fieldTypeSFNode,
fieldTypeMFFloat,
fieldTypeMFInt32,
fieldTypeMFVec2f,
fieldTypeMFVec3f,
fieldTypeMFString,
fieldTypeMFColor,
fieldTypeMFTime,
fieldTypeMFRotation,
fieldTypeMFNode,
fieldTypeMaxNum,
};

class	SFBool;
class	SFFloat;
class	SFInt32;
class	SFVec2f;
class	SFVec3f;
class	SFString;
class	SFColor;
class	SFTime;
class	SFRotation;
//class	SFNode;
class	MFFloat;
class	MFInt32;
class	MFVec2f;
class	MFVec3f;
class	MFString;
class	MFColor;
class	MFTime;
class	MFRotation;
//class	MFNode;

#define	eventInStripString		"set_"
#define eventOutStripString		"_changed"

#define JAVAOBJECT_FIELD		0
#define JAVAOBJECT_CONSTFIELD	1

#define FIELD_BUFFERSIZE		1024

#ifdef SUPPORT_JSAI
class Field : public CJavaVM {
#else
class Field {
#endif

	String	mName;
	int		mType;

public:

	Field() {
		mType = fieldTypeNone;
	}	

	virtual ~Field() {
	}	

	char *getTypeName();

	void setType(int type) {
		mType = type;
	}

	void setType(char *type);

	int getType() {
		return mType;
	}

	void setName(char *name) {
		mName.setValue(name);
	}

	char *getName() {
		return mName.getValue();
	}

	friend ostream& operator<<(ostream &s, Field &value);
	friend ostream& operator<<(ostream &s, Field *value);

	////////////////////////////////////////////////
	//	String
	////////////////////////////////////////////////

	virtual void setValue(char *value){
	}

	virtual char *getValue(char *buffer, int bufferLen = -1){
		buffer[0] = '\0';
		return buffer;
	}

	////////////////////////////////////////////////
	//	Compare
	////////////////////////////////////////////////

	virtual bool equals(Field *field) {
		return false;
	}

	////////////////////////////////////////////////
	//	Java
	////////////////////////////////////////////////

#ifdef SUPPORT_JSAI
	virtual jobject toJavaObject(int bConstField = 0) {
		assert(0);
		return NULL;
	};
	virtual void setValue(jobject field, int bConstField = 0) {
		assert(0);
	};
	virtual void getValue(jobject field, int bConstField = 0) {
		assert(0);
	};
#endif
};

#endif
