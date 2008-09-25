/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MFTime.cpp
*
******************************************************************/

#include "MFTime.h"

MFTime::MFTime()
{
	setType(fieldTypeMFTime);
	InitializeJavaIDs();
}

void MFTime::InitializeJavaIDs()
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void MFTime::addValue(double value)
{
	SFTime *sfvalue = new SFTime(value);
	add(sfvalue);
}

void MFTime::addValue(SFTime *sfvalue)
{
	add(sfvalue);
}

void MFTime::insertValue(int index, double value)
{
	SFTime *sfvalue = new SFTime(value);
	insert(sfvalue, index);
}

double MFTime::get1Value(int index)
{
	SFTime *sfvalue = (SFTime *)getObject(index);
	if (sfvalue)
		return sfvalue->getValue();
	else
		return 0.0;
}

void MFTime::set1Value(int index, double value)
{
	SFTime *sfvalue = (SFTime *)getObject(index);
	if (sfvalue)
		sfvalue->setValue(value);
}

void MFTime::setValue(MFTime *values)
{
	clear();

	int size = values->getSize();
	for (int n=0; n<size; n++) {
		addValue(values->get1Value(n));
	}
}

void MFTime::setValue(MField *mfield)
{
	if (mfield->getType() == fieldTypeMFTime)
		setValue((MFTime *)mfield);
}

void MFTime::setValue(int size, double values[])
{
	clear();

	for (int n=0; n<size; n++)
		addValue(values[n]);
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void MFTime::outputContext(ostream& printStream, char *indentString)
{
	for (int n=0; n<getSize(); n++) {
		if (n < getSize()-1)
			printStream << indentString << get1Value(n) << "," << endl;
		else
			printStream << indentString << get1Value(n) << endl;
	}
}

////////////////////////////////////////////////
//	JSAI
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			MFTime::mInit = 0;

jclass		MFTime::mFieldClassID = 0;
jclass		MFTime::mConstFieldClassID = 0;

jmethodID	MFTime::mInitMethodID = 0;
jmethodID	MFTime::mGetSizeMethodID = 0;
jmethodID	MFTime::mClearMethodID = 0;
jmethodID	MFTime::mDeleteMethodID = 0;
jmethodID	MFTime::mAddValueMethodID = 0;
jmethodID	MFTime::mInsertValueMethodID = 0;
jmethodID	MFTime::mSet1ValueMethodID = 0;
jmethodID	MFTime::mGet1ValueMethodID = 0;
jmethodID	MFTime::mSetNameMethodID = 0;

jmethodID	MFTime::mConstInitMethodID = 0;
jmethodID	MFTime::mConstGetSizeMethodID = 0;
jmethodID	MFTime::mConstClearMethodID = 0;
jmethodID	MFTime::mConstDeleteMethodID = 0;
jmethodID	MFTime::mConstAddValueMethodID = 0;
jmethodID	MFTime::mConstInsertValueMethodID = 0;
jmethodID	MFTime::mConstSet1ValueMethodID = 0;
jmethodID	MFTime::mConstGet1ValueMethodID = 0;
jmethodID	MFTime::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	MFTime::setJavaIDs
////////////////////////////////////////////////

void MFTime::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/MFTime");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstMFTime");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid			= getFieldID();
		mInitMethodID			= jniEnv->GetMethodID(classid, "<init>",		"()V");
		mGetSizeMethodID		= jniEnv->GetMethodID(classid, "getSize",		"()I");
		mClearMethodID			= jniEnv->GetMethodID(classid, "clear",			"()V");
		mDeleteMethodID			= jniEnv->GetMethodID(classid, "delete",		"(I)V");
		mAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(D)V");
		mInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(ID)V");
		mSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(ID)V");
		mGet1ValueMethodID		= jniEnv->GetMethodID(classid, "get1Value",		"(I)D");
		mSetNameMethodID		= jniEnv->GetMethodID(classid, "setName",		"(Ljava/lang/String;)V");

		assert(mInitMethodID);
		assert(mGetSizeMethodID);
		assert(mClearMethodID);
		assert(mDeleteMethodID);
		assert(mAddValueMethodID);
		assert(mInsertValueMethodID);
		assert(mSet1ValueMethodID);
		assert(mGet1ValueMethodID);
		assert(mSetNameMethodID);

		// Const MethodIDs
		classid						= getConstFieldID();
		mConstInitMethodID			= jniEnv->GetMethodID(classid, "<init>",		"()V");
		mConstGetSizeMethodID		= jniEnv->GetMethodID(classid, "getSize",		"()I");
		mConstClearMethodID			= jniEnv->GetMethodID(classid, "clear",			"()V");
		mConstDeleteMethodID		= jniEnv->GetMethodID(classid, "delete",		"(I)V");
		mConstAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(D)V");
		mConstInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(ID)V");
		mConstSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(ID)V");
		mConstGet1ValueMethodID		= jniEnv->GetMethodID(classid, "get1Value",		"(I)D");
		mConstSetNameMethodID		= jniEnv->GetMethodID(classid, "setName",		"(Ljava/lang/String;)V");

		assert(mConstInitMethodID);
		assert(mConstGetSizeMethodID);
		assert(mConstClearMethodID);
		assert(mConstDeleteMethodID);
		assert(mConstAddValueMethodID);
		assert(mConstInsertValueMethodID);
		assert(mConstSet1ValueMethodID);
		assert(mConstGet1ValueMethodID);
		assert(mConstSetNameMethodID);
		assert(mConstInitMethodID && mConstSetNameMethodID);

		mInit = 1;
	}
}

////////////////////////////////////////////////
//	MFTime::toJavaObject
////////////////////////////////////////////////

jobject MFTime::toJavaObject(int bConstField) {
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	initMethod		= bConstField ? getConstInitMethodID() : getInitMethodID();
	jobject		fieldObject		= jniEnv->NewObject(classid, initMethod);
	jmethodID	setNameMethod	= bConstField ? getConstSetNameMethodID() : getSetNameMethodID();
	jmethodID	addValueMethod	= bConstField ? getConstAddValueMethodID() : getAddValueMethodID();

	char		*fieldName		= getName();
	jstring		jfieldName		= NULL;
	if (fieldName && strlen(fieldName))
		jfieldName = jniEnv->NewStringUTF(getName());
	jniEnv->CallVoidMethod(fieldObject, setNameMethod, jfieldName);
	if (jfieldName)
		jniEnv->DeleteLocalRef(jfieldName);

	int size = getSize();
	for (int n=0; n<size; n++) {
		jdouble value = get1Value(n);
		jniEnv->CallVoidMethod(fieldObject, addValueMethod, value);
	}

	return fieldObject;
}

////////////////////////////////////////////////
//	MFTime::setValue
////////////////////////////////////////////////

void MFTime::setValue(jobject field, int bConstField)
{
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getSizeMethod	= bConstField ? getConstGetSizeMethodID() : getGetSizeMethodID();
	jmethodID	getValueMethod	= bConstField ? getConstGet1ValueMethodID() : getGet1ValueMethodID();
	assert(classid && getValueMethod);

	jint	jsize	= jniEnv->CallIntMethod(field, getSizeMethod);
	int		size	= getSize();

	for (int n=0; n<jsize; n++) {
		jdouble value = jniEnv->CallDoubleMethod(field, getValueMethod, n);
		if (n < size)
			set1Value(0, value);
		else
			addValue(value);
	}
}

////////////////////////////////////////////////
//	MFTime::getValue
////////////////////////////////////////////////

void MFTime::getValue(jobject field, int bConstField) {
}

#endif



