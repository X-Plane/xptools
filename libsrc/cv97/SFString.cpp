/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFString.cpp
*
******************************************************************/

#include "SFString.h"

SFString::SFString() 
{
	setType(fieldTypeSFString);
	setValue((char *)NULL);
	InitializeJavaIDs();
}

SFString::SFString(char *value) 
{
	setType(fieldTypeSFString);
	setValue(value);
	InitializeJavaIDs();
}

SFString::SFString(SFString *value) 
{
	setType(fieldTypeSFString);
	setValue(value);
	InitializeJavaIDs();
}

void SFString::InitializeJavaIDs() 
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

SFString::~SFString() 
{
}

void SFString::setValue(char *value) 
{
	mValue.setValue(value);
}

void SFString::setValue(SFString *value) 
{
	mValue.setValue(value->getValue());
}

char *SFString::getValue() 
{
	return mValue.getValue();
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

ostream& operator<<(ostream &s, SFString &string) 
{
	if (string.getValue())
		return s << "\"" << string.getValue() << "\"";
	else
		return s << "\"" << "\"";
}

ostream& operator<<(ostream &s, SFString *string) 
{
	if (string->getValue())
		return s << "\"" << string->getValue() << "\"";
	else
		return s << "\"" << "\"";
}

////////////////////////////////////////////////
//	String
////////////////////////////////////////////////

char *SFString::getValue(char *buffer, int bufferLen) 
{
	sprintf(buffer, "%s", getValue());
	return buffer;
}

////////////////////////////////////////////////
//	Compare
////////////////////////////////////////////////

bool SFString::equals(Field *field) 
{
	SFString *stringField = (SFString *)field;
	if (!getValue() && !stringField->getValue())
		return true;
	if (getValue() && stringField->getValue())
		return (!strcmp(getValue(), stringField->getValue()) ? true : false);
	else
		return false;
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			SFString::mInit = 0;

jclass		SFString::mFieldClassID = 0;
jclass		SFString::mConstFieldClassID = 0;

jmethodID	SFString::mInitMethodID = 0;
jmethodID	SFString::mSetValueMethodID = 0;
jmethodID	SFString::mGetValueMethodID = 0;
jmethodID	SFString::mSetNameMethodID = 0;

jmethodID	SFString::mConstInitMethodID = 0;
jmethodID	SFString::mConstSetValueMethodID = 0;
jmethodID	SFString::mConstGetValueMethodID = 0;
jmethodID	SFString::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	SFString::setJavaIDs
////////////////////////////////////////////////

void SFString::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/SFString");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstSFString");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid = getFieldID();
		mInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(Ljava/lang/String;)V");
		mGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()Ljava/lang/String;");
		mSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(Ljava/lang/String;)V");
		mSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mInitMethodID && mGetValueMethodID && mSetValueMethodID && mSetNameMethodID);

		// Const MethodIDs
		classid	 = getConstFieldID();
		mConstInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(Ljava/lang/String;)V");
		mConstGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()Ljava/lang/String;");
		mConstSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(Ljava/lang/String;)V");
		mConstSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mConstInitMethodID && mConstGetValueMethodID && mConstSetValueMethodID && mConstSetNameMethodID);

		mInit = 1;
	}
}

////////////////////////////////////////////////
//	SFString::toJavaObject
////////////////////////////////////////////////

jobject SFString::toJavaObject(int bConstField) {
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	initMethod		= bConstField ? getConstInitMethodID() : getInitMethodID();
	jstring		value			= NULL;
	char		*string			= getValue();
		
	if (string && strlen(string))
		value = jniEnv->NewStringUTF(string);
	jobject		eventField		= jniEnv->NewObject(classid, initMethod, value);
	jmethodID	setNameMethod	= bConstField ? getConstSetNameMethodID() : getSetNameMethodID();
	if (value)
		jniEnv->ReleaseStringUTFChars(value, jniEnv->GetStringUTFChars(value, NULL));

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
//	SFString::setValue
////////////////////////////////////////////////

void SFString::setValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getValueMethod	= bConstField ? getConstGetValueMethodID() : getGetValueMethodID();
	assert(classid && getValueMethod);
	jstring		value			= (jstring)jniEnv->CallObjectMethod(field, getValueMethod);
	if (value) {
		const char	*string		= jniEnv->GetStringUTFChars(value, NULL);
		setValue((char *)string);
		jniEnv->ReleaseStringUTFChars(value, string);
		jniEnv->DeleteLocalRef(value);
	}
	else
		setValue((char *)NULL);

	jniEnv->DeleteLocalRef(classid);
}

////////////////////////////////////////////////
//	SFString::getValue
////////////////////////////////////////////////

void SFString::getValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	setValueMethod	= bConstField ? getConstSetValueMethodID() : getSetValueMethodID();
	assert(classid && setValueMethod);
	jstring		value			= NULL;
	char		*string			= getValue();
	if (string && strlen(string))
		value = jniEnv->NewStringUTF(string);
	jniEnv->CallVoidMethod(field, setValueMethod, value);
	if (value)
		jniEnv->DeleteLocalRef(value);
}

#endif
