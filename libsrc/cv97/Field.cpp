/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Field.cpp
*
******************************************************************/

#include <assert.h>
#include "VRMLField.h"

static char fieldTypeString[][16] = {
"None",
"SFBool",
"SFFloat",
"SFInt32",
"SFVec2f",
"SFVec3f",
"SFString",
"SFColor",
"SFTime",
"SFRotation",
"SFImage",
"SFNode",
"MFFloat",
"MFInt32",
"MFVec2f",
"MFVec3f",
"MFString",
"MFColor",
"MFTime",
"MFRotation",
"MFNode",
};

////////////////////////////////////////////////////////////
//	Field::getTypeName
////////////////////////////////////////////////////////////

char *Field::getTypeName() {
	if (0 < getType() && getType() < fieldTypeMaxNum)
		return fieldTypeString[getType()];
	else
		return NULL;
}

////////////////////////////////////////////////////////////
//	Field::setTypeName
////////////////////////////////////////////////////////////

void Field::setType(char *type) {

	if (!type || strlen(type) == 0) {
		setType(fieldTypeNone);
		return;
	}

	for (int n=1; n<fieldTypeMaxNum; n++) {
		if (strcmp(fieldTypeString[n], type) == 0) {
			setType(n);
			return;
		}
	}
	setType(fieldTypeNone);
}

////////////////////////////////////////////////////////////
//	Field::operator
////////////////////////////////////////////////////////////

ostream& operator<<(ostream &s, Field &value) {
	switch (value.getType()) {
	case fieldTypeSFBool		: return s << ((SFBool &)value);
	case fieldTypeSFColor		: return s << ((SFColor &)value);
	case fieldTypeSFFloat		: return s << ((SFFloat &)value);
	case fieldTypeSFInt32		: return s << ((SFInt32 &)value);
	case fieldTypeSFRotation	: return s << ((SFRotation &)value);
	case fieldTypeSFString		: return s << ((SFString &)value);
	case fieldTypeSFTime		: return s << ((SFTime &)value);
  	case fieldTypeSFVec2f		: return s << ((SFVec2f &)value);
  	case fieldTypeSFVec3f		: return s << ((SFVec3f &)value);
//	case fieldTypeSFNode		: return s << (SFNode &)value;
/*
	case fieldTypeMFColor		: return s << (MFColor &)value;
	case fieldTypeMFFloat		: return s << (MFFloat &)value;
	case fieldTypeMFInt32		: return s << (MFInt32 &)value;
	case fieldTypeMFRotation	: return s << (MFRotation &)value;
	case fieldTypeMFString		: return s << (MFString &)value;
	case fieldTypeMFTime		: return s << (MFTime &)value;
  	case fieldTypeMFVec2f		: return s << (MFVec2f &)value;
  	case fieldTypeMFVec3f		: return s << (MFVec3f &)value;
//	case fieldTypeMFNode		: return s << (MFNode &)value;
*/
	}
	return s;
}

ostream& operator<<(ostream &s, Field *value) {
	switch (value->getType()) {
	case fieldTypeSFBool		: return s << (SFBool *)value;
	case fieldTypeSFColor		: return s << (SFColor *)value;
	case fieldTypeSFFloat		: return s << (SFFloat *)value;
	case fieldTypeSFInt32		: return s << (SFInt32 *)value;
	case fieldTypeSFRotation	: return s << (SFRotation *)value;
	case fieldTypeSFString		: return s << (SFString *)value;
	case fieldTypeSFTime		: return s << (SFTime *)value;
 	case fieldTypeSFVec2f		: return s << (SFVec2f *)value;
  	case fieldTypeSFVec3f		: return s << (SFVec3f *)value;
//	  	case fieldTypeSFNode		: return s << (SFNode *)value;
	case fieldTypeMFColor		: return s << (MFColor *)value;
	case fieldTypeMFFloat		: return s << (MFFloat *)value;
	case fieldTypeMFInt32		: return s << (MFInt32 *)value;
	case fieldTypeMFRotation	: return s << (MFRotation *)value;
	case fieldTypeMFString		: return s << (MFString *)value;
	case fieldTypeMFTime		: return s << (MFTime *)value;
  	case fieldTypeMFVec2f		: return s << (MFVec2f *)value;
  	case fieldTypeMFVec3f		: return s << (MFVec3f *)value;
//	  	case fieldTypeMFNode		: return s << (MFNode *)value;
	}
	return s;
}

