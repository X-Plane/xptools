/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFInt32.cpp
*
******************************************************************/

#include "SFInt32.h"

SFInt32::SFInt32()
{
	setType(fieldTypeSFInt32);
	setValue(0);
	InitializeJavaIDs();
}

SFInt32::SFInt32(int value)
{
	setType(fieldTypeSFInt32);
	setValue(value);
	InitializeJavaIDs();
}

SFInt32::SFInt32(SFInt32 *value)
{
	setType(fieldTypeSFInt32);
	setValue(value);
	InitializeJavaIDs();
}

void SFInt32::InitializeJavaIDs()
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void SFInt32::setValue(int value)
{
	mValue = value;
}

void SFInt32::setValue(SFInt32 *value)
{
	mValue = value->getValue();
}

int SFInt32::getValue()
{
	return mValue;
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

ostream& operator<<(ostream &s, SFInt32 &value)
{
	return s << value.getValue();
}

ostream& operator<<(ostream &s, SFInt32 *value)
{
	return s << value->getValue();
}

////////////////////////////////////////////////
//	String
////////////////////////////////////////////////

void SFInt32::setValue(char *value)
{
	if (!value)
		return;
	setValue(atoi(value));
}

char *SFInt32::getValue(char *buffer, int bufferLen)
{
	sprintf(buffer, "%d", getValue());
	return buffer;
}

////////////////////////////////////////////////
//	Compare
////////////////////////////////////////////////

bool SFInt32::equals(Field *field)
{
	SFInt32 *intField = (SFInt32 *)field;
	if (getValue() == intField->getValue())
		return true;
	else
		return false;
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			SFInt32::mInit = 0;

jclass		SFInt32::mFieldClassID = 0;
jclass		SFInt32::mConstFieldClassID = 0;

jmethodID	SFInt32::mInitMethodID = 0;
jmethodID	SFInt32::mSetValueMethodID = 0;
jmethodID	SFInt32::mGetValueMethodID = 0;
jmethodID	SFInt32::mSetNameMethodID = 0;

jmethodID	SFInt32::mConstInitMethodID = 0;
jmethodID	SFInt32::mConstSetValueMethodID = 0;
jmethodID	SFInt32::mConstGetValueMethodID = 0;
jmethodID	SFInt32::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	SFInt32::setJavaIDs
////////////////////////////////////////////////

void SFInt32::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/SFInt32");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstSFInt32");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid = getFieldID();
		mInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(I)V");
		mGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()I");
		mSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(I)V");
		mSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mInitMethodID && mGetValueMethodID && mSetValueMethodID && mSetNameMethodID);

		// MethodIDs
		classid	 = getConstFieldID();
		mConstInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(I)V");
		mConstGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()I");
		mConstSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(I)V");
		mConstSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mConstInitMethodID && mConstGetValueMethodID && mConstSetValueMethodID && mConstSetNameMethodID);

		mInit = 1;
	}
}


////////////////////////////////////////////////
//	SFInt32::toJavaObject
////////////////////////////////////////////////

jobject SFInt32::toJavaObject(int bConstField) {
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	initMethod		= bConstField ? getConstInitMethodID() : getInitMethodID();
	jint		value			= getValue();
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
//	SFInt32::setValue
////////////////////////////////////////////////

void SFInt32::setValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getValueMethod	= bConstField ? getConstGetValueMethodID() : getGetValueMethodID();
	assert(classid && getValueMethod);
	jint		value			= jniEnv->CallIntMethod(field, getValueMethod);
	setValue(value);
}

////////////////////////////////////////////////
//	SFInt32::getValue
////////////////////////////////////////////////

void SFInt32::getValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	setValueMethod	= bConstField ? getConstSetValueMethodID() : getSetValueMethodID();
	assert(classid && setValueMethod);
	jint		value			= getValue();
	jniEnv->CallVoidMethod(field, setValueMethod, value);
}

#endif
