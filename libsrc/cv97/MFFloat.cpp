/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MFFloat.cpp
*
******************************************************************/

#include "MFFloat.h"

MFFloat::MFFloat()
{
	setType(fieldTypeMFFloat);
	InitializeJavaIDs();
}

void MFFloat::InitializeJavaIDs()
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void MFFloat::addValue(float value)
{
	SFFloat *sfvalue = new SFFloat(value);
	add(sfvalue);
}

void MFFloat::addValue(SFFloat *sfvalue)
{
	add(sfvalue);
}

void MFFloat::insertValue(int index, float value)
{
	SFFloat *sfvalue = new SFFloat(value);
	insert(sfvalue, index);
}

float MFFloat::get1Value(int index)
{
	SFFloat *sfvalue = (SFFloat *)getObject(index);
	if (sfvalue)
		return sfvalue->getValue();
	else
		return 0.0f;
}

void MFFloat::set1Value(int index, float value)
{
	SFFloat *sfvalue = (SFFloat *)getObject(index);
	if (sfvalue)
		sfvalue->setValue(value);
}

void MFFloat::setValue(MFFloat *values)
{
	clear();

	int size = values->getSize();
	for (int n=0; n<size; n++) {
		addValue(values->get1Value(n));
	}
}

void MFFloat::setValue(MField *mfield)
{
	if (mfield->getType() == fieldTypeMFFloat)
		setValue((MFFloat *)mfield);
}

void MFFloat::setValue(int size, float values[])
{
	clear();

	for (int n=0; n<size; n++)
		addValue(values[n]);
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void MFFloat::outputContext(ostream& printStream, char *indentString)
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

int			MFFloat::mInit = 0;

jclass		MFFloat::mFieldClassID = 0;
jclass		MFFloat::mConstFieldClassID = 0;

jmethodID	MFFloat::mInitMethodID = 0;
jmethodID	MFFloat::mGetSizeMethodID = 0;
jmethodID	MFFloat::mClearMethodID = 0;
jmethodID	MFFloat::mDeleteMethodID = 0;
jmethodID	MFFloat::mAddValueMethodID = 0;
jmethodID	MFFloat::mInsertValueMethodID = 0;
jmethodID	MFFloat::mSet1ValueMethodID = 0;
jmethodID	MFFloat::mGet1ValueMethodID = 0;
jmethodID	MFFloat::mSetNameMethodID = 0;

jmethodID	MFFloat::mConstInitMethodID = 0;
jmethodID	MFFloat::mConstGetSizeMethodID = 0;
jmethodID	MFFloat::mConstClearMethodID = 0;
jmethodID	MFFloat::mConstDeleteMethodID = 0;
jmethodID	MFFloat::mConstAddValueMethodID = 0;
jmethodID	MFFloat::mConstInsertValueMethodID = 0;
jmethodID	MFFloat::mConstSet1ValueMethodID = 0;
jmethodID	MFFloat::mConstGet1ValueMethodID = 0;
jmethodID	MFFloat::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	MFFloat::setJavaIDs
////////////////////////////////////////////////

void MFFloat::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/MFFloat");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstMFFloat");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid			= getFieldID();
		mInitMethodID			= jniEnv->GetMethodID(classid, "<init>",		"()V");
		mGetSizeMethodID		= jniEnv->GetMethodID(classid, "getSize",		"()I");
		mClearMethodID			= jniEnv->GetMethodID(classid, "clear",			"()V");
		mDeleteMethodID			= jniEnv->GetMethodID(classid, "delete",		"(I)V");
		mAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(F)V");
		mInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(IF)V");
		mSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(IF)V");
		mGet1ValueMethodID		= jniEnv->GetMethodID(classid, "get1Value",		"(I)F");
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
		mConstAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(F)V");
		mConstInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(IF)V");
		mConstSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(IF)V");
		mConstGet1ValueMethodID		= jniEnv->GetMethodID(classid, "get1Value",		"(I)F");
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

		mInit = 1;
	}
}

////////////////////////////////////////////////
//	MFFloat::toJavaObject
////////////////////////////////////////////////

jobject MFFloat::toJavaObject(int bConstField) {
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
		jfloat value = get1Value(n);
		jniEnv->CallVoidMethod(fieldObject, addValueMethod, value);
	}

	return fieldObject;

}

////////////////////////////////////////////////
//	MFFloat::setValue
////////////////////////////////////////////////

void MFFloat::setValue(jobject field, int bConstField)
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
		jfloat value = jniEnv->CallFloatMethod(field, getValueMethod, n);
		if (n < size)
			set1Value(0, value);
		else
			addValue(value);
	}
}

////////////////////////////////////////////////
//	MFFloat::getValue
////////////////////////////////////////////////

void MFFloat::getValue(jobject field, int bConstField) {
}

#endif


