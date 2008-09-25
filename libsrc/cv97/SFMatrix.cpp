/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFMatrix.cpp
*
*	[ a00 a10 a20 a30 ][ X ]
*	[ a01 a11 a21 a31 ][ Y ]
*	[ a02 a12 a22 a32 ][ Z ]
*	[ a03 a13 a23 a33 ][ 1 ]
*
******************************************************************/

#include "SFMatrix.h"
#include "SFRotation.h"

SFMatrix::SFMatrix() {
	init();
}

SFMatrix::SFMatrix(float value[4][4]) {
	setValue(value);
}

SFMatrix::SFMatrix(SFMatrix *value) {
	setValue(value);
}

SFMatrix::SFMatrix(SFRotation *rot)
{
	setRotation(rot);
}

SFMatrix::SFMatrix(float x, float y, float z, float angle)
{
	setRotation(x, y, z, angle);
}

SFMatrix::SFMatrix(float x, float y, float z)
{
	setDirection(x, y, z);
}

////////////////////////////////////////////////
//	set value
////////////////////////////////////////////////

void SFMatrix::setValue(float value[4][4])
{
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++)
			mValue[i][j] = value[i][j];
	}
}

void SFMatrix::setValue(SFMatrix *matrix)
{
	float	value[4][4];
	matrix->getValue(value);
	setValue(value);
}

////////////////////////////////////////////////
//	set as scaling value
////////////////////////////////////////////////

void SFMatrix::setScaling(SFVec3f *vector)
{
	setScaling(vector->getX(), vector->getY(), vector->getZ());
}

void SFMatrix::setScaling(float value[])
{
	init();
	setScaling(value[0], value[1], value[2]);
}

void SFMatrix::setScaling(float x, float y, float z)
{
	init();
	mValue[0][0] = x;
	mValue[1][1] = y;
	mValue[2][2] = z;
}

////////////////////////////////////////////////
//	set as translation value
////////////////////////////////////////////////

void SFMatrix::setTranslation(SFVec3f *vector)
{
	setTranslation(vector->getX(), vector->getY(), vector->getZ());
}

void SFMatrix::setTranslation(float value[])
{
	setTranslation(value[0], value[1], value[2]);
}

void SFMatrix::setTranslation(float x, float y, float z)
{
	init();
	mValue[3][0] = x;
	mValue[3][1] = y;
	mValue[3][2] = z;
}

////////////////////////////////////////////////
//	set as direction value
////////////////////////////////////////////////

void SFMatrix::setDirection(SFVec3f *vector)
{
	setDirection(vector->getX(), vector->getY(), vector->getZ());
}

void SFMatrix::setDirection(float value[])
{
	setDirection(value[0], value[1], value[2]);
}

void SFMatrix::setDirection(float x, float y, float z) {
	SFMatrix	mx;
	SFMatrix	my;
	float		mxValue[4][4];
	float		myValue[4][4];
	float		normal[3];

	mx.getValue(mxValue);
	my.getValue(myValue);

	normal[0] = x;
	normal[1] = y;
	normal[2] = z;

	float d = (float)sqrt(normal[1]*normal[1] + normal[2]*normal[2]);

	if (d) {
		float cosa = normal[2] / d;
		float sina = normal[1] / d;
		mxValue[0][0] = 1.0;
		mxValue[0][1] = 0.0;
		mxValue[0][2] = 0.0;
		mxValue[1][0] = 0.0;
		mxValue[1][1] = cosa;
		mxValue[1][2] = sina;
		mxValue[2][0] = 0.0;
		mxValue[2][1] = -sina;
		mxValue[2][2] = cosa;
	}

	float cosb = d;
	float sinb = normal[0];

	myValue[0][0] = cosb;
	myValue[0][1] = 0.0;
	myValue[0][2] = sinb;
	myValue[1][0] = 0.0;
	myValue[1][1] = 1.0;
	myValue[1][2] = 0.0;
	myValue[2][0] = -sinb;
	myValue[2][1] = 0.0;
	myValue[2][2] = cosb;

	mx.setValue(mxValue);
	my.setValue(myValue);

	init();
	add(&my);
	add(&mx);
}

////////////////////////////////////////////////
//	set as rotation value
////////////////////////////////////////////////

void SFMatrix::setRotation(SFRotation *rotation)
{
	setRotation(rotation->getX(), rotation->getY(), rotation->getZ(), rotation->getAngle());
}

void SFMatrix::setRotation(float value[])
{
	setRotation(value[0], value[1], value[2], value[3]);
}

void SFMatrix::setRotation(float x, float y, float z, float rot)
{
	SFRotation rotation(x, y, z, rot);
	SFMatrix matrix;
	rotation.getSFMatrix(&matrix);
	float value[4][4];
	matrix.getValue(value);
	setValue(value);
}

////////////////////////////////////////////////
//	get value
////////////////////////////////////////////////

void SFMatrix::getValue(float value[4][4])
{
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++)
			value[i][j] = mValue[i][j];
	}
}

////////////////////////////////////////////////
//	get value only translation
////////////////////////////////////////////////

void SFMatrix::getTranslation(float value[])
{
	value[0] = mValue[3][0];
	value[1] = mValue[3][1];
	value[2] = mValue[3][2];
}

////////////////////////////////////////////////
//	toString
////////////////////////////////////////////////

void SFMatrix::setValue(char *value)
{
}

char *SFMatrix::getValue(char *buffer, int bufferLen)
{
	buffer[0] = '\0';
	return buffer;
}

////////////////////////////////////////////////
//	other
////////////////////////////////////////////////

void SFMatrix::init()
{
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++)
			mValue[i][j] = 0.0f;
	}

	for (int n=0; n<4; n++)
		mValue[n][n] = 1.0f;
}

////////////////////////////////////////////////
//	add
////////////////////////////////////////////////

void SFMatrix::add(SFMatrix *matrix)
{
	float ma1[4][4];
	float ma2[4][4];
	float maOut[4][4];

	getValue(ma1);
	matrix->getValue(ma2);

	for (int i = 0; i<4; i++) {
		for (int j = 0; j<4; j++) {
			maOut[j][i] = 0.0f;
			for (int k = 0; k<4; k++)
				maOut[j][i] += ma1[k][i]*ma2[j][k];
		}
	}

	setValue(maOut);
}

////////////////////////////////////////////////
//	multi
////////////////////////////////////////////////

void SFMatrix::multi(float vector[])
{
	float vectorIn[4];

	vectorIn[0] = vector[0];
	vectorIn[1] = vector[1];
	vectorIn[2] = vector[2];
	vectorIn[3] = 1.0f;

	float vectorOut[4];

	float ma[4][4];
	getValue(ma);
	for (int n = 0; n<4; n++)
		vectorOut[n] = ma[0][n]*vectorIn[0] + ma[1][n]*vectorIn[1] + ma[2][n]*vectorIn[2] + ma[3][n]*vectorIn[3];

	vector[0] = vectorOut[0];
	vector[1] = vectorOut[1];
	vector[2] = vectorOut[2];
}

void SFMatrix::multi(float *x, float *y, float *z)
{
	float value[3];
	value[0] = *x;
	value[1] = *y;
	value[2] = *z;
	multi(value);
	*x = value[0];
	*y = value[1];
	*z = value[2];
}

void SFMatrix::multi(SFVec3f *vector)
{
	float value[3];
	vector->getValue(value);
	multi(value);
	vector->setValue(value);
}

////////////////////////////////////////////////
//	getSFRotation
////////////////////////////////////////////////

void SFMatrix::getSFRotation(SFRotation *rotation)
{
	float x, y, z, w;
	float m[4][4];
	getValue(m);

	float w2 = 1.0f/4.0f*(1.0f + m[0][0] + m[1][1] + m[2][2]);

	if (0.0f < w2) {
		w = (float)sqrt(w2);
		x = (m[1][2] - m[2][1])/(4.0f*w);
		y = (m[2][0] - m[0][2])/(4.0f*w);
		z = (m[0][1] - m[1][0])/(4.0f*w);
	}
	else {
		w = 0.0f;
		float x2 = 0.0f;
		float m1122 = m[1][1] + m[2][2];
		if (m1122 != 0.0f)
			x2 = -1.0f/(2.0f*m1122);
		if (0.0f < x2) {
			x = (float)sqrt(x2);
			y = m[0][1] / (2.0f*x);
			z = m[0][2] / (2.0f*x);
		}
		else {
			x = 0.0f;
			float y2 = 0.0f;
			float m22 = 1.0f - m[2][2];
			if (m22 != 0.0f)
				y2 = 1.0f / (2.0f*m22);
			if (0.0f < y2) {
				y = (float)sqrt(y2);
				z = m[1][2] / (2.0f*y);
			}
			else {
				y = 0.0f;
				z = 1.0f;
			}
		}
	}

	float angle = (float)acos(2.0f*w*w - 1.0f);
	float value[4];
	if (angle != 0.0f) {
		value[0] = x / (float)sin(angle);
		value[1] = y / (float)sin(angle);
		value[2] = z / (float)sin(angle);
		value[3] = angle;
	}
	else {
		value[0] = 0.0f;
		value[1] = 0.0f;
		value[2] = 1.0f;
		value[3] = 0.0f;
	}

	rotation->setValue(value);
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

void SFMatrix::invert()
{
	float d = determinant();
	if (d == 0.0f)
		return;

	mValue[0][0] = mValue[1][1]*(mValue[2][2]*mValue[3][3] - mValue[2][3]*mValue[3][2]) + mValue[1][2]*(mValue[2][3]*mValue[3][1] - mValue[2][1]*mValue[3][3]) + mValue[1][3]*(mValue[2][1]*mValue[3][2] - mValue[2][2]*mValue[3][1]) / d;
	mValue[0][1] = mValue[2][1]*(mValue[0][2]*mValue[3][3] - mValue[0][3]*mValue[3][2]) + mValue[2][2]*(mValue[0][3]*mValue[3][1] - mValue[0][1]*mValue[3][3]) + mValue[2][3]*(mValue[0][1]*mValue[3][2] - mValue[0][2]*mValue[3][1]) / d;
	mValue[0][2] = mValue[3][1]*(mValue[0][2]*mValue[1][3] - mValue[0][3]*mValue[1][2]) + mValue[3][2]*(mValue[0][3]*mValue[1][1] - mValue[0][1]*mValue[1][3]) + mValue[3][3]*(mValue[0][1]*mValue[1][2] - mValue[0][2]*mValue[1][1]) / d;
	mValue[0][3] = mValue[0][1]*(mValue[1][3]*mValue[2][2] - mValue[1][2]*mValue[2][3]) + mValue[0][2]*(mValue[1][1]*mValue[2][3] - mValue[1][3]*mValue[2][1]) + mValue[0][3]*(mValue[1][2]*mValue[2][1] - mValue[1][1]*mValue[2][2]) / d;

	mValue[1][0] = mValue[1][2]*(mValue[2][0]*mValue[3][3] - mValue[2][3]*mValue[3][0]) + mValue[1][3]*(mValue[2][2]*mValue[3][0] - mValue[2][0]*mValue[3][2]) + mValue[1][0]*(mValue[2][3]*mValue[3][2] - mValue[2][2]*mValue[3][3]) / d;
	mValue[1][1] = mValue[2][2]*(mValue[0][0]*mValue[3][3] - mValue[0][3]*mValue[3][0]) + mValue[2][3]*(mValue[0][2]*mValue[3][0] - mValue[0][0]*mValue[3][2]) + mValue[2][0]*(mValue[0][3]*mValue[3][2] - mValue[0][2]*mValue[3][3]) / d;
	mValue[1][2] = mValue[3][2]*(mValue[0][0]*mValue[1][3] - mValue[0][3]*mValue[1][0]) + mValue[3][3]*(mValue[0][2]*mValue[1][0] - mValue[0][0]*mValue[1][2]) + mValue[3][0]*(mValue[0][3]*mValue[1][2] - mValue[0][2]*mValue[1][3]) / d;
	mValue[1][3] = mValue[0][2]*(mValue[1][3]*mValue[2][0] - mValue[1][0]*mValue[2][3]) + mValue[0][3]*(mValue[1][0]*mValue[2][2] - mValue[1][2]*mValue[2][0]) + mValue[0][0]*(mValue[1][2]*mValue[2][3] - mValue[1][3]*mValue[2][2]) / d;

	mValue[2][0] = mValue[1][3]*(mValue[2][0]*mValue[3][1] - mValue[2][1]*mValue[3][0]) + mValue[1][0]*(mValue[2][1]*mValue[3][3] - mValue[2][3]*mValue[3][1]) + mValue[1][1]*(mValue[2][3]*mValue[3][0] - mValue[2][0]*mValue[3][3]) / d;
	mValue[2][1] = mValue[2][3]*(mValue[0][0]*mValue[3][1] - mValue[0][1]*mValue[3][0]) + mValue[2][0]*(mValue[0][1]*mValue[3][3] - mValue[0][3]*mValue[3][1]) + mValue[2][1]*(mValue[0][3]*mValue[3][0] - mValue[0][0]*mValue[3][3]) / d;
	mValue[2][2] = mValue[3][3]*(mValue[0][0]*mValue[1][1] - mValue[0][1]*mValue[1][0]) + mValue[3][0]*(mValue[0][1]*mValue[1][3] - mValue[0][3]*mValue[1][1]) + mValue[3][1]*(mValue[0][3]*mValue[1][0] - mValue[0][0]*mValue[1][3]) / d;
	mValue[2][3] = mValue[0][3]*(mValue[1][1]*mValue[2][0] - mValue[1][0]*mValue[2][1]) + mValue[0][0]*(mValue[1][3]*mValue[2][1] - mValue[1][1]*mValue[2][3]) + mValue[0][1]*(mValue[1][0]*mValue[2][3] - mValue[1][3]*mValue[2][0]) / d;

	mValue[3][0] = mValue[1][0]*(mValue[2][2]*mValue[3][1] - mValue[2][1]*mValue[3][2]) + mValue[1][1]*(mValue[2][0]*mValue[3][2] - mValue[2][2]*mValue[3][0]) + mValue[1][2]*(mValue[2][1]*mValue[3][0] - mValue[2][0]*mValue[3][1]) / d;
	mValue[3][1] = mValue[2][0]*(mValue[0][2]*mValue[3][1] - mValue[0][1]*mValue[3][2]) + mValue[2][1]*(mValue[0][0]*mValue[3][2] - mValue[0][2]*mValue[3][0]) + mValue[2][2]*(mValue[0][1]*mValue[3][0] - mValue[0][0]*mValue[3][1]) / d;
	mValue[3][2] = mValue[3][0]*(mValue[0][2]*mValue[1][1] - mValue[0][1]*mValue[1][2]) + mValue[3][1]*(mValue[0][0]*mValue[1][2] - mValue[0][2]*mValue[1][0]) + mValue[3][2]*(mValue[0][1]*mValue[1][0] - mValue[0][0]*mValue[1][1]) / d;
	mValue[3][3] = mValue[0][0]*(mValue[1][1]*mValue[2][2] - mValue[1][2]*mValue[2][1]) + mValue[0][1]*(mValue[1][2]*mValue[2][0] - mValue[1][0]*mValue[2][2]) + mValue[0][2]*(mValue[1][0]*mValue[2][1] - mValue[1][1]*mValue[2][0]) / d;
}

float SFMatrix::determinant()
{
	return	 (mValue[0][0]*mValue[1][1] - mValue[0][1]*mValue[1][0])*(mValue[2][2]*mValue[3][3] - mValue[2][3]*mValue[3][2])
			-(mValue[0][0]*mValue[1][2] - mValue[0][2]*mValue[1][0])*(mValue[2][1]*mValue[3][3] - mValue[2][3]*mValue[3][1])
			+(mValue[0][0]*mValue[1][3] - mValue[0][3]*mValue[1][0])*(mValue[2][1]*mValue[3][2] - mValue[2][2]*mValue[3][1])
			+(mValue[0][1]*mValue[1][2] - mValue[0][2]*mValue[1][1])*(mValue[2][0]*mValue[3][3] - mValue[2][3]*mValue[3][0])
			-(mValue[0][1]*mValue[1][3] - mValue[0][3]*mValue[1][1])*(mValue[2][0]*mValue[3][2] - mValue[2][2]*mValue[3][0])
			+(mValue[0][2]*mValue[1][3] - mValue[0][3]*mValue[1][2])*(mValue[2][0]*mValue[3][1] - mValue[2][1]*mValue[3][0]);
}


////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

jobject SFMatrix::toJavaObject(int bConstField)
{
	return NULL;
}

void SFMatrix::setValue(jobject field, int bConstField)
{
}

void SFMatrix::getValue(jobject field, int bConstField)
{
}

#endif
