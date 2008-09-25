/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFRotation.cpp
*
******************************************************************/

#include "SFRotation.h"
#include "SFMatrix.h"

SFRotation::SFRotation()
{
	setType(fieldTypeSFRotation);
	setValue(0.0f, 0.0f, 1.0f, 0.0f);
	InitializeJavaIDs();
}

SFRotation::SFRotation(float x, float y, float z, float rot)
{
	setType(fieldTypeSFRotation);
	setValue(x, y, z, rot);
	InitializeJavaIDs();
}

SFRotation::SFRotation(float vector[3], float rot)
{
	setType(fieldTypeSFRotation);
	setValue(vector, rot);
	InitializeJavaIDs();
}

SFRotation::SFRotation(float value[])
{
	setType(fieldTypeSFRotation);
	setValue(value);
	InitializeJavaIDs();
}

SFRotation::SFRotation(SFRotation *value)
{
	setType(fieldTypeSFRotation);
	setValue(value);
	InitializeJavaIDs();
}

SFRotation::SFRotation(SFMatrix *matrix) {
	setType(fieldTypeSFRotation);
	setValue(matrix);
	InitializeJavaIDs();
}

void SFRotation::InitializeJavaIDs() {
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

////////////////////////////////////////////////
//	set value
////////////////////////////////////////////////

void SFRotation::setValue(float x, float y, float z, float rot)
{
	mVector.setValue(x, y, z);
	mVector.normalize();
	mAngle = rot;
}

void SFRotation::setValue(float value[])
{
	mVector.setValue(value);
	mVector.normalize();
	mAngle = value[3];
}

void SFRotation::setValue(float value[], float angle)
{
	mVector.setValue(value);
	mVector.normalize();
	mAngle = angle;
}

void SFRotation::setValue(SFRotation *rotation)
{
	setValue(rotation->getX(), rotation->getY(), rotation->getZ(), rotation->getAngle());
}

////////////////////////////////////////////////
//	get value
////////////////////////////////////////////////

void SFRotation::getValue(float value[])
{
	mVector.getValue(value);
	value[3] = mAngle;
}

void SFRotation::getVector(float vector[])
{
	mVector.getValue(vector);
}

float SFRotation::getX()
{
	return mVector.getX();
}

float SFRotation::getY()
{
	return mVector.getY();
}

float SFRotation::getZ()
{
	return mVector.getZ();
}

float SFRotation::getAngle()
{
	return mAngle;
}

////////////////////////////////////////////////
//	add
////////////////////////////////////////////////

void SFRotation::add(float rotationValue[])
{
	SFRotation rotation(rotationValue);
	add(&rotation);
}

void SFRotation::add(float x, float y, float z, float rot)
{
	float rotationValue[4];
	rotationValue[0] = x;
	rotationValue[1] = y;
	rotationValue[2] = z;
	rotationValue[3] = rot;
	add(rotationValue);
}

////////////////////////////////////////////////
//	invert
///////////////////////////////////////////////

void SFRotation::invert()
{
	mAngle = -mAngle;
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

ostream& operator<<(ostream &s, SFRotation &rotation)
{
	return s << rotation.getX() << " " << rotation.getY() << " " << rotation.getZ() << " " << rotation.getAngle();
}

ostream& operator<<(ostream &s, SFRotation *rotation)
{
	return s << rotation->getX() << " " << rotation->getY() << " " << rotation->getZ()  << " " << rotation->getAngle();
}

////////////////////////////////////////////////
//	String
////////////////////////////////////////////////

void SFRotation::setValue(char *value)
{
	if (!value)
		return;
	float	x, y, z, angle;
	if (sscanf(value,"%f %f %f %f", &x, &y, &z, &angle) == 4)
		setValue(x, y, z, angle);
}

char *SFRotation::getValue(char *buffer, int bufferLen)
{
	sprintf(buffer, "%g %g %g %g", getX(), getY(), getZ(), getAngle());
	return buffer;
}

////////////////////////////////////////////////
//	initialize
////////////////////////////////////////////////

void SFRotation::initValue()
{
	setValue(0.0f, 0.0f, 1.0f, 0.0f);
}

////////////////////////////////////////////////
//	Compare
////////////////////////////////////////////////

bool SFRotation::equals(Field *field)
{
	SFRotation *rotation = (SFRotation *)field;
	if (getX() == rotation->getX() && getY() == rotation->getY() && getZ() == rotation->getZ() && getAngle() == rotation->getAngle())
		return true;
	else
		return false;
}

bool SFRotation::equals(float value[4])
{
	SFRotation rotation(value);
	return equals(&rotation);
}

bool SFRotation::equals(float x, float y, float z, float angle)
{
	SFRotation rotation(x, y, z, angle);
	return equals(&rotation);
}

////////////////////////////////////////////////
//	SFRotation::add
////////////////////////////////////////////////

void SFRotation::add(SFRotation *rot)
{
	SFMatrix m1;
	getSFMatrix(&m1);

	SFMatrix m2;
	rot->getSFMatrix(&m2);

	m1.add(&m2);

	SFRotation newRotation;
	m1.getSFRotation(&newRotation);

	setValue(&newRotation);
}

////////////////////////////////////////////////
//	SFRotation::multi
////////////////////////////////////////////////

void SFRotation::multi(float vector[])
{
	SFMatrix m;
	getSFMatrix(&m);
	m.multi(vector);
}

void SFRotation::multi(float *x, float *y, float *z)
{
	SFMatrix m;
	getSFMatrix(&m);
	m.multi(x, y, z);
}

void SFRotation::multi(SFVec3f *vector)
{
	SFMatrix m;
	getSFMatrix(&m);
	m.multi(vector);
}

////////////////////////////////////////////////
//	SFRotation::setValue
////////////////////////////////////////////////

void SFRotation::setValue(SFMatrix *matrix)
{
	matrix->getSFRotation(this);
}

////////////////////////////////////////////////
//	SFRotation::getSFMatrix
////////////////////////////////////////////////

void SFRotation::getSFMatrix(SFMatrix *matrix)
{
	float vector[3];
	getVector(vector);
	float rotation = getAngle();

	float k = (float)(1.0 - cos(rotation));
	float s = (float)sin(rotation);
	float c = (float)cos(rotation);
	float ax = vector[0];
	float ay = vector[1];
	float az = vector[2];
	float ax2 = ax * ax;
	float ay2 = ay * ay;
	float az2 = az * az;

	float ma[4][4];
	for (int i=0; i<4; i++) {
		ma[i][3] = 0.0f;
		ma[3][i] = 0.0f;
		ma[i][i] = 1.0f;
	}

	ma[0][0] = k*ax2 + c;
	ma[0][1] = k*ax*ay + s*az;
	ma[0][2] = k*ax*az - s*ay;
	ma[1][0] = k*ax*ay - s*az;
	ma[1][1] = k*ay2 + c;
	ma[1][2] = k*ay*az + s*ax;
	ma[2][0] = k*ax*az + s*ay;
	ma[2][1] = k*ay*az - s*ax;
	ma[2][2] = k*az2 + c;

	matrix->setValue(ma);
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			SFRotation::mInit = 0;

jclass		SFRotation::mFieldClassID = 0;
jclass		SFRotation::mConstFieldClassID = 0;

jmethodID	SFRotation::mInitMethodID = 0;
jmethodID	SFRotation::mSetValueMethodID = 0;
jmethodID	SFRotation::mGetXMethodID = 0;
jmethodID	SFRotation::mGetYMethodID = 0;
jmethodID	SFRotation::mGetZMethodID = 0;
jmethodID	SFRotation::mGetAngleMethodID = 0;
jmethodID	SFRotation::mSetNameMethodID = 0;

jmethodID	SFRotation::mConstInitMethodID = 0;
jmethodID	SFRotation::mConstSetValueMethodID = 0;
jmethodID	SFRotation::mConstGetXMethodID = 0;
jmethodID	SFRotation::mConstGetYMethodID = 0;
jmethodID	SFRotation::mConstGetZMethodID = 0;
jmethodID	SFRotation::mConstGetAngleMethodID = 0;
jmethodID	SFRotation::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	SFRotation::setJavaIDs
////////////////////////////////////////////////

void SFRotation::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/SFRotation");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstSFRotation");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid = getFieldID();
		mInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(FFFF)V");
		mGetXMethodID		= jniEnv->GetMethodID(classid, "getX", "()F");
		mGetYMethodID		= jniEnv->GetMethodID(classid, "getY", "()F");
		mGetZMethodID		= jniEnv->GetMethodID(classid, "getZ", "()F");
		mGetAngleMethodID	= jniEnv->GetMethodID(classid, "getAngle", "()F");
		mSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(FFFF)V");
		mSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mInitMethodID && mGetXMethodID && mGetYMethodID && mGetZMethodID && mGetAngleMethodID && mSetValueMethodID && mSetNameMethodID);

		// Const MethodIDs
		classid = getConstFieldID();
		mConstInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(FFFF)V");
		mConstGetXMethodID		= jniEnv->GetMethodID(classid, "getX", "()F");
		mConstGetYMethodID		= jniEnv->GetMethodID(classid, "getY", "()F");
		mConstGetZMethodID		= jniEnv->GetMethodID(classid, "getZ", "()F");
		mConstGetAngleMethodID	= jniEnv->GetMethodID(classid, "getAngle", "()F");
		mConstSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(FFFF)V");
		mConstSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mConstInitMethodID && mConstGetXMethodID && mConstGetYMethodID && mConstGetZMethodID && mConstGetAngleMethodID && mConstSetValueMethodID && mConstSetNameMethodID);

		mInit = 1;
	}
}

////////////////////////////////////////////////
//	SFRotation::toJavaObject
////////////////////////////////////////////////

jobject SFRotation::toJavaObject(int bConstField) {
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	initMethod		= bConstField ? getConstInitMethodID() : getInitMethodID();
	jfloat		x				= getX();
	jfloat		y				= getY();
	jfloat		z				= getZ();
	jfloat		angle			= getAngle();
	jobject		eventField		= jniEnv->NewObject(classid, initMethod, x, y, z, angle);
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
//	SFRotation::setValue
////////////////////////////////////////////////

void SFRotation::setValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getXMethod		= bConstField ? getConstGetXMethodID() : getGetXMethodID();
	jmethodID	getYMethod		= bConstField ? getConstGetYMethodID() : getGetYMethodID();
	jmethodID	getZMethod		= bConstField ? getConstGetZMethodID() : getGetZMethodID();
	jmethodID	getAngleMethod	= bConstField ? getConstGetAngleMethodID() : getGetAngleMethodID();
	assert(classid && getXMethod && getYMethod && getZMethod && getAngleMethod);
	jfloat		x				= jniEnv->CallFloatMethod(field, getXMethod);
	jfloat		y				= jniEnv->CallFloatMethod(field, getYMethod);
	jfloat		z				= jniEnv->CallFloatMethod(field, getZMethod);
	jfloat		angle			= jniEnv->CallFloatMethod(field, getAngleMethod);
	setValue(x, y, z, angle);
}

////////////////////////////////////////////////
//	SFRotation::getValue
////////////////////////////////////////////////

void SFRotation::getValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	setValueMethod	= bConstField ? getConstSetValueMethodID() : getSetValueMethodID();
	assert(classid && setValueMethod);
	jfloat		x				= getX();
	jfloat		y				= getY();
	jfloat		z				= getZ();
	jfloat		angle			= getAngle();
	jniEnv->CallVoidMethod(field, setValueMethod, x, y, z, angle);
}

#endif





