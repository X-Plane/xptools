/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Event.cpp
*
******************************************************************/

#include "SceneGraph.h"
#include "UrlFile.h"

Parser::Parser() 
{
	setParserResult(false);
	setParseringState(false);
}

Parser::~Parser() 
{
}

////////////////////////////////////////////////
//	find node
////////////////////////////////////////////////

Node *Parser::findNodeByType(char *typeName) 
{
	if (!typeName)
		return NULL;
	if (strlen(typeName) <= 0)
		return NULL;
	Node *node = getRootNode()->nextTraversalByType(typeName);
	if (node) {
		while (node->isInstanceNode() == true)
			node = node->getReferenceNode();
	}
	return node;
}

Node *Parser::findNodeByName(char *name) 
{
	if (!name)
		return NULL;
	if (strlen(name) <= 0)
		return NULL;
	Node *node = getRootNode()->nextTraversalByName(name);
	if (node) {
		while (node->isInstanceNode() == true)
			node = node->getReferenceNode();
	}
	return node;
}

////////////////////////////////////////////////
//	Parser::getNLines
////////////////////////////////////////////////

int Parser::getNLines(char *fileName) 
{
	FILE *fp;
	if ((fp = fopen(fileName, "rt")) == NULL){
		fprintf(stderr, "Cannot open data file %s\n", fileName);
		return 0;
	}

	char *lineBuffer = (char *)malloc(DEFAULT_LEX_LINE_BUFFER_SIZE + 1);

	int nLine = 0;
	while (fgets(lineBuffer, DEFAULT_LEX_LINE_BUFFER_SIZE, fp))
		nLine++;

	delete lineBuffer;

	fclose(fp);

	return nLine;
}

////////////////////////////////////////////////
//	Parser::load
////////////////////////////////////////////////

void Parser::load(char *fileName, void (*callbackFn)(int nLine, void *info), void *callbackFnInfo) 
{
	FILE *fp = fopen(fileName, "rt");

#ifdef SUPPORT_URL
	SceneGraph *sg = (SceneGraph *)this;
#endif

#ifdef SUPPORT_URL
	if (fp == NULL){
		if (sg->getUrlStream(fileName)) {
			char *outputFilename = sg->getUrlOutputFilename();
			fp = fopen(outputFilename, "rt");
			sg->setUrl(fileName);
		}
	}
#endif

	if (fp == NULL) {
		fprintf(stderr, "Cannot open data file %s\n", fileName);
		setParserResult(false);
		setParseringState(false);
		return;
	}

	fseek(fp, 0, SEEK_END);
	int lexBufferSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (GetParserObject() == NULL)
		MakeLexerBuffers(lexBufferSize, DEFAULT_LEX_LINE_BUFFER_SIZE);

	mParserNodeList.deleteNodes();
	deleteDEFs();
	deletePROTOs();

	PushParserObject(this);

	SetLexCallbackFn(callbackFn, callbackFnInfo);

	setErrorLineNumber(0);
	setErrorLineString("");
	setErrorToken("");

    SetInputFile(fp);

	setParseringState(true);

    setParserResult(!yyparse() ? true : false);

	setParseringState(false);

	PopParserObject();

	if (GetParserObject() == NULL)
		DeleteLexerBuffers();


	fclose(fp);

#ifdef SUPPORT_URL
	sg->deleteUrlOutputFilename();
#endif
}

////////////////////////////////////////////////
//	Parse Action
////////////////////////////////////////////////

void Parser::addNode(Node *node, bool initialize) 
{
	moveNode(node);
	if (initialize)
		node->initialize();
}

void Parser::addNodeAtFirst(Node *node, bool initialize) 
{
	moveNodeAtFirst(node);
	if (initialize)
		node->initialize();
}

void Parser::moveNode(Node *node) 
{
	Node *parentNode = getCurrentNode();
	if (!parentNode || !getParseringState())
		mNodeList.addNode(node);
	else
		parentNode->moveChildNode(node);

	node->setParentNode(parentNode);
	node->setSceneGraph((SceneGraph *)this);
}

void Parser::moveNodeAtFirst(Node *node) 
{
	Node *parentNode = getCurrentNode();
	if (!parentNode || !getParseringState())
		mNodeList.addNodeAtFirst(node);
	else
		parentNode->moveChildNodeAtFirst(node);

	node->setParentNode(parentNode);
	node->setSceneGraph((SceneGraph *)this);
}

void Parser::pushNode(Node *node, int type)
{
	ParserNode *parserNode = new ParserNode(node, type);
	mParserNodeList.addNode(parserNode);
}

void Parser::popNode()
{
	ParserNode *lastNode = mParserNodeList.getLastNode(); 
	delete lastNode;
}

Node *Parser::getCurrentNode() 
{
	ParserNode *lastNode = mParserNodeList.getLastNode(); 
	if (!lastNode)
		return NULL;
	else
		return lastNode->getNode();
}

int Parser::getCurrentNodeType() 
{
	ParserNode *lastNode = mParserNodeList.getLastNode(); 
	if (!lastNode)
		return 0;
	else
		return lastNode->getType();
}

int Parser::getPrevNodeType() 
{
	ParserNode *lastNode = mParserNodeList.getLastNode(); 
	if (!lastNode)
		return 0;
	else {
		ParserNode *prevNode = lastNode->prev(); 
		if (prevNode->isHeaderNode())
			return 0;
		else
			return prevNode->getType();
	}
}

///////////////////////////////////////////////
//	DEF
///////////////////////////////////////////////

DEF *Parser::getDEFs() 
{
	return (DEF *)mDEFList.getNodes();
}

char *Parser::getDEFString(char *name) 
{
	for (DEF *def=getDEFs(); def; def=def->next()) {
		char *defName = def->getName();
		if (defName && !strcmp(defName, name))
			return def->getString();
	}
	return NULL;
}

void Parser::addDEF(DEF *def) 
{
	mDEFList.addNode(def);
}
	
void Parser::addDEF(char *name, char *string) 
{
	DEF *def = new DEF(name, string);
	addDEF(def);
}

void Parser::deleteDEFs() 
{
	mDEFList.deleteNodes();
}

///////////////////////////////////////////////
//	PROTO
///////////////////////////////////////////////

PROTO *Parser::getPROTOs() 
{
	return (PROTO *)mPROTOList.getNodes();
}

PROTO *Parser::getPROTO(char *name) 
{
	if (!name || !strlen(name))
		return NULL;

	for (PROTO *proto=getPROTOs(); proto; proto=proto->next()) {
		char *protoName = proto->getName();
		if (protoName && !strcmp(protoName, name))
			return proto;
	}
	return NULL;
}

void Parser::addPROTO(PROTO *proto) 
{
	mPROTOList.addNode(proto);
}

void Parser::deletePROTOs() 
{
	mPROTOList.deleteNodes();
}

///////////////////////////////////////////////
//	ROUTE
///////////////////////////////////////////////

Route *Parser::getRoutes() 
{
	return (Route *)mRouteList.getNodes();
}

Route *Parser::getRoute(Node *eventOutNode, Field *eventOutField, Node *eventInNode, Field *eventInField)
{
	for (Route *route=getRoutes(); route; route=route->next()) {
		if (eventOutNode == route->getEventOutNode() && eventOutField == route->getEventOutField() &&
			eventInNode == route->getEventInNode() && eventInField == route->getEventInField() ) {
			return route;
		}
	}
	return NULL;
}

void Parser::addRoute(Route *route) 
{
	if (route->getEventOutNode() == route->getEventInNode())
		return;
	if (getRoute(route->getEventOutNode(), route->getEventOutField(), route->getEventInNode(), route->getEventInField()))
		return;
	mRouteList.addNode(route);
}

Route *Parser::addRoute(char *eventOutNodeName, char *eventOutFieldName, char *eventInNodeName, char *eventInFieldName)
{
	Node *eventInNode = findNodeByName(eventInNodeName);
	Node *eventOutNode = findNodeByName(eventOutNodeName);

	Field *eventOutField = NULL;

	if (eventOutNode) {
		eventOutField = eventOutNode->getEventOut(eventOutFieldName);
		if (!eventOutField)
			eventOutField = eventOutNode->getExposedField(eventOutFieldName);
	}

	Field *eventInField = NULL;

	if (eventInNode) {
		eventInField = eventInNode->getEventIn(eventInFieldName);
		if (!eventInField)
			eventInField = eventInNode->getExposedField(eventInFieldName);
	}
	
	if (!eventInNode || !eventOutNode || !eventInField || !eventOutField)
		return NULL;
	Route *route = new Route(eventOutNode, eventOutField, eventInNode, eventInField);
	addRoute(route);
	return route;
}

Route *Parser::addRoute(Node *eventOutNode, Field *eventOutField, Node *eventInNode, Field *eventInField)
{
	Route *route = new Route(eventOutNode, eventOutField, eventInNode, eventInField);
	addRoute(route);
	return route;
}

void Parser::deleteRoute(Node *eventOutNode, Field *eventOutField, Node *eventInNode, Field *eventInField)
{
	Route *route  = getRoute(eventOutNode, eventOutField, eventInNode, eventInField);
	if (route)
		delete route;
}

void Parser::deleteRoutes(Node *node) {
	Route *route = getRoutes();
	while (route) {
		Route *nextRoute = route->next();
		if (node == route->getEventInNode() || node == route->getEventOutNode())
			delete route;
		route = nextRoute;
	}
}

void Parser::deleteEventInFieldRoutes(Node *node, Field *field)
{
	Route	*route = getRoutes();
	while (route) {
		Route *nextRoute = route->next();
		if (route->getEventInNode() == node && route->getEventInField() == field)
			delete route;
		route = nextRoute;
	}
}

void Parser::deleteEventOutFieldRoutes(Node *node, Field *field)
{
	Route	*route = getRoutes();
	while (route) {
		Route *nextRoute = route->next();
		if (route->getEventOutNode() == node && route->getEventOutField() == field)
			delete route;
		route = nextRoute;
	}
}

void Parser::deleteRoutes(Node *node, Field *field)
{
	deleteEventInFieldRoutes(node, field);
	deleteEventOutFieldRoutes(node, field);
}

void Parser::deleteRoute(Route *deleteRoute)
{
	for (Route *route=getRoutes(); route; route=route->next()) {
		if (deleteRoute == route) {
			delete route;
			return;
		}
	}
}

void Parser::removeRoute(Node *eventOutNode, Field *eventOutField, Node *eventInNode, Field *eventInField)
{
	Route *route  = getRoute(eventOutNode, eventOutField, eventInNode, eventInField);
	if (route)
		route->remove();
}

void Parser::removeRoutes(Node *node) 
{
	Route *route = getRoutes();
	while (route) {
		Route *nextRoute = route->next();
		if (node == route->getEventInNode() || node == route->getEventOutNode())
			route->remove();
		route = nextRoute;
	}
}

void Parser::removeEventInFieldRoutes(Node *node, Field *field)
{
	Route	*route = getRoutes();
	while (route) {
		Route *nextRoute = route->next();
		if (route->getEventInNode() == node && route->getEventInField() == field)
			route->remove();
		route = nextRoute;
	}
}

void Parser::removeEventOutFieldRoutes(Node *node, Field *field)
{
	Route	*route = getRoutes();
	while (route) {
		Route *nextRoute = route->next();
		if (route->getEventOutNode() == node && route->getEventOutField() == field)
			route->remove();
		route = nextRoute;
	}
}

void Parser::removeRoutes(Node *node, Field *field)
{
	removeEventInFieldRoutes(node, field);
	removeEventOutFieldRoutes(node, field);
}

void Parser::removeRoute(Route *removeRoute)
{
	for (Route *route=getRoutes(); route; route=route->next()) {
		if (removeRoute == route) {
			route->remove();
			return;
		}
	}
}
