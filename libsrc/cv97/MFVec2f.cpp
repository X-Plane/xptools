/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MFVec2f.cpp
*
******************************************************************/

#include "MFVec2f.h"

MFVec2f::MFVec2f()
{
	setType(fieldTypeMFVec2f);
	InitializeJavaIDs();
}

void MFVec2f::InitializeJavaIDs()
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void MFVec2f::addValue(float x, float y)
{
	SFVec2f *vector = new SFVec2f(x, y);
	add(vector);
}

void MFVec2f::addValue(float value[])
{
	SFVec2f *vector = new SFVec2f(value);
	add(vector);
}

void MFVec2f::addValue(SFVec2f *vector)
{
	add(vector);
}

void MFVec2f::insertValue(int index, float x, float y)
{
	SFVec2f *vector = new SFVec2f(x, y);
	insert(vector, index);
}

void MFVec2f::insertValue(int index, float value[])
{
	SFVec2f *vector = new SFVec2f(value);
	insert(vector, index);
}

void MFVec2f::insertValue(int index, SFVec2f *vector)
{
	insert(vector, index);
}

void MFVec2f::get1Value(int index, float value[])
{
	SFVec2f *vector = (SFVec2f *)getObject(index);
	if (vector)
		vector->getValue(value);
}

void MFVec2f::set1Value(int index, float value[])
{
	SFVec2f *vector = (SFVec2f *)getObject(index);
	if (vector)
		vector->setValue(value);
}

void MFVec2f::set1Value(int index, float x, float y)
{
	SFVec2f *vector = (SFVec2f *)getObject(index);
	if (vector)
		vector->setValue(x, y);
}

void MFVec2f::setValue(MFVec2f *vectors)
{
	clear();

	float value[3];
	int size = vectors->getSize();
	for (int n=0; n<size; n++) {
		vectors->get1Value(n, value);
		addValue(value);
	}
}

void MFVec2f::setValue(MField *mfield)
{
	if (mfield->getType() == fieldTypeMFVec2f)
		setValue((MFVec2f *)mfield);
}

void MFVec2f::setValue(int size, float vectors[][2])
{
	clear();

	for (int n=0; n<size; n++)
		addValue(vectors[n]);
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void MFVec2f::outputContext(ostream& printStream, char *indentString)
{
	float value[2];
	for (int n=0; n<getSize(); n++) {
		get1Value(n, value);
		if (n < getSize()-1)
			printStream << indentString << value[0] << " " << value[1] << "," << endl;
		else
			printStream << indentString << value[0] << " " << value[1] << endl;
	}
}

////////////////////////////////////////////////
//	JSAI
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			MFVec2f::mInit = 0;

jclass		MFVec2f::mFieldClassID = 0;
jclass		MFVec2f::mConstFieldClassID = 0;

jmethodID	MFVec2f::mInitMethodID = 0;
jmethodID	MFVec2f::mGetSizeMethodID = 0;
jmethodID	MFVec2f::mClearMethodID = 0;
jmethodID	MFVec2f::mDeleteMethodID = 0;
jmethodID	MFVec2f::mAddValueMethodID = 0;
jmethodID	MFVec2f::mInsertValueMethodID = 0;
jmethodID	MFVec2f::mSet1ValueMethodID = 0;
jmethodID	MFVec2f::mGet1ValueMethodID = 0;
jmethodID	MFVec2f::mSetNameMethodID = 0;

jmethodID	MFVec2f::mConstInitMethodID = 0;
jmethodID	MFVec2f::mConstGetSizeMethodID = 0;
jmethodID	MFVec2f::mConstClearMethodID = 0;
jmethodID	MFVec2f::mConstDeleteMethodID = 0;
jmethodID	MFVec2f::mConstAddValueMethodID = 0;
jmethodID	MFVec2f::mConstInsertValueMethodID = 0;
jmethodID	MFVec2f::mConstSet1ValueMethodID = 0;
jmethodID	MFVec2f::mConstGet1ValueMethodID = 0;
jmethodID	MFVec2f::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	MFVec2f::setJavaIDs
////////////////////////////////////////////////

void MFVec2f::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/MFVec2f");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstMFVec2f");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid			= getFieldID();
		mInitMethodID			= jniEnv->GetMethodID(classid, "<init>",		"()V");
		mGetSizeMethodID		= jniEnv->GetMethodID(classid, "getSize",		"()I");
		mClearMethodID			= jniEnv->GetMethodID(classid, "clear",			"()V");
		mDeleteMethodID			= jniEnv->GetMethodID(classid, "delete",		"(I)V");
		mAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(FF)V");
		mInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(IFF)V");
		mSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(IFF)V");
		mGet1ValueMethodID		= jniEnv->GetMethodID(classid, "get1Value",		"(I[F)V");
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
		mConstAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(FF)V");
		mConstInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(IFF)V");
		mConstSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(IFF)V");
		mConstGet1ValueMethodID		= jniEnv->GetMethodID(classid, "get1Value",		"(I[F)V");
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
//	MFVec2f::toJavaObject
////////////////////////////////////////////////

jobject MFVec2f::toJavaObject(int bConstField)
{
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
		float value[2];
		get1Value(n, value);
		jfloat x = value[0];
		jfloat y = value[1];
		jniEnv->CallVoidMethod(fieldObject, addValueMethod, x, y);
	}

	return fieldObject;
}


////////////////////////////////////////////////
//	MFVec2f::setValue
////////////////////////////////////////////////

void MFVec2f::setValue(jobject field, int bConstField) {
}

////////////////////////////////////////////////
//	MFVec2f::getValue
////////////////////////////////////////////////

void MFVec2f::getValue(jobject field, int bConstField) {
}

#endif





