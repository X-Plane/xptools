/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFVec2f.cpp
*
******************************************************************/

#include "SFVec2f.h"

SFVec2f::SFVec2f()
{
	setType(fieldTypeSFVec2f);
	setValue(0.0f, 0.0f);
	InitializeJavaIDs();
}

SFVec2f::SFVec2f(float x, float y)
{
	setType(fieldTypeSFVec2f);
	setValue(x, y);
	InitializeJavaIDs();
}

SFVec2f::SFVec2f(float value[])
{
	setType(fieldTypeSFVec2f);
	setValue(value);
	InitializeJavaIDs();
}

SFVec2f::SFVec2f(SFVec2f *value)
{
	setType(fieldTypeSFVec2f);
	setValue(value);
	InitializeJavaIDs();
}

void SFVec2f::InitializeJavaIDs()
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

////////////////////////////////////////////////
//	get value
////////////////////////////////////////////////

void SFVec2f::getValue(float value[])
{
	value[0] = mValue[0];
	value[1] = mValue[1];
}

float *SFVec2f::getValue()
{
	return mValue;
}

float SFVec2f::getX()
{
	return mValue[0];
}

float SFVec2f::getY()
{
	return mValue[1];
}

////////////////////////////////////////////////
//	set value
////////////////////////////////////////////////

void SFVec2f::setValue(float x, float y)
{
	mValue[0] = x;
	mValue[1] = y;
}

void SFVec2f::setValue(float value[])
{
	mValue[0] = value[0];
	mValue[1] = value[1];
}

void SFVec2f::setValue(SFVec2f *vector)
{
	setValue(vector->getX(), vector->getY());
}

void SFVec2f::setX(float x)
{
	setValue(x, getY());
}

void SFVec2f::setY(float y)
{
	setValue(getX(), y);
}

////////////////////////////////////////////////
//	add value
////////////////////////////////////////////////

void SFVec2f::add(float x, float y)
{
	mValue[0] += x;
	mValue[1] += y;
}

void SFVec2f::add(float value[])
{
	mValue[0] += value[0];
	mValue[1] += value[1];
}

void SFVec2f::add(SFVec2f value)
{
	add(value.getValue());
}

void SFVec2f::translate(float x, float y)
{
	add(x, y);
}

void SFVec2f::translate(float value[])
{
	add(value);
}

void SFVec2f::translate(SFVec2f value)
{
	add(value);
}

////////////////////////////////////////////////
//	sub value
////////////////////////////////////////////////

void SFVec2f::sub(float x, float y)
{
	mValue[0] -= x;
	mValue[1] -= y;
}

void SFVec2f::sub(float value[])
{
	mValue[0] -= value[0];
	mValue[1] -= value[1];
}

void SFVec2f::sub(SFVec2f value)
{
	sub(value.getValue());
}

////////////////////////////////////////////////
//	scale
////////////////////////////////////////////////

void SFVec2f::scale(float value)
{
	mValue[0] *= value;
	mValue[1] *= value;
}

void SFVec2f::scale(float xscale, float yscale)
{
	mValue[0] *= xscale;
	mValue[1] *= yscale;
}

void SFVec2f::scale(float value[2])
{
	scale(value[0], value[1]);
}

////////////////////////////////////////////////
//	invert
////////////////////////////////////////////////

void SFVec2f::invert()
{
	mValue[0] = -mValue[0];
	mValue[1] = -mValue[1];
}

////////////////////////////////////////////////
//	scalar
////////////////////////////////////////////////

float SFVec2f::getScalar()
{
	return (float)sqrt(mValue[0]*mValue[0]+mValue[1]*mValue[1]);
}

////////////////////////////////////////////////
//	normalize
////////////////////////////////////////////////

void SFVec2f::normalize()
{
	float scale = getScalar();
	if (scale != 0.0f) {
		mValue[0] /= scale;
		mValue[1] /= scale;
	}
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

ostream& operator<<(ostream &s, SFVec2f &vector)
{
	return s << vector.getX() << " " << vector.getY();
}

ostream& operator<<(ostream &s, SFVec2f *vector)
{
	return s << vector->getX() << " " << vector->getY();
}

////////////////////////////////////////////////
//	String
////////////////////////////////////////////////

void SFVec2f::setValue(char *value)
{
	if (!value)
		return;
	float	x, y;
	if (sscanf(value,"%f %f", &x, &y) == 2)
		setValue(x, y);
}

char *SFVec2f::getValue(char *buffer, int bufferLen)
{
	sprintf(buffer, "%g %g", getX(), getY());
	return buffer;
}

////////////////////////////////////////////////
//	Compare
////////////////////////////////////////////////

bool SFVec2f::equals(Field *field)
{
	SFVec2f *vector = (SFVec2f *)field;
	if (getX() == vector->getX() && getY() == vector->getY())
		return true;
	else
		return false;
}

bool SFVec2f::equals(float value[2])
{
	SFVec2f vector(value);
	return equals(&vector);
}

bool SFVec2f::equals(float x, float y)
{
	SFVec2f vector(x, y);
	return equals(&vector);
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			SFVec2f::mInit = 0;

jclass		SFVec2f::mFieldClassID = 0;
jclass		SFVec2f::mConstFieldClassID = 0;

jmethodID	SFVec2f::mInitMethodID = 0;
jmethodID	SFVec2f::mSetValueMethodID = 0;
jmethodID	SFVec2f::mGetXMethodID = 0;
jmethodID	SFVec2f::mGetYMethodID = 0;
jmethodID	SFVec2f::mSetNameMethodID = 0;

jmethodID	SFVec2f::mConstInitMethodID = 0;
jmethodID	SFVec2f::mConstSetValueMethodID = 0;
jmethodID	SFVec2f::mConstGetXMethodID = 0;
jmethodID	SFVec2f::mConstGetYMethodID = 0;
jmethodID	SFVec2f::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	SFVec2f::setJavaIDs
////////////////////////////////////////////////

void SFVec2f::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/SFVec2f");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstSFVec2f");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid = getFieldID();
		mInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(FF)V");
		mGetXMethodID		= jniEnv->GetMethodID(classid, "getX", "()F");
		mGetYMethodID		= jniEnv->GetMethodID(classid, "getY", "()F");
		mSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(FF)V");
		mSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mInitMethodID && mGetXMethodID && mGetYMethodID && mSetValueMethodID && mSetNameMethodID);

		// Const MethodIDs
		classid = getConstFieldID();
		mConstInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(FF)V");
		mConstGetXMethodID		= jniEnv->GetMethodID(classid, "getX", "()F");
		mConstGetYMethodID		= jniEnv->GetMethodID(classid, "getY", "()F");
		mConstSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(FF)V");
		mConstSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mConstInitMethodID && mConstGetXMethodID && mConstGetYMethodID && mConstSetValueMethodID && mConstSetNameMethodID);

		mInit = 1;
	}
}

////////////////////////////////////////////////
//	SFVec2f::toJavaObject
////////////////////////////////////////////////

jobject SFVec2f::toJavaObject(int bConstField) {
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	initMethod		= bConstField ? getConstInitMethodID() : getInitMethodID();
	jfloat		x				= getX();
	jfloat		y				= getY();
	jobject		eventField		= jniEnv->NewObject(classid, initMethod, x, y);
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
//	SFVec2f::setValue
////////////////////////////////////////////////

void SFVec2f::setValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getXMethod		= bConstField ? getConstGetXMethodID() : getGetXMethodID();
	jmethodID	getYMethod		= bConstField ? getConstGetYMethodID() : getGetYMethodID();
	assert(classid && getXMethod && getYMethod);
	jfloat		x				= jniEnv->CallFloatMethod(field, getXMethod);
	jfloat		y				= jniEnv->CallFloatMethod(field, getYMethod);
	setValue(x, y);
}

////////////////////////////////////////////////
//	SFVec2f::getValue
////////////////////////////////////////////////

void SFVec2f::getValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	setValueMethod	= bConstField ? getConstSetValueMethodID() : getSetValueMethodID();
	assert(classid && setValueMethod);
	jfloat		x				= getX();
	jfloat		y				= getY();
	jniEnv->CallVoidMethod(field, setValueMethod, x, y);
}

#endif
