/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	RouteList.h
*
******************************************************************/

#ifndef _CV97_ROUTELIST_H_
#define _CV97_ROUTELIST_H_

#include "LinkedList.h"
#include "Route.h"

class RouteList : public LinkedList<Route> {

public:

	RouteList();
	~RouteList();

	void addRoute(Route *route);
	Route *getRoutes();
	Route *getRoute(int n);
	int getNRoutes();
};

#endif
