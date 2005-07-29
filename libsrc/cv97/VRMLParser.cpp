/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	VRMLParser.cpp
*
******************************************************************/

#include <string.h>
#include "SceneGraph.h"

/******************************************************************
*
*	lex action
*
******************************************************************/

/******************************************************************
*	DEF action
******************************************************************/

void AddDEFInfo(
char *name,
char *string)
{
	((Parser *)GetParserObject())->addDEF(name, string);
}

char *GetDEFSrting(
char *name)
{
	return ((Parser *)GetParserObject())->getDEFString(name);
}

/******************************************************************
*	PROTO action
******************************************************************/

PROTO *AddPROTOInfo(
char *name,
char *string,
char *fieldString)
{
	PROTO *proto = new PROTO(name, string, fieldString);
	((Parser *)GetParserObject())->addPROTO(proto);
	return proto;
}

PROTO *IsPROTOName(
char *name)
{
	return ((Parser *)GetParserObject())->getPROTO(name);
}

/******************************************************************
*
*	Node Name
*
******************************************************************/

#define	MAX_DEFNAME	512

static char	gDEFName[MAX_DEFNAME];

void SetDEFName(char *name)
{
	((Parser *)GetParserObject())->setDefName(name);
}

char *GetDEFName(void)
{
	char *defName = ((Parser *)GetParserObject())->getDefName();
	if (defName)
		strcpy(gDEFName, defName);
	SetDEFName(NULL);
	if (defName)
		return gDEFName;
	else
		return NULL;
}

/******************************************************************
*
*	AddRouteInfo
*
******************************************************************/

#define ROUTE_STRING_MAX	2048

static char targetNodeName[ROUTE_STRING_MAX];
static char sourceNodeName[ROUTE_STRING_MAX];
static char targetNodeTypeName[ROUTE_STRING_MAX];
static char sourceNodeTypeName[ROUTE_STRING_MAX];

void AddRouteInfo(char *string)
{

	if (!string || !strlen(string))
		return;

	for (int n=0; n<(int)strlen(string); n++) {
		if (string[n] == '.')
			string[n] = ' ';
	}

	sscanf(string, "%s %s TO %s %s", sourceNodeName, sourceNodeTypeName, targetNodeName, targetNodeTypeName);

	((Parser *)GetParserObject())->addRoute(sourceNodeName, sourceNodeTypeName, targetNodeName, targetNodeTypeName);
}

/******************************************************************
*
*	New for yacc action
*
******************************************************************/

static LinkedList<Parser> mParserList;

void PushParserObject(Parser *parser)
{
	mParserList.addNode(parser);
}

void PopParserObject()
{
	Parser *lastNode = mParserList.getLastNode(); 
	lastNode->remove();
}

Parser *GetParserObject()
{
	return mParserList.getLastNode(); 
}

/*

static Parser *gParserObject = NULL;

void SetParserObject(Parser *parser)
{
	gParserObject = parser; 
}

Parser *GetParserObject(void)
{
	return gParserObject; 
}
*/

int GetCurrentNodeType(void)
{
	return ((Parser *)GetParserObject())->getCurrentNodeType();
}

int GetPrevNodeType(void)
{
	return ((Parser *)GetParserObject())->getPrevNodeType();
}

Node *GetCurrentNodeObject(void)
{
	return ((Parser *)GetParserObject())->getCurrentNode();
}

void PushNode(int parserType, Node *node)
{
	((Parser *)GetParserObject())->pushNode(node, parserType);
}

void PopNode(void)
{
	((Parser *)GetParserObject())->popNode();
}

void AddNode(Node *node)
{
	((Parser *)GetParserObject())->addNode(node, 0);
}
