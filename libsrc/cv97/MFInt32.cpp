/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MFInt32.cpp
*
******************************************************************/

#include "MFInt32.h"

MFInt32::MFInt32()
{
	setType(fieldTypeMFInt32);
	InitializeJavaIDs();
}

void MFInt32::InitializeJavaIDs()
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void MFInt32::addValue(int value)
{
	SFInt32 *sfvalue = new SFInt32(value);
	add(sfvalue);
}

void MFInt32::addValue(SFInt32 *sfvalue)
{
	add(sfvalue);
}

void MFInt32::insertValue(int index, int value)
{
	SFInt32 *sfvalue = new SFInt32(value);
	insert(sfvalue, index);
}

int MFInt32::get1Value(int index)
{
	SFInt32 *sfvalue = (SFInt32 *)getObject(index);
	if (sfvalue)
		return sfvalue->getValue();
	else
		return 0;
}

void MFInt32::set1Value(int index, int value)
{
	SFInt32 *sfvalue = (SFInt32 *)getObject(index);
	if (sfvalue)
		sfvalue->setValue(value);
}

void MFInt32::setValue(MFInt32 *values)
{
	clear();

	int size = values->getSize();
	for (int n=0; n<size; n++) {
		addValue(values->get1Value(n));
	}
}

void MFInt32::setValue(MField *mfield)
{
	if (mfield->getType() == fieldTypeMFInt32)
		setValue((MFInt32 *)mfield);
}

void MFInt32::setValue(int size, int values[])
{
	clear();

	for (int n=0; n<size; n++)
		addValue(values[n]);
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void MFInt32::outputContext(ostream& printStream, char *indentString)
{
	for (int n=0; n<getSize(); n++) {
		if (n < getSize()-1)
			printStream << indentString << get1Value(n) << "," << endl;
		else
			printStream << indentString << get1Value(n) << endl;
	}
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			MFInt32::mInit = 0;

jclass		MFInt32::mFieldClassID = 0;
jclass		MFInt32::mConstFieldClassID = 0;

jmethodID	MFInt32::mInitMethodID = 0;
jmethodID	MFInt32::mGetSizeMethodID = 0;
jmethodID	MFInt32::mClearMethodID = 0;
jmethodID	MFInt32::mDeleteMethodID = 0;
jmethodID	MFInt32::mAddValueMethodID = 0;
jmethodID	MFInt32::mInsertValueMethodID = 0;
jmethodID	MFInt32::mSet1ValueMethodID = 0;
jmethodID	MFInt32::mGet1ValueMethodID = 0;
jmethodID	MFInt32::mSetNameMethodID = 0;

jmethodID	MFInt32::mConstInitMethodID = 0;
jmethodID	MFInt32::mConstGetSizeMethodID = 0;
jmethodID	MFInt32::mConstClearMethodID = 0;
jmethodID	MFInt32::mConstDeleteMethodID = 0;
jmethodID	MFInt32::mConstAddValueMethodID = 0;
jmethodID	MFInt32::mConstInsertValueMethodID = 0;
jmethodID	MFInt32::mConstSet1ValueMethodID = 0;
jmethodID	MFInt32::mConstGet1ValueMethodID = 0;
jmethodID	MFInt32::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	MFInt32::setJavaIDs
////////////////////////////////////////////////

void MFInt32::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/MFInt32");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstMFInt32");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid			= getFieldID();
		mInitMethodID			= jniEnv->GetMethodID(classid, "<init>",		"()V");
		mGetSizeMethodID		= jniEnv->GetMethodID(classid, "getSize",		"()I");
		mClearMethodID			= jniEnv->GetMethodID(classid, "clear",			"()V");
		mDeleteMethodID			= jniEnv->GetMethodID(classid, "delete",		"(I)V");
		mAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(I)V");
		mInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(II)V");
		mSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(II)V");
		mGet1ValueMethodID		= jniEnv->GetMethodID(classid, "get1Value",		"(I)I");
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
		mConstAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(I)V");
		mConstInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(II)V");
		mConstSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(II)V");
		mConstGet1ValueMethodID		= jniEnv->GetMethodID(classid, "get1Value",		"(I)I");
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
//	MFInt32::toJavaObject
////////////////////////////////////////////////

jobject MFInt32::toJavaObject(int bConstField) {
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
		jint value = get1Value(n);
		jniEnv->CallVoidMethod(fieldObject, addValueMethod, value);
	}

	return fieldObject;
}

////////////////////////////////////////////////
//	MFInt32::setValue
////////////////////////////////////////////////

void MFInt32::setValue(jobject field, int bConstField) {
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getSizeMethod	= bConstField ? getConstGetSizeMethodID() : getGetSizeMethodID();
	jmethodID	getValueMethod	= bConstField ? getConstGet1ValueMethodID() : getGet1ValueMethodID();
	assert(classid && getValueMethod);

	jint	jsize	= jniEnv->CallIntMethod(field, getSizeMethod);
	int		size	= getSize();

	for (int n=0; n<jsize; n++) {
		jint value = jniEnv->CallIntMethod(field, getValueMethod, n);
		if (n < size)
			set1Value(0, value);
		else
			addValue(value);
	}
}

////////////////////////////////////////////////
//	MFInt32::getValue
////////////////////////////////////////////////

void MFInt32::getValue(jobject field, int bConstField) {
}

#endif





