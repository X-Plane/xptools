/* 
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */
#ifndef ENUMSYSTEM_H
#define ENUMSYSTEM_H

struct	XAtomContainer;

/* The Token macro defines tokens.  First, publicly declare all of our token
 * enums.   They will be assigned from 0 in the order of the file, so we acn
 * do enum range checks, etc. */
 
#define	TOKEN(x)		x,
#define EXTRACT_TOKENS 1

enum {

#include "ParamDefs.h"

	NUMBER_OF_DEFAULT_TOKENS
};

#undef TOKEN
#undef EXTRACT_TOKENS


/* A token map goes from the int to the string - use a vector because 
 * all enums should be packed. */
 
typedef	vector<string>			TokenMap;
typedef hash_map<string, int>	TokenReverseMap;
typedef	vector<int>				TokenConversionMap;

extern	TokenMap		gTokens;

const char *	FetchTokenString(int);
int				NewToken(const char * inString);
int				LookupToken(const char * inString);
int				LookupTokenCreate(const char * inString);

void	InitEnumSystem(void);

void	BuildTokenReverseMap(
						const TokenMap&		inTokens,
						TokenReverseMap&	inReverse);

// This routine builds a conversion map.  The destination
// is not const because tokens may have to be added!
void	BuildTokenConversionMap(
						TokenMap&			ioDestination,
						const TokenMap&		inSource,
						TokenConversionMap&	outConversion);

// These routines read or write a token map to a file.  Use this to store the current
// tokens.
void	WriteEnumsAtomToFile(FILE * inFile, const TokenMap& inTokens, int atomID);
void	ReadEnumsAtomFromFile(XAtomContainer& inAtomContainer, TokenMap& outTokens, int atomID);

void	EnumSystemSelfCheck(void);

#endif /* ENUMSYSTEM_H */
