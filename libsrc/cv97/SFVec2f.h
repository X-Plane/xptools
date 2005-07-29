/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFVec2f.h
*
******************************************************************/

#ifndef _CV97_SFVEC2F_H_
#define _CV97_SFVEC2F_H_

#include <math.h>
#include <stdio.h>
#include <iostream.h>
#include "Field.h"

class SFVec2f : public Field {

	static	int	mInit;

	float mValue[2]; 

public:

	SFVec2f();
	SFVec2f(float x, float y);
	SFVec2f(float value[]);
	SFVec2f(SFVec2f *value);

	void InitializeJavaIDs();

	////////////////////////////////////////////////
	//	get value
	////////////////////////////////////////////////

	void getValue(float value[]);
	float *getValue();
	float getX();
	float getY();

	////////////////////////////////////////////////
	//	set value
	////////////////////////////////////////////////

	void setValue(float x, float y);
	void setValue(float value[]);
	void setValue(SFVec2f *vector);
	void setX(float x);
	void setY(float y);

	////////////////////////////////////////////////
	//	add value
	////////////////////////////////////////////////

	void add(float x, float y);
	void add(float value[]);
	void add(SFVec2f value);
	void translate(float x, float y);
	void translate(float value[]);
	void translate(SFVec2f value);

	////////////////////////////////////////////////
	//	sub value
	////////////////////////////////////////////////

	void sub(float x, float y);
	void sub(float value[]);
	void sub(SFVec2f value);

	////////////////////////////////////////////////
	//	scale
	////////////////////////////////////////////////

	void scale(float value);
	void scale(float xscale, float yscale);
	void scale(float value[2]);

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
	//	Output
	////////////////////////////////////////////////

	friend ostream& operator<<(ostream &s, SFVec2f &vector);
	friend ostream& operator<<(ostream &s, SFVec2f *vector);

	////////////////////////////////////////////////
	//	String
	////////////////////////////////////////////////

	void setValue(char *value);
	char *getValue(char *buffer, int bufferLen);

	////////////////////////////////////////////////
	//	Compare
	////////////////////////////////////////////////

	bool equals(Field *field);
	bool equals(float value[2]);
	bool equals(float x, float y);

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
	static jmethodID	mSetValueMethodID;
	static jmethodID	mSetNameMethodID;

	static jmethodID	mConstInitMethodID;
	static jmethodID	mConstGetXMethodID;
	static jmethodID	mConstGetYMethodID;
	static jmethodID	mConstSetValueMethodID;
	static jmethodID	mConstSetNameMethodID;

public:

	void		setJavaIDs();

	jclass		getFieldID()				{return mFieldClassID;}
	jclass		getConstFieldID()			{return mConstFieldClassID;}

	jmethodID	getInitMethodID()			{return mInitMethodID;}
	jmethodID	getGetXMethodID()			{return mGetXMethodID;}
	jmethodID	getGetYMethodID()			{return mGetYMethodID;}
	jmethodID	getSetValueMethodID()		{return mSetValueMethodID;}
	jmethodID	getSetNameMethodID()		{return mSetNameMethodID;}

	jmethodID	getConstInitMethodID()		{return mConstInitMethodID;}
	jmethodID	getConstGetXMethodID()		{return mConstGetXMethodID;}
	jmethodID	getConstGetYMethodID()		{return mConstGetYMethodID;}
	jmethodID	getConstSetValueMethodID()	{return mConstSetValueMethodID;}
	jmethodID	getConstSetNameMethodID()	{return mConstSetNameMethodID;}

	jobject toJavaObject(int bConstField = 0);
	void setValue(jobject field, int bConstField = 0);
	void getValue(jobject field, int bConstField = 0);

#endif

};

#endif