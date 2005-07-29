/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MFString.cpp
*
******************************************************************/

#include "MFString.h"

MFString::MFString() 
{
	setType(fieldTypeMFString);
	InitializeJavaIDs();
}

void MFString::InitializeJavaIDs() 
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void MFString::addValue(char *value) 
{
	SFString *sfvalue = new SFString(value);
	add(sfvalue);
}

void MFString::addValue(SFString *sfvalue) 
{
	add(sfvalue);
}

void MFString::insertValue(int index, char *value) 
{
	SFString *sfvalue = new SFString(value);
	insert(sfvalue, index);
}

char *MFString::get1Value(int index) 
{
	SFString *sfvalue = (SFString *)getObject(index);
	if (sfvalue)
		return sfvalue->getValue();
	else
		return NULL;
}

void MFString::set1Value(int index, char *value) 
{
	SFString *sfvalue = (SFString *)getObject(index);
	if (sfvalue)
		sfvalue->setValue(value);
}

void MFString::setValue(MFString *values)
{
	clear();

	int size = values->getSize();
	for (int n=0; n<size; n++) {
		addValue(values->get1Value(n));
	}
}

void MFString::setValue(MField *mfield)
{
	if (mfield->getType() == fieldTypeMFString)
		setValue((MFString *)mfield);
}

void MFString::setValue(int size, char *values[])
{
	clear();

	for (int n=0; n<size; n++)
		addValue(values[n]);
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void MFString::outputContext(ostream& printStream, char *indentString) 
{
	for (int n=0; n<getSize(); n++) {
		char *value = get1Value(n);
		if (value) {
			if (n < getSize()-1)
				printStream << indentString << "\"" << get1Value(n) << "\"" << "," << endl;
			else	
				printStream << indentString << "\"" << get1Value(n) << "\"" << endl;
		}
		else {
			if (n < getSize()-1)
				printStream << indentString << "\"" << "\"" << "," << endl;
			else	
				printStream << indentString << "\"" << "\"" << endl;
		}
	}
}

////////////////////////////////////////////////
//	JSAI
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			MFString::mInit = 0;

jclass		MFString::mFieldClassID = 0;
jclass		MFString::mConstFieldClassID = 0;

jmethodID	MFString::mInitMethodID = 0;
jmethodID	MFString::mGetSizeMethodID = 0;
jmethodID	MFString::mClearMethodID = 0;
jmethodID	MFString::mDeleteMethodID = 0;
jmethodID	MFString::mAddValueMethodID = 0;
jmethodID	MFString::mInsertValueMethodID = 0;
jmethodID	MFString::mSet1ValueMethodID = 0;
jmethodID	MFString::mGet1ValueMethodID = 0;
jmethodID	MFString::mSetNameMethodID = 0;

jmethodID	MFString::mConstInitMethodID = 0;
jmethodID	MFString::mConstGetSizeMethodID = 0;
jmethodID	MFString::mConstClearMethodID = 0;
jmethodID	MFString::mConstDeleteMethodID = 0;
jmethodID	MFString::mConstAddValueMethodID = 0;
jmethodID	MFString::mConstInsertValueMethodID = 0;
jmethodID	MFString::mConstSet1ValueMethodID = 0;
jmethodID	MFString::mConstGet1ValueMethodID = 0;
jmethodID	MFString::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	MFString::setJavaIDs
////////////////////////////////////////////////

void MFString::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/MFString");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstMFString");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid			= getFieldID();
		mInitMethodID			= jniEnv->GetMethodID(classid, "<init>",		"()V");
		mGetSizeMethodID		= jniEnv->GetMethodID(classid, "getSize",		"()I");
		mClearMethodID			= jniEnv->GetMethodID(classid, "clear",			"()V");
		mDeleteMethodID			= jniEnv->GetMethodID(classid, "delete",		"(I)V");
		mAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(Ljava/lang/String;)V");
		mInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(ILjava/lang/String;)V");
		mSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(ILjava/lang/String;)V");
		mGet1ValueMethodID		= jniEnv->GetMethodID(classid, "get1Value",		"(I)Ljava/lang/String;");
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
		mConstAddValueMethodID		= jniEnv->GetMethodID(classid, "addValue",		"(Ljava/lang/String;)V");
		mConstInsertValueMethodID	= jniEnv->GetMethodID(classid, "insertValue",	"(ILjava/lang/String;)V");
		mConstSet1ValueMethodID		= jniEnv->GetMethodID(classid, "set1Value",		"(ILjava/lang/String;)V");
		mConstGet1ValueMethodID		= jniEnv->GetMethodID(classid, "get1Value",		"(I)Ljava/lang/String;");
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
//	MFString::toJavaObject
////////////////////////////////////////////////

jobject MFString::toJavaObject(int bConstField) {
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
		char *value = get1Value(n);
		jstring jvalue = NULL;
		if (value && strlen(value))
			jvalue = jniEnv->NewStringUTF(value);
		jniEnv->CallVoidMethod(fieldObject, addValueMethod, jvalue);
		if (jvalue)
			jniEnv->ReleaseStringUTFChars(jvalue, jniEnv->GetStringUTFChars(jvalue, NULL));
	}

	return fieldObject;
}

void MFString::setValue(jobject field, int bConstField) 
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
		jstring value = (jstring)jniEnv->CallObjectMethod(field, getValueMethod, n);
		if (value) {
			const char	*string	= jniEnv->GetStringUTFChars(value, NULL);
			if (n < size)
				set1Value(n, (char *)string);
			else
				addValue((char *)string);
			jniEnv->ReleaseStringUTFChars(value, string);	
			jniEnv->DeleteLocalRef(value);
		}
		else {
			if (n < size)
				set1Value(n, (char *)NULL);
			else
				addValue((char *)NULL);
		}
	}
}

void MFString::getValue(jobject field, int bConstField) {
}

#endif 
