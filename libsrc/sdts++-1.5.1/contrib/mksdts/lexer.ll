%{

#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

#include "parser.h"

extern int    lineno;		// current line number
extern string yylval_string;	// current string value

%}


%x TYPE
%x QUOTED_STRING


%%

0x[[:xdigit:]]+		{ yylval.d_val = strtol(yytext, (char **)NULL, 16);
                          return BINARY; }

(-)?[0-9]+		{ yylval.i_val = atoi(yytext); 
                          return INT;  /* a field label */
                        }

(-)?[0-9]+"."[0-9]+([Ee][+-]?[0-9]+)?	{ yylval.d_val = atof(yytext); return FLOAT; }

[\"]                    { BEGIN QUOTED_STRING; yylval_string = ""; }

<QUOTED_STRING>[^\"]*  { yylval_string = yytext; }

<QUOTED_STRING>[\"]     { BEGIN INITIAL; return STRING; }

field			{ return FIELD; }

module			{ return MODULE; }

record			{ return RECORD; }

just_data		{ return DROP; }

[[:alpha:]]{1,5}        { yylval_string = yytext; return ID; }

"{"			{ return '{'; }

"}"			{ return '}'; }

"["			{ return '['; }

"]"			{ return ']'; }

","			{ return ','; }

":"			{ BEGIN TYPE; return ':'; }

<TYPE>A			{ BEGIN INITIAL; return 'A'; }

<TYPE>I			{ BEGIN INITIAL; return 'I'; }

<TYPE>R			{ BEGIN INITIAL; return 'R'; }

<TYPE>S			{ BEGIN INITIAL; return 'S'; }

<TYPE>BI8		{ BEGIN INITIAL; return BI8; }

<TYPE>BI16		{ BEGIN INITIAL; return BI16; }

<TYPE>BI24		{ BEGIN INITIAL; return BI24; }

<TYPE>BI32		{ BEGIN INITIAL; return BI32; }

<TYPE>BUI8		{ BEGIN INITIAL; return BUI8; }

<TYPE>BUI16		{ BEGIN INITIAL; return BUI16; }

<TYPE>BUI24		{ BEGIN INITIAL; return BUI24; }

<TYPE>BUI32		{ BEGIN INITIAL; return BUI32; }

<TYPE>BFP32		{ BEGIN INITIAL; return BFP32; }

<TYPE>BFP64		{ BEGIN INITIAL; return BFP64; }

<TYPE>[[:alnum:]\(\)\*\+\$\^\{\}\[\]]+	{ BEGIN INITIAL;
                                          yylval_string = yytext; 
                                          return REGEXP; }

<INITIAL,TYPE>#.*	{ /* eat comment*/ }

<INITIAL,TYPE>[ \t]*	{ /* eat whitespace */ }

<INITIAL,TYPE,QUOTED_STRING>[\n]      { lineno++; }

.			{ cerr << "unknown char " << yytext[0] << "\n"; }


%%

int lineno;

static const char* _ident = "$Id: lexer.ll,v 1.3 2003/02/13 23:37:45 mcoletti Exp $";
