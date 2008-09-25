/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Route.h
*
******************************************************************/

#ifndef _CV97_ROUTE_H_
#define _CV97_ROUTE_H_

#include <iostream.h>
#include "LinkedList.h"
#include "VRMLField.h"
#include "VRMLNodes.h"
#include "JavaVM.h"

#ifdef SUPPORT_JSAI
class Route : public LinkedListNode<Route>, public CJavaVM {
#else
class Route : public LinkedListNode<Route> {
#endif

	Node	*mEventOutNode;
	Node	*mEventInNode;
	Field	*mEventOutField;
	Field	*mEventInField;

	bool	mIsActive;
	void	*mValue;

public:

	Route(Node *eventOutNode, Field *eventOutField, Node *eventInNode, Field *eventInField);
	Route(Route *route);
	~Route();

	void	setEventOutNode(Node *node);
	void	setEventInNode(Node *node);
	Node	*getEventOutNode();
	Node	*getEventInNode();
	void	setEventOutField(Field *field);
	Field	*getEventOutField();
	void	setEventInField(Field *field);
	Field	*getEventInField();

	////////////////////////////////////////////////
	//	Active
	////////////////////////////////////////////////

	void setIsActive(bool active);
	bool	isActive();

	////////////////////////////////////////////////
	//	update
	////////////////////////////////////////////////

	void initialize();
	void update();

	////////////////////////////////////////////////
	//	update
	////////////////////////////////////////////////

	void setValue(void *value);
	void *getValue();

	////////////////////////////////////////////////
	//	output
	////////////////////////////////////////////////

	void output(ostream& printStream);
};

#endif
