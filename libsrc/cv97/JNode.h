/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	JNode.h
*
******************************************************************/

#ifndef _CV97_JNODE_H_
#define _CV97_JNODE_H_

#ifdef SUPPORT_JSAI

#include "CJavaVM.h"
#include "Field.h"

class Node;

class JNode : public CJavaVM  {

	static int			mJavaIDsInit;

	static jclass		mNodeObjectClassID;
	static jmethodID	mInitMethodID;
	static jmethodID	mSetNameMethodID;

	static jmethodID	mAddEventInMethodID;
	static jmethodID	mAddEventOutMethodID;
	static jmethodID	mAddFieldMethodID;
	static jmethodID	mAddExposedFieldMethodID;

	static jmethodID	mGetEventInMethodID;
	static jmethodID	mGetEventOutMethodID;
	static jmethodID	mGetFieldMethodID;
	static jmethodID	mGetExposedFieldMethodID;

	static jmethodID	mGetNEventInMethodID;
	static jmethodID	mGetNEventOutMethodID;
	static jmethodID	mGetNFieldsMethodID;
	static jmethodID	mGetNExposedFieldsMethodID;

	jobject	mNodeObject;

public:

	JNode();
	JNode(Node *node);
	JNode(jobject nodeObject);

	~JNode();

	void		setJavaIDs();

	////////////////////////////////////////////////
	//	class ID	
	////////////////////////////////////////////////

	virtual		jclass		getNodeObjectClassID()			{return mNodeObjectClassID;}

	////////////////////////////////////////////////
	//	method ID
	////////////////////////////////////////////////

	virtual		jmethodID	getInitMethodID()				{return mInitMethodID;}
	virtual		jmethodID	getSetNameMethodID()			{return mSetNameMethodID;}

	virtual		jmethodID	getAddEventInMethodID()			{return mAddEventInMethodID;}
	virtual		jmethodID	getAddEventOutMethodID()		{return mAddEventOutMethodID;}
	virtual		jmethodID	getAddFieldMethodID()			{return mAddFieldMethodID;}
	virtual		jmethodID	getAddExposedFieldMethodID()	{return mAddExposedFieldMethodID;}

	virtual		jmethodID	getGetEventInMethodID()			{return mGetEventInMethodID;}
	virtual		jmethodID	getGetEventOutMethodID()		{return mGetEventOutMethodID;}
	virtual		jmethodID	getGetFieldMethodID()			{return mGetFieldMethodID;}
	virtual		jmethodID	getGetExposedFieldMethodID()	{return mGetExposedFieldMethodID;}

	virtual		jmethodID	getGetNEventInMethodID()		{return mGetNEventInMethodID;}
	virtual		jmethodID	getGetNEventOutMethodID()		{return mGetNEventOutMethodID;}
	virtual		jmethodID	getGetNFieldsMethodID()			{return mGetNFieldsMethodID;}
	virtual		jmethodID	getGetNExposedFieldsMethodID()	{return mGetNExposedFieldsMethodID;}

	////////////////////////////////////////////////
	//	Object	
	////////////////////////////////////////////////

	void		setNodeObject(jobject object)				{mNodeObject = object;}
	jobject		getNodeObject()								{return mNodeObject;}

	void		addFields(Node *node);

	////////////////////////////////////////////////
	//	add*	
	////////////////////////////////////////////////

	void addFieldObject(jmethodID id, Field *field, int bConstField = JAVAOBJECT_FIELD);

	void addEventIn(Field *field) {
		addFieldObject(getAddEventInMethodID(), field);
	}
	virtual void addEventOut(Field *field) {
		addFieldObject(getAddEventOutMethodID(), field, JAVAOBJECT_CONSTFIELD);
	}
	void addField(Field *field) {
		addFieldObject(getAddFieldMethodID(), field);
	}
	void addExposedField(Field *field) {
		addFieldObject(getAddExposedFieldMethodID(), field);
	}

	////////////////////////////////////////////////
	//	get*	
	////////////////////////////////////////////////

	jobject getFieldObject(jmethodID id, char *name);

	jobject	getEventIn(Field *field) {
		return getFieldObject(getGetEventInMethodID(), field->getName());
	}
	jobject	getEventOut(Field *field) {
		return getFieldObject(getGetEventOutMethodID(), field->getName());
	}
	jobject	getField(Field *field) {
		return getFieldObject(getGetFieldMethodID(), field->getName());
	}
	jobject	getExposedField(Field *field) {
		return getFieldObject(getGetExposedFieldMethodID(), field->getName());
	}

	////////////////////////////////////////////////
	//	getN*
	////////////////////////////////////////////////

	int			getNEventIn() {
		return 	getJniEnv()->CallIntMethod(getNodeObject(), getGetNEventInMethodID());
	}
	int			getNEventOut() {
		return 	getJniEnv()->CallIntMethod(getNodeObject(), getGetNEventOutMethodID());
	}
	int			getNFields() {
		return 	getJniEnv()->CallIntMethod(getNodeObject(), getGetNFieldsMethodID());
	}
	int			getNExposedFields() {
		return 	getJniEnv()->CallIntMethod(getNodeObject(), getGetNExposedFieldsMethodID());
	}

	////////////////////////////////////////////////
	//	set/getValue	
	////////////////////////////////////////////////

	virtual void setValue(Node *node);
	virtual void getValue(Node *node);
};

#endif

#endif

