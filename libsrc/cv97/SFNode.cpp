/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	SFNode.cpp
*
******************************************************************/

#include "SFNode.h"
#include "JNode.h"
#include "Node.h"

SFNode::SFNode()
{
	setType(fieldTypeSFNode);
	setValue((Node *)NULL);
	InitializeJavaIDs();
}

SFNode::SFNode(Node *value)
{
	setType(fieldTypeSFNode);
	setValue(value);
	InitializeJavaIDs();
}

SFNode::SFNode(SFNode *value)
{
	setType(fieldTypeSFNode);
	setValue(value);
	InitializeJavaIDs();
}

void SFNode::InitializeJavaIDs()
{
#ifdef SUPPORT_JSAI
	setJavaIDs();
#endif
}

SFNode::~SFNode()
{
}

void SFNode::setValue(Node *value)
{
	mValue = value;
}

void SFNode::setValue(SFNode *value)
{
	mValue = value->getValue();
}

void SFNode::setValue(char *buffer)
{
}

char *SFNode::getValue(char *buffer, int bufferLen)
{
	sprintf(buffer, "%s", getValue()->getName());
	return buffer;
}

Node *SFNode::getValue()
{
	return mValue;
}

////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////

ostream& operator<<(ostream &s, SFNode &node)
{
	return s;
}

ostream& operator<<(ostream &s, SFNode *node)
{
	return s;
}

////////////////////////////////////////////////
//	Compare
////////////////////////////////////////////////

bool SFNode::equals(Field *field)
{
	SFNode *nodeField = (SFNode *)field;
	if (getValue() == nodeField->getValue())
		return true;
	else
		return false;
}

////////////////////////////////////////////////
//	Java
////////////////////////////////////////////////

#ifdef SUPPORT_JSAI

int			SFNode::mInit = 0;

jclass		SFNode::mFieldClassID = 0;
jclass		SFNode::mConstFieldClassID = 0;

jmethodID	SFNode::mInitMethodID = 0;
jmethodID	SFNode::mSetValueMethodID = 0;
jmethodID	SFNode::mGetValueMethodID = 0;
jmethodID	SFNode::mSetNameMethodID = 0;

jmethodID	SFNode::mConstInitMethodID = 0;
jmethodID	SFNode::mConstSetValueMethodID = 0;
jmethodID	SFNode::mConstGetValueMethodID = 0;
jmethodID	SFNode::mConstSetNameMethodID = 0;

////////////////////////////////////////////////
//	SFNode::toJavaObject
////////////////////////////////////////////////

void SFNode::setJavaIDs() {

	if (!mInit) {
		JNIEnv *jniEnv = getJniEnv();

		if (jniEnv == NULL)
			return;

		// Class IDs
		mFieldClassID		= jniEnv->FindClass("vrml/field/SFNode");
		mConstFieldClassID	= jniEnv->FindClass("vrml/field/ConstSFNode");

		assert(mFieldClassID && mConstFieldClassID);

		// MethodIDs
		jclass classid = getFieldID();
		mInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(Lvrml/BaseNode;)V");
		mGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()Lvrml/BaseNode;");
		mSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(Lvrml/BaseNode;)V");
		mSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mInitMethodID && mGetValueMethodID && mSetValueMethodID && mSetNameMethodID);

		// MethodIDs
		classid	 = getConstFieldID();
		mConstInitMethodID		= jniEnv->GetMethodID(classid, "<init>", "(Lvrml/BaseNode;)V");
		mConstGetValueMethodID	= jniEnv->GetMethodID(classid, "getValue", "()Lvrml/BaseNode;");
		mConstSetValueMethodID	= jniEnv->GetMethodID(classid, "setValue", "(Lvrml/BaseNode;)V");
		mConstSetNameMethodID	= jniEnv->GetMethodID(classid, "setName", "(Ljava/lang/String;)V");

		assert(mConstInitMethodID && mConstGetValueMethodID && mConstSetValueMethodID && mConstSetNameMethodID);

		mInit = 1;
	}
}

////////////////////////////////////////////////
//	SFNode::toJavaObject
////////////////////////////////////////////////

jobject SFNode::toJavaObject(int bConstField) {
	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= bConstField ? getConstFieldID() : getFieldID();
	jmethodID	initMethod		= bConstField ? getConstInitMethodID() : getInitMethodID();
	JNode		*node			= new JNode(getValue());
	jobject		jnode			= node->getNodeObject();
	jobject		eventField		= jniEnv->NewObject(classid, initMethod, jnode);
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
//	SFNode::setValue
////////////////////////////////////////////////

void SFNode::setValue(jobject field, int bConstField) {
	Node	*node = getValue();

	JNIEnv		*jniEnv			= getJniEnv();
	jmethodID	getValueMethod	= bConstField ? getConstGetValueMethodID() : getGetValueMethodID();
	jobject value = jniEnv->CallObjectMethod(field, getValueMethod);

	JNode	jnode(value);
	jnode.getValue(node);
}

////////////////////////////////////////////////
//	SFNode::getValue
////////////////////////////////////////////////

void SFNode::getValue(jobject field, int bConstField) {
	Node	*node = getValue();

	JNIEnv		*jniEnv			= getJniEnv();
	jmethodID	getValueMethod	= bConstField ? getConstGetValueMethodID() : getGetValueMethodID();
	jobject value = jniEnv->CallObjectMethod(field, getValueMethod);

	JNode	jnode(value);
	jnode.setValue(node);
}

#endif



