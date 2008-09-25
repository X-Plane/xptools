/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MFColor.cpp
*
******************************************************************/

#include "MFColor.h"

MFColor::MFColor()
{
	setType(fieldTypeMFColor);
	InitializeJavaIDs();
}

void MFColor::InitializeJavaIDs()
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

void MFColor::addValue(float r, float g, float b)
{
	SFColor *color = new SFColor(r, g, b);
	add(color);
}

void MFColor::addValue(float value[])
{
	SFColor *color = new SFColor(value);
	add(color);
}

void MFColor::addValue(SFColor *color)
{
	add(color);
}

void MFColor::insertValue(int index, float r, float g, float b)
{
	SFColor *color = new SFColor(r, g, b);
	insert(color, index);
}

void MFColor::insertValue(int index, float value[])
{
	SFColor *color = new SFColor(value);
	insert(color, index);
}

void MFColor::insertValue(int index, SFColor *color)
{
	insert(color, index);
}

void MFColor::get1Value(int index, float value[])
{
	SFColor *color = (SFColor *)getObject(index);
	if (color)
		color->getValue(value);
}

void MFColor::set1Value(int index, float value[])
{
	SFColor *color = (SFColor *)getObject(index);
	if (color)
		color->setValue(value);
}

void MFColor::set1Value(int index, float r, float g, float b)
{
	SFColor *color = (SFColor *)getObject(index);
	if (color)
		color->setValue(r, g, b);
}

void MFColor::setValue(MFColor *colors)
{
	clear();

	float value[3];
	int size = colors->getSize();
	for (int n=0; n<size; n++) {
		colors->get1Value(n, value);
		addValue(value);
	}
}

void MFColor::setValue(MField *mfield)
{
	if (mfield->getType() == fieldTypeMFColor)
		setValue((MFColor *)mfield);
}

void MFColor::setValue(int size, float colors[][3])
{
	clear();

	for (int n=0; n<size; n++)
		addValue(colors[n]);
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

void MFColor::outputContext(ostream& printStream, char *indentString)
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

int			MFColor::mInit = 0;

jclass		MFColor::mFieldClassID = 0;
jclass		MFColor::mConstFieldClassID = 0;

jmethodID	MFColor::mInitMethodID = 0;
jmethodID	MFColor::mGetSizeMethodID = 0;
jmethodID	MFColor::mClearMethodID = 0;
jmethodID	MFColor::mDeleteMethodID = 0;
jmethodID	MFColor::mAddValueMethodID = 0;
jmethodID	MFColor::mInsertValueMethodID = 0;
jmethodID	MFColor::mSet1ValueMethodID = 0;
jmethodID	MFColor::mGet1ValueMethodID = 0;
jmethodID	MFColor::mSetNameMethodID = 0;

jmethodID	MFColor::mConstInitMethodID = 0;
jmethodID	MFColor::mConstGetSizeMethodID = 0;
jmethodID	MFColor::mConstClearMethodID = 0;
jmethodID	MFColor::mConstDeleteMethodID = 0;
jmethodID	MFColor::mConstAddValueMethodID = 0;
jmethodID	MFColor::mConstInsertValueMethodID = 0;
jmethodID	MFColor::mConstSet1ValueMethodID = 0;
jmethodID	MFColor::mConstGet1ValueMethodID = 0;
jmethodID	MFColor::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	MFColor::setJavaIDs
////////////////////////////////////////////////

void MFColor::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/MFColor");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstMFColor");

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
//	MFColor.cpp::toJavaObject
////////////////////////////////////////////////

jobject MFColor::toJavaObject(int bConstField)
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
		jfloat r = value[0];
		jfloat g = value[0];
		jfloat b = value[0];
		jniEnv->CallVoidMethod(fieldObject, addValueMethod, r, g, b);
	}

	return fieldObject;
}

////////////////////////////////////////////////
//	MFColor::setValue
////////////////////////////////////////////////

void MFColor::setValue(jobject field, int bConstField)
{
	assert(field);
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	getSizeMethod	= bConstField ? getConstGetSizeMethodID() : getGetSizeMethodID();
	jmethodID	getValueMethod	= bConstField ? getConstGet1ValueMethodID() : getGet1ValueMethodID();
	assert(classid && getValueMethod);

	jfloatArray valueArray = jniEnv->NewFloatArray(3);
    jfloat		value[3];

	jint	jsize	= jniEnv->CallIntMethod(field, getSizeMethod);
	int		size	= getSize();

	for (int n=0; n<jsize; n++) {
		jniEnv->CallVoidMethod(field, getValueMethod, n, valueArray);
		jniEnv->GetFloatArrayRegion(valueArray, 0, 3, value);
		if (n < size)
			set1Value(0, value[0], value[1], value[2]);
		else
			addValue(value[0], value[1], value[2]);
	}

	jniEnv->DeleteLocalRef(valueArray);
}

////////////////////////////////////////////////
//	MFColor::getValue
////////////////////////////////////////////////

void MFColor::getValue(jobject field, int bConstField)
{
}

#endif

