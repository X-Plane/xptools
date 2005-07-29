/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFBool.cpp
*
******************************************************************/

#include "SFBool.h"

SFBool::SFBool() 
{
	setType(fieldTypeSFBool);
	setValue(true);
	InitializeJavaIDs();
}

SFBool::SFBool(bool value) 
{
	setType(fieldTypeSFBool);
	setValue(value);
	InitializeJavaIDs();
}

SFBool::SFBool(int value) 
{
	setType(fieldTypeSFBool);
	setValue(value);
	InitializeJavaIDs();
}

SFBool::SFBool(SFBool *value) 
{
	setType(fieldTypeSFBool);
	setValue(value);
	InitializeJavaIDs();
}

void SFBool::InitializeJavaIDs() 
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void SFBool::setValue(bool value) 
{
	mValue = value;
}

void SFBool::setValue(int value) 
{
	setValue(value ? true : false);
}

void SFBool::setValue(SFBool *value) 
{
	mValue = value->getValue();
}

bool SFBool::getValue() 
{
	return mValue;
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

ostream& operator<<(ostream &s, SFBool &value) {
	return s << (value.getValue() ? "TRUE" : "FALSE");
}

ostream& operator<<(ostream &s, SFBool *value) {
	return s << (value->getValue() ? "TRUE" : "FALSE");
}

////////////////////////////////////////////////
//	String
////////////////////////////////////////////////

void SFBool::setValue(char *value) 
{
	if (!value)
		return;
	if (!strcmp(value, "TRUE"))
		setValue(true);
	else if (!strcmp(value, "FALSE"))
		setValue(false);
}

char *SFBool::getValue(char *buffer, int bufferLen) 
{
	sprintf(buffer, "%s", (getValue() ? "TRUE" : "FALSE"));
	return buffer;
}

////////////////////////////////////////////////
//	Compare
////////////////////////////////////////////////

bool SFBool::equals(Field *field) 
{
	SFBool *boolField = (SFBool *)field;
	if (getValue() == boolField->getValue())
		return true;
	else
		return false;
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			SFBool::mInit = 0;

jclass		SFBool::mFieldClassID = 0;
jclass		SFBool::mConstFieldClassID = 0;

jmethodID	SFBool::mInitMethodID = 0;
jmethodID	SFBool::mSetValueMethodID = 0;
jmethodID	SFBool::mGetValueMethodID = 0;
jmethodID	SFBool::mSetNameMethodID = 0;

jmethodID	SFBool::mConstInitMethodID = 0;
jmethodID	SFBool::mConstSetValueMethodID = 0;
jmethodID	SFBool::mConstGetValueMethodID = 0;
jmethodID	SFBool::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	SFBool::toJavaObject
////////////////////////////////////////////////

void SFBool::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/SFBool");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstSFBool");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid = getFieldID();
		mInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(Z)V");
		mGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()Z");
		mSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(Z)V");
		mSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mInitMethodID && mGetValueMethodID && mSetValueMethodID && mSetNameMethodID);

		// MethodIDs
		classid	 = getConstFieldID();
		mConstInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(Z)V");
		mConstGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()Z");
		mConstSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(Z)V");
		mConstSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mConstInitMethodID && mConstGetValueMethodID && mConstSetValueMethodID && mConstSetNameMethodID);

		mInit = 1;
	}
}

////////////////////////////////////////////////
//	SFBool::toJavaObject
////////////////////////////////////////////////

jobject SFBool::toJavaObject(int bConstField) {
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	initMethod		= bConstField ? getConstInitMethodID() : getInitMethodID();
	jboolean	value			= getValue();
	jobject		eventField		= jniEnv->NewObject(classid, initMethod, value);
	jmethodID	setNameMethod	= bConstField ? getConstSetNameMethodID() : getSetNameMethodID();
		
	char		*fieldName		= getName();
	jstring		jfieldName		= NULL;
	if (fieldName && strlen(fieldName))
		jfieldName = jniEnv->NewStringUTF(getName());
	jniEnv->CallVoidMethod(eventField, setNameMethod, jfieldName);
	if (jfieldName)
		jniEnv->DeleteLocalRef(jfieldName);
	
	return eventField;
}

////////////////////////////////////////////////
//	SFBool::setValue
////////////////////////////////////////////////

void SFBool::setValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getValueMethod	= bConstField ? getConstGetValueMethodID() : getGetValueMethodID();
	assert(classid && getValueMethod);
	jboolean	value			= jniEnv->CallBooleanMethod(field, getValueMethod);
	setValue(value ? true : false);
}

////////////////////////////////////////////////
//	SFBool::getValue
////////////////////////////////////////////////

void SFBool::getValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	setValueMethod	= bConstField ? getConstSetValueMethodID() : getSetValueMethodID();
	assert(classid && setValueMethod);
	jboolean	value			= getValue();
	jniEnv->CallVoidMethod(field, setValueMethod, value);
}

#endif
