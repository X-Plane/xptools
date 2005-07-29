/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MFTime.h
*
******************************************************************/

#ifndef _CV97_MFTIME_H_
#define _CV97_MFTIME_H_

#include "MField.h"
#include "SFTime.h"

class MFTime : public MField {
	
	static	int	mInit;

public:

	MFTime();

	void InitializeJavaIDs();

	void addValue(double value);
	void addValue(SFTime *sfvalue);
	void insertValue(int index, double value);

	double get1Value(int index);
	void set1Value(int index, double value);

	void setValue(MField *mfield);
	void setValue(MFTime *times);
	void setValue(int size, double times[]);

	////////////////////////////////////////////////
	//	Output
	////////////////////////////////////////////////

	void outputContext(ostream& printStream, char *indentString);

	////////////////////////////////////////////////
	//	Java
	////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

private:

	static jclass		mFieldClassID;
	static jclass		mConstFieldClassID;

	static jmethodID	mInitMethodID;
	static jmethodID	mGetSizeMethodID;
	static jmethodID	mClearMethodID;
	static jmethodID	mDeleteMethodID;
	static jmethodID	mAddValueMethodID;
	static jmethodID	mInsertValueMethodID;
	static jmethodID	mSet1ValueMethodID;
	static jmethodID	mGet1ValueMethodID;
	static jmethodID	mSetNameMethodID;

	static jmethodID	mConstInitMethodID;
	static jmethodID	mConstGetSizeMethodID;
	static jmethodID	mConstClearMethodID;
	static jmethodID	mConstDeleteMethodID;
	static jmethodID	mConstAddValueMethodID;
	static jmethodID	mConstInsertValueMethodID;
	static jmethodID	mConstSet1ValueMethodID;
	static jmethodID	mConstGet1ValueMethodID;
	static jmethodID	mConstSetNameMethodID;

public:

	void		setJavaIDs();

	jclass		getFieldID()				{return mFieldClassID;}
	jclass		getConstFieldID()			{return mConstFieldClassID;}

	jmethodID	getInitMethodID()			{return mInitMethodID;}
	jmethodID	getGetSizeMethodID()		{return mGetSizeMethodID;}
	jmethodID	getClearMethodID()			{return mClearMethodID;}
	jmethodID	getDeleteMethodID()			{return mDeleteMethodID;}
	jmethodID	getAddValueMethodID()		{return mAddValueMethodID;}
	jmethodID	getInsertValueMethodID()	{return mInsertValueMethodID;}
	jmethodID	getSet1ValueMethodID()		{return mSet1ValueMethodID;}
	jmethodID	getGet1ValueMethodID()		{return mGet1ValueMethodID;}
	jmethodID	getSetNameMethodID()		{return mSetNameMethodID;}

	jmethodID	getConstInitMethodID()			{return mConstInitMethodID;}
	jmethodID	getConstGetSizeMethodID()		{return mConstGetSizeMethodID;}
	jmethodID	getConstClearMethodID()			{return mConstClearMethodID;}
	jmethodID	getConstDeleteMethodID()		{return mConstDeleteMethodID;}
	jmethodID	getConstAddValueMethodID()		{return mConstAddValueMethodID;}
	jmethodID	getConstInsertValueMethodID()	{return mConstInsertValueMethodID;}
	jmethodID	getConstSet1ValueMethodID()		{return mConstSet1ValueMethodID;}
	jmethodID	getConstGet1ValueMethodID()		{return mConstGet1ValueMethodID;}
	jmethodID	getConstSetNameMethodID()		{return mConstSetNameMethodID;}

	jobject toJavaObject(int bConstField = 0);
	void setValue(jobject field, int bConstField = 0);
	void getValue(jobject field, int bConstField = 0);

#endif
};

#endif