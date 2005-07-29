/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	JNode.cpp
*
******************************************************************/

#ifdef SUPPORT_JSAI

#include "JNode.h"
#include "Node.h"

int			JNode::mJavaIDsInit = 0;
jclass		JNode::mNodeObjectClassID = 0;
jmethodID	JNode::mInitMethodID = 0;
jmethodID	JNode::mSetNameMethodID = 0;
jmethodID	JNode::mAddEventInMethodID = 0;
jmethodID	JNode::mAddEventOutMethodID = 0;
jmethodID	JNode::mAddFieldMethodID = 0;
jmethodID	JNode::mAddExposedFieldMethodID = 0;
jmethodID	JNode::mGetEventInMethodID = 0;
jmethodID	JNode::mGetEventOutMethodID = 0;
jmethodID	JNode::mGetFieldMethodID = 0;
jmethodID	JNode::mGetExposedFieldMethodID = 0;
jmethodID	JNode::mGetNEventInMethodID = 0;
jmethodID	JNode::mGetNEventOutMethodID = 0;
jmethodID	JNode::mGetNFieldsMethodID = 0;
jmethodID	JNode::mGetNExposedFieldsMethodID = 0;

////////////////////////////////////////////////
//	JNode::JNode
////////////////////////////////////////////////

JNode::JNode(Node *node)
{
	setJavaIDs();

	JNIEnv		*jniEnv			= getJniEnv();
	jclass		classid			= getNodeObjectClassID();
	jmethodID	initMethod		= getInitMethodID();

	mNodeObject	= jniEnv->NewObject(classid, initMethod);

	if (node)
		addFields(node);
}

JNode::JNode(jobject nodeObject)
{
	setJavaIDs();
	mNodeObject = nodeObject;
}

JNode::JNode()
{
	mNodeObject				= NULL;
}

////////////////////////////////////////////////
//	JNode::addFields
////////////////////////////////////////////////

void JNode::addFields(Node *node)
{
	int	n;

	int nEventIn = node->getNEventIn();
	for (n=0; n<nEventIn; n++)
		addEventIn(node->getEventIn(n));

	int nEventOut = node->getNEventOut();
	for (n=0; n<nEventOut; n++)
		addEventOut(node->getEventOut(n));

	int nField = node->getNFields();
	for (n=0; n<nField; n++)
		addField(node->getField(n));

	int nExposedField = node->getNExposedFields();
	for (n=0; n<nExposedField; n++)
		addExposedField(node->getExposedField(n));
}

////////////////////////////////////////////////
//	JNode::~JNode
////////////////////////////////////////////////

JNode::~JNode()
{
	getJniEnv()->DeleteLocalRef(getNodeObject());
}

////////////////////////////////////////////////
//	JNode::setJavaIDs
////////////////////////////////////////////////

void JNode::setJavaIDs() {

	if (!mJavaIDsInit) {
		JNIEnv *jniEnv = getJniEnv();

		// Class IDs
		mNodeObjectClassID	= jniEnv->FindClass("vrml/node/NodeObject");

		// MethodIDs
		jclass classid				= getNodeObjectClassID();
		mInitMethodID				= jniEnv->GetMethodID(classid, "<init>",	"()V");
		mSetNameMethodID			= jniEnv->GetMethodID(classid, "setName",	"(Ljava/lang/String;)V");

		mAddEventInMethodID			= jniEnv->GetMethodID(classid, "addEventIn",		"(Ljava/lang/String;Lvrml/Field;)V");
		mAddEventOutMethodID		= jniEnv->GetMethodID(classid, "addEventOut",		"(Ljava/lang/String;Lvrml/ConstField;)V");
		mAddFieldMethodID			= jniEnv->GetMethodID(classid, "addField",			"(Ljava/lang/String;Lvrml/Field;)V");
		mAddExposedFieldMethodID	= jniEnv->GetMethodID(classid, "addExposedField",	"(Ljava/lang/String;Lvrml/Field;)V");

		mGetEventInMethodID			= jniEnv->GetMethodID(classid, "getEventIn",		"(Ljava/lang/String;)Lvrml/Field;");
		mGetEventOutMethodID		= jniEnv->GetMethodID(classid, "getEventOut",		"(Ljava/lang/String;)Lvrml/ConstField;");
		mGetFieldMethodID			= jniEnv->GetMethodID(classid, "getField",			"(Ljava/lang/String;)Lvrml/Field;");
		mGetExposedFieldMethodID	= jniEnv->GetMethodID(classid, "getExposedField",	"(Ljava/lang/String;)Lvrml/Field;");

		mGetNEventInMethodID		= jniEnv->GetMethodID(classid, "getNEventIn",		"()I");
		mGetNEventOutMethodID		= jniEnv->GetMethodID(classid, "getNEventOut",		"()I");
		mGetNFieldsMethodID			= jniEnv->GetMethodID(classid, "getNFields",		"()I");
		mGetNExposedFieldsMethodID	= jniEnv->GetMethodID(classid, "getNExposedFields",	"()I");

		assert(mInitMethodID && mSetNameMethodID);
		assert(mAddEventInMethodID && mAddEventOutMethodID && mAddFieldMethodID && mAddExposedFieldMethodID);
		assert(mGetEventInMethodID && mGetEventOutMethodID && mGetFieldMethodID && mGetExposedFieldMethodID);
		assert(mGetNEventInMethodID && mGetNEventOutMethodID && mGetNFieldsMethodID && mGetNExposedFieldsMethodID);

		mJavaIDsInit = 1;
	}
}

////////////////////////////////////////////////
//	add*	
////////////////////////////////////////////////

void JNode::addFieldObject(jmethodID id, Field *field, int bConstField) {
	char *name = field->getName();
	if (id && name && strlen(name) > 0 && field) {
		jstring value	= getJniEnv()->NewStringUTF(name);
		jobject	jfield	= field->toJavaObject(bConstField);
		assert(value && jfield);
		getJniEnv()->CallVoidMethod(getNodeObject(), id, value, jfield);
		getJniEnv()->DeleteLocalRef(value);
		getJniEnv()->DeleteLocalRef(jfield);
	}
}

///////////////////////////////////////////////
//	get*	
////////////////////////////////////////////////

jobject JNode::getFieldObject(jmethodID id, char *name) {
	assert(id && name && strlen(name) > 0);
	jstring value	= getJniEnv()->NewStringUTF(name);
	jobject jfield = getJniEnv()->CallObjectMethod(getNodeObject(), id, value);
	getJniEnv()->DeleteLocalRef(value);
	return jfield;
}

////////////////////////////////////////////////
//	JNode::setValue
////////////////////////////////////////////////

void JNode::setValue(Node *node)
{
	if (!node)
		return;

	Field	*field;
	jobject	jfield;
	int		n;

	int nEventIn = node->getNEventIn();
	for (n=0; n<nEventIn; n++) {
		field	= node->getEventIn(n);
		jfield	= getEventIn(field);
		assert(jfield && field);
		field->getValue(jfield);
	}

	int nEventOut = node->getNEventOut();
	for (n=0; n<nEventOut; n++) {
		field	= node->getEventOut(n);
		jfield = getEventOut(field);
		assert(jfield && field);
		field->getValue(jfield, JAVAOBJECT_CONSTFIELD);
	}

	int nField = node->getNFields();
	for (n=0; n<nField; n++) {
		field	= node->getField(n);
		jfield = getField(field);
		assert(jfield && field);
		field->getValue(jfield);
	}

	int nExposedField = node->getNExposedFields();
	for (n=0; n<nExposedField; n++) {
		field	= node->getExposedField(n);
		jfield	= getExposedField(field);
		assert(jfield && field);
		field->getValue(jfield);
	}
}


////////////////////////////////////////////////
//	JNode::getValue
////////////////////////////////////////////////

void JNode::getValue(Node *node)
{
	if (!node)
		return;

	Field	*field;
	jobject	jfield;
	int		n;

	int nEventIn = node->getNEventIn();
	for (n=0; n<nEventIn; n++) {
		field	= node->getEventIn(n);
		jfield	= getEventIn(field);
		assert(jfield && field);
		field->setValue(jfield);
	}

	int nEventOut = node->getNEventOut();
	for (n=0; n<nEventOut; n++) {
		field	= node->getEventOut(n);
		jfield = getEventOut(field);
		assert(jfield && field);
		field->setValue(jfield, JAVAOBJECT_CONSTFIELD);
	}

	int nField = node->getNFields();
	for (n=0; n<nField; n++) {
		field	= node->getField(n);
		jfield = getField(field);
		assert(jfield && field);
		field->setValue(jfield);
	}

	int nExposedField = node->getNExposedFields();
	for (n=0; n<nExposedField; n++) {
		field	= node->getExposedField(n);
		jfield	= getExposedField(field);
		assert(jfield && field);
		field->setValue(jfield);
	}
}

#endif