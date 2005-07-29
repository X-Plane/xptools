/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MFVec3f.cpp
*
******************************************************************/

#include "MFVec3f.h"

MFVec3f::MFVec3f() 
{
	setType(fieldTypeMFVec3f);
	InitializeJavaIDs();
}

void MFVec3f::InitializeJavaIDs() 
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void MFVec3f::addValue(float x, float y, float z) 
{
	SFVec3f *vector = new SFVec3f(x, y, z);
	add(vector);
}

void MFVec3f::addValue(float value[]) 
{
	SFVec3f *vector = new SFVec3f(value);
	add(vector);
}

void MFVec3f::addValue(SFVec3f *vector) 
{
	add(vector);
}

void MFVec3f::insertValue(int index, float x, float y, float z) 
{
	SFVec3f *vector = new SFVec3f(x, y, z);
	insert(vector, index);
}

void MFVec3f::insertValue(int index, float value[]) 
{
	SFVec3f *vector = new SFVec3f(value);
	insert(vector, index);
}

void MFVec3f::insertValue(int index, SFVec3f *vector) 
{
	insert(vector, index);
}

void MFVec3f::get1Value(int index, float value[]) 
{
	SFVec3f *vector = (SFVec3f *)getObject(index);
	if (vector)
		vector->getValue(value);
}

void MFVec3f::set1Value(int index, float value[]) 
{
	SFVec3f *vector = (SFVec3f *)getObject(index);
	if (vector)
		vector->setValue(value);
}

void MFVec3f::set1Value(int index, float x, float y, float z) 
{
	SFVec3f *vector = (SFVec3f *)getObject(index);
	if (vector)
		vector->setValue(x, y, z);
}

void MFVec3f::setValue(MFVec3f *vectors)
{
	clear();

	float value[3];
	int size = vectors->getSize();
	for (int n=0; n<size; n++) {
		vectors->get1Value(n, value);
		addValue(value);
	}
}

void MFVec3f::setValue(MField *mfield)
{
	if (mfield->getType() == fieldTypeMFVec3f)
		setValue((MFVec3f *)mfield);
}

void MFVec3f::setValue(int size, float vectors[][3])
{
	clear();

	for (int n=0; n<size; n++)
		addValue(vectors[n]);
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void MFVec3f::outputContext(ostream& printStream, char *indentString) 
{
	float value[3];
	for (int n=0; n<getSize(); n++) {
		get1Value(n, value);
		if (n < getSize()-1)
			printStream << indentString << value[0] << " " << value[1] << " " << value[2] << "," << endl;
		else	
			printStream << indentString << value[0] << " " << value[1] << " " << value[2] << endl;
	}
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			MFVec3f::mInit = 0;

jclass		MFVec3f::mFieldClassID = 0;
jclass		MFVec3f::mConstFieldClassID = 0;

jmethodID	MFVec3f::mInitMethodID = 0;
jmethodID	MFVec3f::mGetSizeMethodID = 0;
jmethodID	MFVec3f::mClearMethodID = 0;
jmethodID	MFVec3f::mDeleteMethodID = 0;
jmethodID	MFVec3f::mAddValueMethodID = 0;
jmethodID	MFVec3f::mInsertValueMethodID = 0;
jmethodID	MFVec3f::mSet1ValueMethodID = 0;
jmethodID	MFVec3f::mGet1ValueMethodID = 0;
jmethodID	MFVec3f::mSetNameMethodID = 0;

jmethodID	MFVec3f::mConstInitMethodID = 0;
jmethodID	MFVec3f::mConstGetSizeMethodID = 0;
jmethodID	MFVec3f::mConstClearMethodID = 0;
jmethodID	MFVec3f::mConstDeleteMethodID = 0;
jmethodID	MFVec3f::mConstAddValueMethodID = 0;
jmethodID	MFVec3f::mConstInsertValueMethodID = 0;
jmethodID	MFVec3f::mConstSet1ValueMethodID = 0;
jmethodID	MFVec3f::mConstGet1ValueMethodID = 0;
jmethodID	MFVec3f::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	MFVec3f::setJavaIDs
////////////////////////////////////////////////

void MFVec3f::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/MFVec3f");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstMFVec3f");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid			= getFieldID();
		mInitMethodID			= jniEnv->GetMethodID(classid, "<init>",		"()V");
		mGetSizeMethodID		= jniEnv->GetMethodID(classid, "getSize",		"()I");
		mClearMethodID			= jniEnv->GetMethodID(classid, "clear",			"()V");
		mDeleteMethodID			= jniEnv->GetMethodID(classid, "delete",		"(I)V");
		mAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(FFF)V");
		mInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(IFFF)V");
		mSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(IFFF)V");
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
		mConstAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(FFF)V");
		mConstInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(IFFF)V");
		mConstSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(IFFF)V");
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
//	MFVec3f::toJavaObject
////////////////////////////////////////////////

jobject MFVec3f::toJavaObject(int bConstField) 
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
		float value[3];
		get1Value(n, value);
		jfloat x = value[0];
		jfloat y = value[1];
		jfloat z = value[2];
		jniEnv->CallVoidMethod(fieldObject, addValueMethod, x, y, z);
	}

	return fieldObject;
}

////////////////////////////////////////////////
//	MFVec3f::setValue
////////////////////////////////////////////////

void MFVec3f::setValue(jobject field, int bConstField) {
}

////////////////////////////////////////////////
//	MFVec3f::getValue
////////////////////////////////////////////////

void MFVec3f::getValue(jobject field, int bConstField) {
}

#endif 

