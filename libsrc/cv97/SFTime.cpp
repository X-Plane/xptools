/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFTime.cpp
*
******************************************************************/

#include <time.h>

#if defined(WIN32)
#if defined(__CYGWIN__) || defined(__MINGW32__)
#include <ctype.h>
#else
//#include <sys/timeb.h>
#endif
#endif

#if defined(__BEOS__)
#include <KernelKit.h>
#endif

#include "SFTime.h"

SFTime::SFTime()
{
	setType(fieldTypeSFTime);
	setValue(-1);
	InitializeJavaIDs();
}

SFTime::SFTime(double value)
{
	setType(fieldTypeSFTime);
	setValue(value);
	InitializeJavaIDs();
}

SFTime::SFTime(SFTime *value)
{
	setType(fieldTypeSFTime);
	setValue(value);
	InitializeJavaIDs();
}

void SFTime::InitializeJavaIDs()
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void SFTime::setValue(double value)
{
	mValue = value;
}

void SFTime::setValue(SFTime *value)
{
	setValue(value->getValue());
}

double SFTime::getValue()
{
	return mValue;
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

ostream& operator<<(ostream &s, SFTime &time)
{
	return s << time.getValue();
}

ostream& operator<<(ostream &s, SFTime *time)
{
	return s << time->getValue();
}

////////////////////////////////////////////////
//	String
////////////////////////////////////////////////

void SFTime::setValue(char *value)
{
	if (!value)
		return;
	setValue(atof(value));
}

char *SFTime::getValue(char *buffer, int bufferLen)
{
	sprintf(buffer, "%f", (float)getValue());
	return buffer;
}

////////////////////////////////////////////////
//	Compare
////////////////////////////////////////////////

bool SFTime::equals(Field *field)
{
	SFTime *time = (SFTime *)field;
	if (getValue() == time->getValue())
		return true;
	else
		return false;
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			SFTime::mInit = 0;

jclass		SFTime::mFieldClassID = 0;
jclass		SFTime::mConstFieldClassID = 0;

jmethodID	SFTime::mInitMethodID = 0;
jmethodID	SFTime::mSetValueMethodID = 0;
jmethodID	SFTime::mGetValueMethodID = 0;
jmethodID	SFTime::mSetNameMethodID = 0;

jmethodID	SFTime::mConstInitMethodID = 0;
jmethodID	SFTime::mConstSetValueMethodID = 0;
jmethodID	SFTime::mConstGetValueMethodID = 0;
jmethodID	SFTime::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	SFTime::toJavaObject
////////////////////////////////////////////////

void SFTime::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/SFTime");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstSFTime");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid = getFieldID();
		mInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(D)V");
		mGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()D");
		mSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(D)V");
		mSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mInitMethodID && mGetValueMethodID && mSetValueMethodID && mSetNameMethodID);

		// MethodIDs
		classid	 = getConstFieldID();
		mConstInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(D)V");
		mConstGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()D");
		mConstSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(D)V");
		mConstSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mConstInitMethodID && mConstGetValueMethodID && mConstSetValueMethodID && mConstSetNameMethodID);

		mInit = 1;
	}
}

////////////////////////////////////////////////
//	SFTime::toJavaObject
////////////////////////////////////////////////

jobject SFTime::toJavaObject(int bConstField) {
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	initMethod		= bConstField ? getConstInitMethodID() : getInitMethodID();
	jdouble		value			= getValue();
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
//	SFTime::setValue
////////////////////////////////////////////////

void SFTime::setValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getValueMethod	= bConstField ? getConstGetValueMethodID() : getGetValueMethodID();
	assert(classid && getValueMethod);
	jdouble		value			= jniEnv->CallDoubleMethod(field, getValueMethod);
	setValue(value);
}

////////////////////////////////////////////////
//	SFTime::getValue
////////////////////////////////////////////////

void SFTime::getValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	setValueMethod	= bConstField ? getConstSetValueMethodID() : getSetValueMethodID();
	assert(classid && setValueMethod);
	jdouble		value			= getValue();
	jniEnv->CallVoidMethod(field, setValueMethod, value);
}

#endif

////////////////////////////////////////////////
//	GetCurrentSystemTime
////////////////////////////////////////////////

double GetCurrentSystemTime()
{
	long	second;
	int		milisecond;

#if 0 && defined(WIN32)

#if defined(__CYGWIN__) || defined(__MINGW32__)
  	struct timeb timebuffer;
	ftime( &timebuffer );
#else
	struct _timeb timebuffer;
	_ftime( &timebuffer );
#endif

	second = timebuffer.time;// + timebuffer.timezone*60;
	milisecond = timebuffer.millitm ;

#elif defined(SGI)

	struct timeval	time;
	struct timezone	timeZone;

	BSDgettimeofday(&time, &timeZone);
	second = time.tv_sec + time.tv_usec;
	milisecond = time.tv_usec / 1000;

#elif defined(__BEOS__)

	second = real_time_clock();
	milisecond = real_time_clock_usecs();

#else

	time_t	ltime;

	second = time(&ltime);
	milisecond = 0;

#endif

	return ((double)second + ((double)milisecond / 1000.0));
}
