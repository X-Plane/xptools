/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Parser.h
*
******************************************************************/

#ifndef _CV97_PARSER_H_
#define _CV97_PARSER_H_

#include <assert.h>
#include "VRMLField.h"
#include "VRMLNodes.h"
#include "VRMLParser.h"
#include "NodeList.h"
#include "Route.h"
#include "ParserNode.h"
#include "String.h"
#include "DEF.h"
#include "Proto.h"

#define DEFAULT_LEX_LINE_BUFFER_SIZE	(64*1024)

class	SceneGraph;

class Parser : public LinkedListNode<Parser>{

	NodeList				mNodeList;
	LinkedList<Route>		mRouteList;
	LinkedList<ParserNode>	mParserNodeList;
	LinkedList<DEF>		mDEFList;
	LinkedList<PROTO>		mPROTOList;
	String					mDefName;

	int						mErrorLineNumber;
	String					mErrorToken;
	String					mErrorLineString;
	bool					mIsOK;
	bool					mbParsering;

public:

	Parser();
	~Parser();

	Node *getRootNode() {
		return (Node *)mNodeList.getRootNode();		
	}

	Node *getNodes() {
		return (Node *)mNodeList.getNodes();		
	}

	///////////////////////////////////////////////
	//	Load
	///////////////////////////////////////////////

	void clearNodeList() {
		mNodeList.deleteNodes();		
	}

	void clearRouteList() {
		mRouteList.deleteNodes();		
	}

	////////////////////////////////////////////////
	//	find node
	////////////////////////////////////////////////

	Node *findNodeByType(char *typeName);

	Node *findNodeByName(char *name);

	///////////////////////////////////////////////
	//	Praser action
	///////////////////////////////////////////////

	void addNode(Node *node, bool initialize = true);
	void addNodeAtFirst(Node *node, bool initialize = true);

	void moveNode(Node *node);
	void moveNodeAtFirst(Node *node);

	void pushNode(Node *node, int type);
	void popNode();
	Node *getCurrentNode();
	int getCurrentNodeType();
	int getPrevNodeType();

	///////////////////////////////////////////////
	//	DEF
	///////////////////////////////////////////////

	void setDefName(char *name) {
		mDefName.setValue(name);
	}

	char *getDefName() {
		return mDefName.getValue();
	}

	///////////////////////////////////////////////
	//	for lex & yacc
	///////////////////////////////////////////////

	void setParserResult(bool bOK) { 
		mIsOK = bOK; 
	}
	bool isOK(void) {
		return mIsOK; 
	}

	void setErrorLineNumber(int n) { 
		mErrorLineNumber = n; 
	}
	int	getErrorLineNumber(void){
		return mErrorLineNumber; 
	}

	void setErrorToken(char *error) {
		mErrorToken.setValue(error); 
	}
	char *getErrorToken(void) { 
		return mErrorToken.getValue(); 
	}

	void setErrorLineString(char *error) { 
		mErrorLineString.setValue(error); 
	}
	char *getErrorLineString(void) {
		return mErrorLineString.getValue(); 
	}

	///////////////////////////////////////////////
	//	Load
	///////////////////////////////////////////////

	void	setParseringState(bool state)	{ mbParsering = state; }
	bool	getParseringState()				{ return mbParsering; }

	int		getNLines(char *fileName);
	void	load(char *fileName, void (*callbackFn)(int nLine, void *info) = NULL, void *callbackFnInfo = NULL);

	///////////////////////////////////////////////
	//	DEF
	///////////////////////////////////////////////

	DEF *getDEFs();
	char *getDEFString(char *name);
	void addDEF(DEF *def);
	void addDEF(char *name, char *string);
	void deleteDEFs();

	///////////////////////////////////////////////
	//	PROTO
	///////////////////////////////////////////////

	PROTO *getPROTOs();
	PROTO *getPROTO(char *name);
	void addPROTO(PROTO *proto);
	void deletePROTOs();
	
	///////////////////////////////////////////////
	//	ROUTE
	///////////////////////////////////////////////

	Route *getRoutes();
	Route *getRoute(Node *eventOutNode, Field *eventOutField, Node *eventInNode, Field *eventInField);
	void addRoute(Route *route);
	Route *addRoute(char *eventOutNodeName, char *eventOutFieldName, char *eventInNodeName, char *eventInFieldName);
	Route *addRoute(Node *eventOutNode, Field *eventOutField, Node *eventInNode, Field *eventInField);
	void deleteRoute(Node *eventOutNode, Field *eventOutField, Node *eventInNode, Field *eventInField);
	void deleteRoutes(Node *node);
	void deleteEventInFieldRoutes(Node *node, Field *field);
	void deleteEventOutFieldRoutes(Node *node, Field *field);
	void deleteRoutes(Node *node, Field *field);
	void deleteRoute(Route *deleteRoute);
	void removeRoute(Node *eventOutNode, Field *eventOutField, Node *eventInNode, Field *eventInField);
	void removeRoutes(Node *node);
	void removeEventInFieldRoutes(Node *node, Field *field);
	void removeEventOutFieldRoutes(Node *node, Field *field);
	void removeRoutes(Node *node, Field *field);
	void removeRoute(Route *removeRoute);

};

#endif


