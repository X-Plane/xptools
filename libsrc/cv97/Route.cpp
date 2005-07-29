/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Route.cpp
*
******************************************************************/

#include "SceneGraph.h"

Route::Route(Node *eventOutNode, Field *eventOutField, Node *eventInNode, Field *eventInField) 
{
	setHeaderFlag(false);
	setEventOutNode(eventOutNode);
	setEventInNode(eventInNode);
	setEventOutField(eventOutField);
	setEventInField(eventInField);
	initialize();
}

Route::Route(Route *route) 
{
	setHeaderFlag(false);
	setEventOutNode(route->getEventOutNode());
	setEventInNode(route->getEventInNode());
	setEventOutField(route->getEventOutField());
	setEventInField(route->getEventInField());
	initialize();
}

Route::~Route() 
{ 
	remove();
}

////////////////////////////////////////////////
//	Value
////////////////////////////////////////////////

void Route::setEventOutNode(Node *node)
{
	mEventOutNode = node; 
}

void Route::setEventInNode(Node *node)
{ 
	mEventInNode = node; 
}

Node *Route::getEventOutNode()
{ 
	return mEventOutNode; 
}

Node *Route::getEventInNode()
{ 
	return mEventInNode; 
}

void Route::setEventOutField(Field *field)
{ 
	mEventOutField = field; 
}

Field *Route::getEventOutField()
{ 
	return mEventOutField; 
}

void Route::setEventInField(Field *field)
{ 
	mEventInField = field; 
}

Field *Route::getEventInField()
{ 
	return mEventInField; 
}

////////////////////////////////////////////////
//	Active
////////////////////////////////////////////////

void Route::setIsActive(bool active) 
{
	mIsActive = active;
}

bool Route::isActive() 
{
	return mIsActive;
}

////////////////////////////////////////////////
//	update
////////////////////////////////////////////////

void Route::initialize() 
{
	setIsActive(false);
}

////////////////////////////////////////////////
//	value
////////////////////////////////////////////////

void Route::setValue(void *value) 
{ 
	mValue = value; 
}

void *Route::getValue() 
{
	return mValue; 
}

////////////////////////////////////////////////
//	output
////////////////////////////////////////////////

void Route::output(ostream& printStream) 
{
	if (!getEventOutNode() && !getEventOutField() && !getEventInNode() && !getEventInField())
		return;

	printStream << "ROUTE ";
	printStream << getEventOutNode()->getName() << "." << getEventOutField()->getName() << " TO ";
	printStream << getEventInNode()->getName() << "." << getEventInField()->getName();
	printStream << endl; 
}

////////////////////////////////////////////////
//	Route::update
////////////////////////////////////////////////

void Route::update() 
{

	Field	*eventOutField	= getEventOutField();
	Field	*eventInField	= getEventInField();

	if (eventOutField == NULL || eventInField == NULL)
		return;

	if (isActive()) {
		if (eventOutField->equals(eventInField))
			return;
	}
	else
		setIsActive(true);

	int eventOutFieldType	= eventOutField->getType();
	int eventInFieldType	= eventInField->getType();	
	switch (eventOutFieldType) {

	////////////////////////////////////////////////
	//	SField
	////////////////////////////////////////////////

	case fieldTypeSFBool :
		{
			SFBool *boolOut = (SFBool *)eventOutField;
			bool value = boolOut->getValue();
			switch (eventInFieldType) {
			case fieldTypeSFBool :
				{
					SFBool *boolIn = (SFBool *)eventInField;
					boolIn->setValue(value);
				}
				break;
			}
		}
		break;

	case fieldTypeSFFloat :
		{
			SFFloat *fieldOut = (SFFloat *)eventOutField;
			float value = fieldOut->getValue();
			switch (eventInFieldType) {
			case fieldTypeSFFloat :
				{
					SFFloat *fieldIn = (SFFloat *)eventInField;
					fieldIn->setValue(value);
				}
				break;
			}
		}
		break;

	case fieldTypeSFInt32 :
		{
			SFInt32 *fieldOut = (SFInt32 *)eventOutField;
			int value = fieldOut->getValue();
			switch (eventInFieldType) {
			case fieldTypeSFInt32 :
				{
					SFInt32 *fieldIn = (SFInt32 *)eventInField;
					fieldIn->setValue(value);
				}
				break;
			}
		}
		break;

	case fieldTypeSFTime :
		{
			SFTime *fieldOut = (SFTime *)eventOutField;
			double value = fieldOut->getValue();
			switch (eventInFieldType) {
			case fieldTypeSFTime :
				{
					SFTime *fieldIn = (SFTime *)eventInField;
					fieldIn->setValue(value);
				}
				break;
			}
		}
		break;

	case fieldTypeSFString :
		{
			SFString *fieldOut = (SFString *)eventOutField;
			char *value = fieldOut->getValue();
			switch (eventInFieldType) {
			case fieldTypeSFString :
				{
					SFString *fieldIn = (SFString *)eventInField;
					fieldIn->setValue(value);
				}
				break;
			}
		}
		break;

	case fieldTypeSFVec2f :
		{
			SFVec2f *fieldOut = (SFVec2f *)eventOutField;
			float value[2];
			fieldOut->getValue(value);
			switch (eventInFieldType) {
			case fieldTypeSFVec2f :
				{
					SFVec2f *fieldIn = (SFVec2f *)eventInField;
					fieldIn->setValue(value);
				}
				break;
			}
		}
		break;

	case fieldTypeSFVec3f :
		{
			SFVec3f *fieldOut = (SFVec3f *)eventOutField;
			float value[3];
			fieldOut->getValue(value);
			switch (eventInFieldType) {
			case fieldTypeSFVec3f :
				{
					SFVec3f *fieldIn = (SFVec3f *)eventInField;
					fieldIn->setValue(value);
				}
				break;
			}
		}
		break;

	case fieldTypeSFColor :
		{
			SFColor *fieldOut = (SFColor *)eventOutField;
			float value[3];
			fieldOut->getValue(value);
			switch (eventInFieldType) {
			case fieldTypeSFColor :
				{
					SFColor *fieldIn = (SFColor *)eventInField;
					fieldIn->setValue(value);
				}
				break;
			}
		}
		break;

	case fieldTypeSFRotation :
		{
			SFRotation *fieldOut = (SFRotation *)eventOutField;
			float value[4];
			fieldOut->getValue(value);
			switch (eventInFieldType) {
			case fieldTypeSFRotation :
				{
					SFRotation *fieldIn = (SFRotation *)eventInField;
					fieldIn->setValue(value);
				}
				break;
			}
		}
		break;

	////////////////////////////////////////////////
	//	MField
	////////////////////////////////////////////////
/*
	case fieldTypeMFNode :
		{
			MFNode outNode = (MFNode)eventOutField;
			switch (eventInFieldType) {
			case fieldTypeMFNode :
				{
					MFNode inNode = (MFNode)eventInField;
					inNode->copy(outNode);
				}
				break;
			}
		}
		break;
*/	
	case fieldTypeMFString :
		{
			MFString *outString = (MFString *)eventOutField;
			switch (eventInFieldType) {
			case fieldTypeMFString :
				{
					MFString *inString = (MFString *)eventInField;
					inString->copy(outString);
				}
				break;
			}
		}
		break;

	case fieldTypeMFColor :
		{
			MFColor *outColor = (MFColor *)eventOutField;
			switch (eventInFieldType) {
			case fieldTypeMFColor :
				{
					MFColor *inColor = (MFColor *)eventInField;
					inColor->copy(outColor);
				}
				break;
			}
		}
		break;

	case fieldTypeMFFloat :
		{
			MFFloat *outFloat = (MFFloat *)eventOutField;
			switch (eventInFieldType) {
			case fieldTypeMFFloat :
				{
					MFFloat *inFloat = (MFFloat *)eventInField;
					inFloat->copy(outFloat);
				}
				break;
			}
		}
		break;

	case fieldTypeMFInt32 :
		{
			MFInt32 *outInt32 = (MFInt32 *)eventOutField;
			switch (eventInFieldType) {
			case fieldTypeMFInt32 :
				{
					MFInt32 *inInt32 = (MFInt32 *)eventInField;
					inInt32->copy(outInt32);
				}
				break;
			}
		}
		break;

	case fieldTypeMFRotation :
		{
			MFRotation *outRotation = (MFRotation *)eventOutField;
			switch (eventInFieldType) {
			case fieldTypeMFRotation :
				{
					MFRotation *inRotation = (MFRotation *)eventInField;
					inRotation->copy(outRotation);
				}
				break;
			}
		}
		break;

	case fieldTypeMFTime :
		{
			MFTime *outTime = (MFTime *)eventOutField;
			switch (eventInFieldType) {
			case fieldTypeMFTime :
				{
					MFTime *inTime = (MFTime *)eventInField;
					inTime->copy(outTime);
				}
				break;
			}
		}
		break;

	case fieldTypeMFVec2f :
		{
			MFVec2f *outVec2f = (MFVec2f *)eventOutField;
			switch (eventInFieldType) {
			case fieldTypeMFVec2f :
				{
					MFVec2f *inVec2f = (MFVec2f *)eventInField;
					inVec2f->copy(outVec2f);
				}
				break;
			}
		}
		break;

	case fieldTypeMFVec3f :
		{
			MFVec3f *outVec3f = (MFVec3f *)eventOutField;
			switch (eventInFieldType) {
			case fieldTypeMFVec3f :
				{
					MFVec3f *inVec3f = (MFVec3f *)eventInField;
					inVec3f->copy(outVec3f);
				}
				break;
			}
		}
		break;
	}


	Node *eventInNode = getEventInNode();

	////////////////////////////////////////////////
	//	BindableNode
	////////////////////////////////////////////////

	if (eventInNode->isBindableNode()) {
		if (((BindableNode *)eventInNode)->getBindField() == eventInField) {
			SceneGraph *sceneGraph = eventInNode->getSceneGraph();
			if (sceneGraph)
				sceneGraph->setBindableNode((BindableNode *)eventInNode, ((SFBool *)eventInField)->getValue());			
		}
	}

	////////////////////////////////////////////////
	//	Script
	////////////////////////////////////////////////

	if (eventInNode->isScriptNode())
		((ScriptNode *)eventInNode)->update(eventInField);
}


