/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFVec3f.cpp
*
******************************************************************/

#include "SFVec3f.h"
#include "SFRotation.h"

SFVec3f::SFVec3f() 
{
	setType(fieldTypeSFVec3f);
	setValue(0.0f, 0.0f, 0.0f);
	InitializeJavaIDs();
}

SFVec3f::SFVec3f(float x, float y, float z) 
{
	setType(fieldTypeSFVec3f);
	setValue(x, y, z);
	InitializeJavaIDs();
}

SFVec3f::SFVec3f(float value[]) 
{
	setType(fieldTypeSFVec3f);
	setValue(value);
	InitializeJavaIDs();
}

SFVec3f::SFVec3f(SFVec3f *value) 
{
	setType(fieldTypeSFVec3f);
	setValue(value);
	InitializeJavaIDs();
}

void SFVec3f::InitializeJavaIDs() 
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

////////////////////////////////////////////////
//	get value
////////////////////////////////////////////////

void SFVec3f::getValue(float value[]) 
{
	value[0] = mValue[0];
	value[1] = mValue[1];
	value[2] = mValue[2];
}

float *SFVec3f::getValue() 
{
	return mValue;
}

float SFVec3f::getX() 
{
	return mValue[0];
}

float SFVec3f::getY() 
{
	return mValue[1];
}

float SFVec3f::getZ() 
{
	return mValue[2];
}

////////////////////////////////////////////////
//	set value
////////////////////////////////////////////////

void SFVec3f::setValue(float x, float y, float z) 
{
	mValue[0] = x;
	mValue[1] = y;
	mValue[2] = z;
}

void SFVec3f::setValue(float value[]) 
{
	mValue[0] = value[0];
	mValue[1] = value[1];
	mValue[2] = value[2];
}

void SFVec3f::setValue(SFVec3f *vector) 
{
	setValue(vector->getX(), vector->getY(), vector->getZ());
}

void SFVec3f::setX(float x) 
{
	setValue(x, getY(), getZ());
}

void SFVec3f::setY(float y) 
{
	setValue(getX(), y, getZ());
}

void SFVec3f::setZ(float z) 
{
	setValue(getX(), getY(), z);
}

////////////////////////////////////////////////
//	add value
////////////////////////////////////////////////

void SFVec3f::add(float x, float y, float z) 
{
	mValue[0] += x;
	mValue[1] += y;
	mValue[2] += z;
}

void SFVec3f::add(float value[]) 
{
	mValue[0] += value[0];
	mValue[1] += value[1];
	mValue[2] += value[2];
}

void SFVec3f::add(SFVec3f value) 
{
	add(value.getValue());
}

void SFVec3f::translate(float x, float y, float z) 
{
	add(x, y, z);
}

void SFVec3f::translate(float value[]) 
{
	add(value);
}

void SFVec3f::translate(SFVec3f value) 
{
	add(value);
}

////////////////////////////////////////////////
//	sub value
////////////////////////////////////////////////

void SFVec3f::sub(float x, float y, float z) 
{
	mValue[0] -= x;
	mValue[1] -= y;
	mValue[2] -= z;
}

void SFVec3f::sub(float value[]) 
{
	mValue[0] -= value[0];
	mValue[1] -= value[1];
	mValue[2] -= value[2];
}

void SFVec3f::sub(SFVec3f value) 
{
	sub(value.getValue());
}

////////////////////////////////////////////////
//	scale
////////////////////////////////////////////////

void SFVec3f::scale(float value) 
{
	mValue[0] *= value;
	mValue[1] *= value;
	mValue[2] *= value;
}

void SFVec3f::scale(float xscale, float yscale, float zscale) 
{
	mValue[0] *= xscale;
	mValue[1] *= yscale;
	mValue[2] *= zscale;
}

void SFVec3f::scale(float value[3]) 
{
	scale(value[0], value[1], value[2]);
}

////////////////////////////////////////////////
//	rotate
////////////////////////////////////////////////

void SFVec3f::rotate(SFRotation *rotation) 
{
	rotation->multi(mValue);
}

void SFVec3f::rotate(float x, float y, float z, float angle) 
{
	SFRotation rotation(x, y, z, angle);
	rotate(&rotation);
}

void SFVec3f::rotate(float value[3]) 
{
	rotate(value[0], value[1], value[2], value[3]);
}

////////////////////////////////////////////////
//	invert
////////////////////////////////////////////////

void SFVec3f::invert() 
{
	mValue[0] = -mValue[0];
	mValue[1] = -mValue[1];
	mValue[2] = -mValue[2];
}

////////////////////////////////////////////////
//	scalar
////////////////////////////////////////////////

float SFVec3f::getScalar()
{
	return (float)sqrt(mValue[0]*mValue[0]+mValue[1]*mValue[1]+mValue[2]*mValue[2]);
}

////////////////////////////////////////////////
//	normalize
////////////////////////////////////////////////

void SFVec3f::normalize()
{
	float scale = getScalar();
	if (scale != 0.0f) {
		mValue[0] /= scale;
		mValue[1] /= scale;
		mValue[2] /= scale;
	}
}

////////////////////////////////////////////////
//	String
////////////////////////////////////////////////

void SFVec3f::setValue(char *value) 
{
	if (!value)
		return;
	float	x, y, z;
	if (sscanf(value,"%f %f %f", &x, &y, &z) == 3) 
		setValue(x, y, z);
}

char *SFVec3f::getValue(char *buffer, int bufferLen) 
{
	sprintf(buffer, "%g %g %g", getX(), getY(), getZ());
	return buffer;
}

////////////////////////////////////////////////
//	Compare
////////////////////////////////////////////////

bool SFVec3f::equals(Field *field) 
{
	SFVec3f *vector = (SFVec3f *)field;
	if (getX() == vector->getX() && getY() == vector->getY() && getZ() == vector->getZ())
		return true;
	else
		return false;
}

bool SFVec3f::equals(float value[3]) 
{
	SFVec3f vector(value);
	return equals(&vector);
}

bool SFVec3f::equals(float x, float y, float z) 
{
	SFVec3f vector(x, y, z);
	return equals(&vector);
}


////////////////////////////////////////////////
//	Overload
////////////////////////////////////////////////

ostream& operator<<(ostream &s, SFVec3f &vector) 
{
	return s << vector.getX() << " " << vector.getY() << " " << vector.getZ();
}

ostream& operator<<(ostream &s, SFVec3f *vector) 
{
	return s << vector->getX() << " " << vector->getY() << " " << vector->getZ();
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			SFVec3f::mInit = 0;

jclass		SFVec3f::mFieldClassID = 0;
jclass		SFVec3f::mConstFieldClassID = 0;

jmethodID	SFVec3f::mInitMethodID = 0;
jmethodID	SFVec3f::mSetValueMethodID = 0;
jmethodID	SFVec3f::mGetXMethodID = 0;
jmethodID	SFVec3f::mGetYMethodID = 0;
jmethodID	SFVec3f::mGetZMethodID = 0;
jmethodID	SFVec3f::mSetNameMethodID = 0;

jmethodID	SFVec3f::mConstInitMethodID = 0;
jmethodID	SFVec3f::mConstSetValueMethodID = 0;
jmethodID	SFVec3f::mConstGetXMethodID = 0;
jmethodID	SFVec3f::mConstGetYMethodID = 0;
jmethodID	SFVec3f::mConstGetZMethodID = 0;
jmethodID	SFVec3f::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	SFVec3f::setJavaIDs
////////////////////////////////////////////////

void SFVec3f::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/SFVec3f");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstSFVec3f");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid = getFieldID();
		mInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(FFF)V");
		mGetXMethodID		= jniEnv->GetMethodID(classid, "getX", "()F");
		mGetYMethodID		= jniEnv->GetMethodID(classid, "getY", "()F");
		mGetZMethodID		= jniEnv->GetMethodID(classid, "getZ", "()F");
		mSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(FFF)V");
		mSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mInitMethodID && mGetXMethodID && mGetYMethodID && mGetZMethodID && mSetValueMethodID && mSetNameMethodID);

		// Const MethodIDs
		classid = getConstFieldID();
		mConstInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(FFF)V");
		mConstGetXMethodID		= jniEnv->GetMethodID(classid, "getX", "()F");
		mConstGetYMethodID		= jniEnv->GetMethodID(classid, "getY", "()F");
		mConstGetZMethodID		= jniEnv->GetMethodID(classid, "getZ", "()F");
		mConstSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(FFF)V");
		mConstSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mConstInitMethodID && mConstGetXMethodID && mConstGetYMethodID && mConstGetZMethodID && mConstSetValueMethodID && mConstSetNameMethodID);

		mInit = 1;
	}
}

////////////////////////////////////////////////
//	SFVec3f::toJavaObject
////////////////////////////////////////////////

jobject SFVec3f::toJavaObject(int bConstField) {
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	initMethod		= bConstField ? getConstInitMethodID() : getInitMethodID();
	jfloat		x				= getX();
	jfloat		y				= getY();
	jfloat		z				= getZ();
	jobject		eventField		= jniEnv->NewObject(classid, initMethod, x, y, z);
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
//	SFVec3f::setValue
////////////////////////////////////////////////

void SFVec3f::setValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getXMethod		= bConstField ? getConstGetXMethodID() : getGetXMethodID();
	jmethodID	getYMethod		= bConstField ? getConstGetYMethodID() : getGetYMethodID();
	jmethodID	getZMethod		= bConstField ? getConstGetZMethodID() : getGetZMethodID();
	assert(classid && getXMethod && getYMethod && getZMethod);
	jfloat		x				= jniEnv->CallFloatMethod(field, getXMethod);
	jfloat		y				= jniEnv->CallFloatMethod(field, getYMethod);
	jfloat		z				= jniEnv->CallFloatMethod(field, getZMethod);
	setValue(x, y, z);
}

////////////////////////////////////////////////
//	SFVec3f::getValue
////////////////////////////////////////////////

void SFVec3f::getValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	setValueMethod	= bConstField ? getConstSetValueMethodID() : getSetValueMethodID();
	assert(classid && setValueMethod);
	jfloat		x				= getX();
	jfloat		y				= getY();
	jfloat		z				= getZ();
	jniEnv->CallVoidMethod(field, setValueMethod, x, y, z);
}

#endif


