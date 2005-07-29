/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFColor.cpp
*
******************************************************************/

#include "SFColor.h"

SFColor::SFColor() 
{
	setType(fieldTypeSFColor);
	setValue(1.0f, 1.0f, 1.0f);
	InitializeJavaIDs();
}

SFColor::SFColor(float r, float g, float b) 
{
	setType(fieldTypeSFColor);
	setValue(r, g, b);
	InitializeJavaIDs();
}

SFColor::SFColor(float value[]) 
{
	setType(fieldTypeSFColor);
	setValue(value);
	InitializeJavaIDs();
}

SFColor::SFColor(SFColor *color) 
{
	setType(fieldTypeSFColor);
	setValue(color);
	InitializeJavaIDs();
}

void SFColor::InitializeJavaIDs() 
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

////////////////////////////////////////////////
//	get value
////////////////////////////////////////////////

void SFColor::getValue(float value[]) 
{
	value[0] = mValue[0];
	value[1] = mValue[1];
	value[2] = mValue[2];
}

float *SFColor::getValue() 
{
	return mValue;
}

float SFColor::getRed() 
{
	return mValue[0];
}

float SFColor::getGreen() 
{
	return mValue[1];
}

float SFColor::getBlue() 
{
	return mValue[2];
}

////////////////////////////////////////////////
//	set value
////////////////////////////////////////////////

void SFColor::setValue(float r, float g, float b) 
{
	mValue[0] = r;
	mValue[1] = g;
	mValue[2] = b;
}

void SFColor::setValue(float value[]) 
{
	mValue[0] = value[0];
	mValue[1] = value[1];
	mValue[2] = value[2];
}

void SFColor::setValue(SFColor *color) 
{
	setValue(color->getRed(), color->getGreen(), color->getBlue());
}

////////////////////////////////////////////////
//	add value
////////////////////////////////////////////////

void SFColor::add(float x, float y, float z) 
{
	mValue[0] += x;
	mValue[1] += y;
	mValue[2] += z;
	mValue[0] /= 2.0f;
	mValue[1] /= 2.0f;
	mValue[2] /= 2.0f;
}

void SFColor::add(float value[]) 
{
	add(value[0], value[1], value[2]);
}

void SFColor::add(SFColor value) 
{
	add(value.getValue());
}

////////////////////////////////////////////////
//	sub value
////////////////////////////////////////////////

void SFColor::sub(float x, float y, float z) 
{
	mValue[0] -= x;
	mValue[1] -= y;
	mValue[2] -= z;
	mValue[0] /= 2.0f;
	mValue[1] /= 2.0f;
	mValue[2] /= 2.0f;
}

void SFColor::sub(float value[]) 
{
	sub(value[0], value[1], value[2]);
}

void SFColor::sub(SFColor value) 
{
	sub(value.getValue());
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

ostream& operator<<(ostream &s, SFColor &vector) 
{
	return s << vector.getRed() << " " << vector.getGreen() << " " << vector.getBlue();
}

ostream& operator<<(ostream &s, SFColor *vector) 
{
	return s << vector->getRed() << " " << vector->getGreen() << " " << vector->getBlue();
}

////////////////////////////////////////////////
//	String
////////////////////////////////////////////////

void SFColor::setValue(char *value) 
{
	if (!value)
		return;
	float	r, g, b;
	if (sscanf(value,"%f %f %f", &r, &g, &b) == 3) 
		setValue(r, g, b);
}

char *SFColor::getValue(char *buffer, int bufferLen) 
{
	sprintf(buffer, "%g %g %g", getRed(), getGreen(), getBlue());
	return buffer;
}

////////////////////////////////////////////////
//	scale
////////////////////////////////////////////////

void SFColor::scale(float scale) 
{
	mValue[0] *= scale;
	mValue[1] *= scale;
	mValue[2] *= scale;
}

////////////////////////////////////////////////
//	Compare
////////////////////////////////////////////////

bool SFColor::equals(Field *field) 
{
	SFColor *color = (SFColor *)field;
	if (getRed() == color->getRed() && getGreen() == color->getGreen() && getBlue() == color->getBlue())
		return true;
	else
		return false;
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			SFColor::mInit = 0;

jclass		SFColor::mFieldClassID = 0;
jclass		SFColor::mConstFieldClassID = 0;

jmethodID	SFColor::mInitMethodID = 0;
jmethodID	SFColor::mSetValueMethodID = 0;
jmethodID	SFColor::mGetRedMethodID = 0;
jmethodID	SFColor::mGetGreenMethodID = 0;
jmethodID	SFColor::mGetBlueMethodID = 0;
jmethodID	SFColor::mSetNameMethodID = 0;

jmethodID	SFColor::mConstInitMethodID = 0;
jmethodID	SFColor::mConstSetValueMethodID = 0;
jmethodID	SFColor::mConstGetRedMethodID = 0;
jmethodID	SFColor::mConstGetGreenMethodID = 0;
jmethodID	SFColor::mConstGetBlueMethodID = 0;
jmethodID	SFColor::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	SFColor::setJavaIDs
////////////////////////////////////////////////

void SFColor::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/SFColor");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstSFColor");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid = getFieldID();
		mInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(FFF)V");
		mGetRedMethodID		= jniEnv->GetMethodID(classid, "getRed", "()F");
		mGetGreenMethodID		= jniEnv->GetMethodID(classid, "getGreen", "()F");
		mGetBlueMethodID		= jniEnv->GetMethodID(classid, "getBlue", "()F");
		mSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(FFF)V");
		mSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mInitMethodID && mGetRedMethodID && mGetGreenMethodID && mGetBlueMethodID && mSetValueMethodID && mSetNameMethodID);

		// Const MethodIDs
		classid = getConstFieldID();
		mConstInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(FFF)V");
		mConstGetRedMethodID		= jniEnv->GetMethodID(classid, "getRed", "()F");
		mConstGetGreenMethodID		= jniEnv->GetMethodID(classid, "getGreen", "()F");
		mConstGetBlueMethodID		= jniEnv->GetMethodID(classid, "getBlue", "()F");
		mConstSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(FFF)V");
		mConstSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mConstInitMethodID && mConstGetRedMethodID && mConstGetGreenMethodID && mConstGetBlueMethodID && mConstSetValueMethodID && mConstSetNameMethodID);

		mInit = 1;
	}
}

////////////////////////////////////////////////
//	SFColor::toJavaObject
////////////////////////////////////////////////

jobject SFColor::toJavaObject(int bConstField) {
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	initMethod		= bConstField ? getConstInitMethodID() : getInitMethodID();
	jfloat		r				= getRed();
	jfloat		g				= getGreen();
	jfloat		b				= getBlue();
	jobject		eventField		= jniEnv->NewObject(classid, initMethod, r, g, b);
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
//	SFColor::setValue
////////////////////////////////////////////////

void SFColor::setValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getRedMethod	= bConstField ? getConstGetRedMethodID() : getGetRedMethodID();
	jmethodID	getGreenMethod	= bConstField ? getConstGetGreenMethodID() : getGetGreenMethodID();
	jmethodID	getBlueMethod	= bConstField ? getConstGetBlueMethodID() : getGetBlueMethodID();
	assert(classid && getRedMethod && getGreenMethod && getBlueMethod);
	jfloat		r				= jniEnv->CallFloatMethod(field, getRedMethod);
	jfloat		g				= jniEnv->CallFloatMethod(field, getGreenMethod);
	jfloat		b				= jniEnv->CallFloatMethod(field, getBlueMethod);
	setValue(r, g, b);
}

////////////////////////////////////////////////
//	SFColor::getValue
////////////////////////////////////////////////

void SFColor::getValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	setValueMethod	= bConstField ? getConstSetValueMethodID() : getSetValueMethodID();
	assert(classid && setValueMethod);
	jfloat		r				= getRed();
	jfloat		g				= getGreen();
	jfloat		b				= getBlue();
	jniEnv->CallVoidMethod(field, setValueMethod, r, g, b);
}

#endif
