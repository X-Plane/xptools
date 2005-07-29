/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFVec3f.h
*
******************************************************************/

#ifndef _CV97_SFVEC3F_H_
#define _CV97_SFVEC3F_H_

#include <math.h>
#include <stdio.h>
#include <iostream.h>
#include "Field.h"

class SFRotation;

class SFVec3f : public Field {

	static	int	mInit;
	float	mValue[3]; 

public:

	SFVec3f();
	SFVec3f(float x, float y, float z);
	SFVec3f(float value[]);
	SFVec3f(SFVec3f *value);

	void InitializeJavaIDs();

	////////////////////////////////////////////////
	//	get value
	////////////////////////////////////////////////

	void getValue(float value[]);
	float *getValue();
	float getX();
	float getY();
	float getZ();

	////////////////////////////////////////////////
	//	set value
	////////////////////////////////////////////////

	void setValue(float x, float y, float z);
	void setValue(float value[]);
	void setValue(SFVec3f *vector);
	void setX(float x);
	void setY(float y);
	void setZ(float z);

	////////////////////////////////////////////////
	//	add value
	////////////////////////////////////////////////

	void add(float x, float y, float z);
	void add(float value[]);
	void add(SFVec3f value);
	void translate(float x, float y, float z);
	void translate(float value[]);
	void translate(SFVec3f value);

	////////////////////////////////////////////////
	//	sub value
	////////////////////////////////////////////////

	void sub(float x, float y, float z);
	void sub(float value[]);
	void sub(SFVec3f value);

	////////////////////////////////////////////////
	//	scale
	////////////////////////////////////////////////

	void scale(float value);
	void scale(float xscale, float yscale, float zscale);
	void scale(float value[3]);

	////////////////////////////////////////////////
	//	rotate
	////////////////////////////////////////////////

	void rotate(SFRotation *rotation);
	void rotate(float x, float y, float z, float angle);
	void rotate(float value[3]);

	////////////////////////////////////////////////
	//	invert
	////////////////////////////////////////////////

	void invert();

	////////////////////////////////////////////////
	//	scalar
	////////////////////////////////////////////////

	float getScalar();

	////////////////////////////////////////////////
	//	normalize
	////////////////////////////////////////////////

	void normalize();

	////////////////////////////////////////////////
	//	Overload
	////////////////////////////////////////////////

	friend ostream& operator<<(ostream &s, SFVec3f &vector);
	friend ostream& operator<<(ostream &s, SFVec3f *vector);

	////////////////////////////////////////////////
	//	String
	////////////////////////////////////////////////

	void setValue(char *value);
	char *getValue(char *buffer, int bufferLen);

	////////////////////////////////////////////////
	//	Compare
	////////////////////////////////////////////////

	bool equals(Field *field);
	bool equals(float value[3]);
	bool equals(float x, float y, float z);

	////////////////////////////////////////////////
	//	Java
	////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

private:

	static jclass		mFieldClassID;
	static jclass		mConstFieldClassID;

	static jmethodID	mInitMethodID;
	static jmethodID	mGetXMethodID;
	static jmethodID	mGetYMethodID;
	static jmethodID	mGetZMethodID;
	static jmethodID	mSetValueMethodID;
	static jmethodID	mSetNameMethodID;

	static jmethodID	mConstInitMethodID;
	static jmethodID	mConstGetXMethodID;
	static jmethodID	mConstGetYMethodID;
	static jmethodID	mConstGetZMethodID;
	static jmethodID	mConstSetValueMethodID;
	static jmethodID	mConstSetNameMethodID;

public:

	void		setJavaIDs();

	jclass		getFieldID()				{return mFieldClassID;}
	jclass		getConstFieldID()			{return mConstFieldClassID;}

	jmethodID	getInitMethodID()			{return mInitMethodID;}
	jmethodID	getGetXMethodID()			{return mGetXMethodID;}
	jmethodID	getGetYMethodID()			{return mGetYMethodID;}
	jmethodID	getGetZMethodID()			{return mGetZMethodID;}
	jmethodID	getSetValueMethodID()		{return mSetValueMethodID;}
	jmethodID	getSetNameMethodID()		{return mSetNameMethodID;}

	jmethodID	getConstInitMethodID()		{return mConstInitMethodID;}
	jmethodID	getConstGetXMethodID()		{return mConstGetXMethodID;}
	jmethodID	getConstGetYMethodID()		{return mConstGetYMethodID;}
	jmethodID	getConstGetZMethodID()		{return mConstGetZMethodID;}
	jmethodID	getConstSetValueMethodID()	{return mConstSetValueMethodID;}
	jmethodID	getConstSetNameMethodID()	{return mConstSetNameMethodID;}

	jobject toJavaObject(int bConstField = 0);
	void setValue(jobject field, int bConstField = 0);
	void getValue(jobject field, int bConstField = 0);

#endif

};

#endif