/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	VRMLParser.h
*
******************************************************************/

#ifndef _CV97_VRMLPARSER_H_
#define _CV97_VRMLPARSER_H_

#include <stdio.h>
#include "SceneGraph.h"

/******************************************************************
*
*	For lex action
*
******************************************************************/

class	PROTO;

PROTO	*AddPROTOInfo(char *name, char *string, char *fieldString);
PROTO	*IsPROTOName(char *name);

void	AddDEFInfo(char *name, char *string);
char	*GetDEFSrting(char *name);

void	SetDEFName(char *name);
char	*GetDEFName(void);

void	MakeLexerBuffers(int lexBufferSize, int lineBufferSize);
void	DeleteLexerBuffers(void);

void	SetLexCallbackFn(void (*func)(int nLine, void *info), void *fnInfo);

/******************************************************************
*
*	For yacc action
*
******************************************************************/

class Parser;
class Node;

void	PushParserObject(Parser *parser);
void	PopParserObject();
Parser	*GetParserObject();
/*
void	SetParserObject(Parser *parser);
Parser	*GetParserObject(void);
*/

int		GetCurrentNodeType(void);
int		GetPrevNodeType(void);
Node	*GetCurrentNodeObject(void);

void	PushNode(int parserType, Node *node);
void	PopNode(void);

void	AddNode(Node *node);

int		yyparse();

void	AddRouteInfo(char *string);

/******************************************************************
*
*	For yacc action
*
******************************************************************/

int		GetCurrentLineNumber(void);
void	SetInputFile(FILE *fp);
char	*GetErrorLineString(void);

#endif