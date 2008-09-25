/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFFloat.cpp
*
******************************************************************/

#include "SFFloat.h"

SFFloat::SFFloat()
{
	setType(fieldTypeSFFloat);
	setValue(0.0f);
	InitializeJavaIDs();
}

SFFloat::SFFloat(float value)
{
	setType(fieldTypeSFFloat);
	setValue(value);
	InitializeJavaIDs();
}

SFFloat::SFFloat(SFFloat *value)
{
	setType(fieldTypeSFFloat);
	setValue(value);
	InitializeJavaIDs();
}

void SFFloat::InitializeJavaIDs()
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void SFFloat::setValue(float value)
{
	mValue = value;
}

void SFFloat::setValue(SFFloat *fvalue)
{
	mValue = fvalue->getValue();
}

float SFFloat::getValue()
{
	return mValue;
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

ostream& operator<<(ostream &s, SFFloat &value)
{
	return s << value.getValue();
}

ostream& operator<<(ostream &s, SFFloat *value)
{
	return s << value->getValue();
}

////////////////////////////////////////////////
//	String
////////////////////////////////////////////////

void SFFloat::setValue(char *value)
{
	if (!value)
		return;
	setValue((float)atof(value));
}

char *SFFloat::getValue(char *buffer, int bufferLen)
{
	sprintf(buffer, "%g", getValue());
	return buffer;
}

////////////////////////////////////////////////
//	Compare
////////////////////////////////////////////////

bool SFFloat::equals(Field *field)
{
	SFFloat *floatField = (SFFloat *)field;
	if (getValue() == floatField->getValue())
		return true;
	else
		return false;
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			SFFloat::mInit = 0;

jclass		SFFloat::mFieldClassID = 0;
jclass		SFFloat::mConstFieldClassID = 0;

jmethodID	SFFloat::mInitMethodID = 0;
jmethodID	SFFloat::mSetValueMethodID = 0;
jmethodID	SFFloat::mGetValueMethodID = 0;
jmethodID	SFFloat::mSetNameMethodID = 0;

jmethodID	SFFloat::mConstInitMethodID = 0;
jmethodID	SFFloat::mConstSetValueMethodID = 0;
jmethodID	SFFloat::mConstGetValueMethodID = 0;
jmethodID	SFFloat::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	SFFloat::toJavaObject
////////////////////////////////////////////////

void SFFloat::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/SFFloat");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstSFFloat");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid = getFieldID();
		mInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(F)V");
		mGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()F");
		mSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(F)V");
		mSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mInitMethodID && mGetValueMethodID && mSetValueMethodID && mSetNameMethodID);

		// MethodIDs
		classid	 = getConstFieldID();
		mConstInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(F)V");
		mConstGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()F");
		mConstSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(F)V");
		mConstSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mConstInitMethodID && mConstGetValueMethodID && mConstSetValueMethodID && mConstSetNameMethodID);

		mInit = 1;
	}
}

////////////////////////////////////////////////
//	SFFloat::toJavaObject
////////////////////////////////////////////////

jobject SFFloat::toJavaObject(int bConstField) {
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	initMethod		= bConstField ? getConstInitMethodID() : getInitMethodID();
	jfloat		value			= getValue();
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
//	SFFloat::setValue
////////////////////////////////////////////////

void SFFloat::setValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getValueMethod	= bConstField ? getConstGetValueMethodID() : getGetValueMethodID();
	assert(classid && getValueMethod);
	jfloat		value			= jniEnv->CallFloatMethod(field, getValueMethod);
	setValue(value);
}

////////////////////////////////////////////////
//	SFFloat::getValue
////////////////////////////////////////////////

void SFFloat::getValue(jobject field, int bConstField){
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	setValueMethod	= bConstField ? getConstSetValueMethodID() : getSetValueMethodID();
	assert(classid && setValueMethod);
	jfloat		value			= getValue();
	jniEnv->CallVoidMethod(field, setValueMethod, value);
}

#endif
