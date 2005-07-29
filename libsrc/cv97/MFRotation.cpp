/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MFRotation.cpp
*
******************************************************************/

#include "MFRotation.h"

MFRotation::MFRotation() 
{
	setType(fieldTypeMFRotation);
	InitializeJavaIDs();
}

void MFRotation::InitializeJavaIDs() 
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void MFRotation::addValue(float x, float y, float z, float rot) 
{
	SFRotation *rotation = new SFRotation(x, y, z, rot);
	add(rotation);
}

void MFRotation::addValue(float value[]) 
{
	SFRotation *rotation = new SFRotation(value);
	add(rotation);
}

void MFRotation::addValue(SFRotation *rotation) 
{
	add(rotation);
}

void MFRotation::insertValue(int index, float x, float y, float z, float rot) 
{
	SFRotation *rotation = new SFRotation(x, y, z, rot);
	insert(rotation, index);
}

void MFRotation::insertValue(int index, float value[]) 
{
	SFRotation *rotation = new SFRotation(value);
	insert(rotation, index);
}

void MFRotation::insertValue(int index, SFRotation *rotation) 
{
	insert(rotation, index);
}

void MFRotation::get1Value(int index, float value[]) 
{
	SFRotation *rotation = (SFRotation *)getObject(index);
	if (rotation)
		rotation->getValue(value);
}

void MFRotation::set1Value(int index, float value[]) 
{
	SFRotation *rotation = (SFRotation *)getObject(index);
	if (rotation)
		rotation->setValue(value);
}

void MFRotation::set1Value(int index, float x, float y, float z, float angle) 
{
	SFRotation *rotation = (SFRotation *)getObject(index);
	if (rotation)
		rotation->setValue(x, y, z, angle);
}

void MFRotation::setValue(MFRotation *rotations)
{
	clear();

	float value[3];
	int size = rotations->getSize();
	for (int n=0; n<size; n++) {
		rotations->get1Value(n, value);
		addValue(value);
	}
}

void MFRotation::setValue(MField *mfield)
{
	if (mfield->getType() == fieldTypeMFRotation)
		setValue((MFRotation *)mfield);
}

void MFRotation::setValue(int size, float rotations[][4])
{
	clear();

	for (int n=0; n<size; n++)
		addValue(rotations[n]);
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void MFRotation::outputContext(ostream& printStream, char *indentString) 
{
	float value[4];
	for (int n=0; n<getSize(); n++) {
		get1Value(n, value);
		if (n < getSize()-1)
			printStream << indentString << value[0] << " " << value[1] << " " << value[2] << " " << value[3] << "," << endl;
		else	
			printStream << indentString << value[0] << " " << value[1] << " " << value[2] << " " << value[3] << endl;
	}
}

////////////////////////////////////////////////
//	JSAI
////////////////////////////////////////////////


#ifdef SUPPORT_JSAI

int			MFRotation::mInit = 0;

jclass		MFRotation::mFieldClassID = 0;
jclass		MFRotation::mConstFieldClassID = 0;

jmethodID	MFRotation::mInitMethodID = 0;
jmethodID	MFRotation::mGetSizeMethodID = 0;
jmethodID	MFRotation::mClearMethodID = 0;
jmethodID	MFRotation::mDeleteMethodID = 0;
jmethodID	MFRotation::mAddValueMethodID = 0;
jmethodID	MFRotation::mInsertValueMethodID = 0;
jmethodID	MFRotation::mSet1ValueMethodID = 0;
jmethodID	MFRotation::mGet1ValueMethodID = 0;
jmethodID	MFRotation::mSetNameMethodID = 0;

jmethodID	MFRotation::mConstInitMethodID = 0;
jmethodID	MFRotation::mConstGetSizeMethodID = 0;
jmethodID	MFRotation::mConstClearMethodID = 0;
jmethodID	MFRotation::mConstDeleteMethodID = 0;
jmethodID	MFRotation::mConstAddValueMethodID = 0;
jmethodID	MFRotation::mConstInsertValueMethodID = 0;
jmethodID	MFRotation::mConstSet1ValueMethodID = 0;
jmethodID	MFRotation::mConstGet1ValueMethodID = 0;
jmethodID	MFRotation::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	MFRotation::setJavaIDs
////////////////////////////////////////////////

void MFRotation::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/MFRotation");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstMFRotation");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid			= getFieldID();
		mInitMethodID			= jniEnv->GetMethodID(classid, "<init>",		"()V");
		mGetSizeMethodID		= jniEnv->GetMethodID(classid, "getSize",		"()I");
		mClearMethodID			= jniEnv->GetMethodID(classid, "clear",			"()V");
		mDeleteMethodID			= jniEnv->GetMethodID(classid, "delete",		"(I)V");
		mAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(FFFF)V");
		mInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(IFFFF)V");
		mSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(IFFFF)V");
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
		mConstAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(FFFF)V");
		mConstInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(IFFFF)V");
		mConstSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(IFFFF)V");
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
//	MFRotation::toJavaObject
////////////////////////////////////////////////

jobject MFRotation::toJavaObject(int bConstField) 
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
		float value[4];
		get1Value(n, value);
		jfloat x = value[0];
		jfloat y = value[1];
		jfloat z = value[2];
		jfloat a = value[3];
		jniEnv->CallVoidMethod(fieldObject, addValueMethod, x, y, z, a);
	}

	return fieldObject;
}

////////////////////////////////////////////////
//	MFRotation::setValue
////////////////////////////////////////////////

void MFRotation::setValue(jobject field, int bConstField) {
}

////////////////////////////////////////////////
//	MFRotation::getValue
////////////////////////////////////////////////

void MFRotation::getValue(jobject field, int bConstField) {
}

#endif 

