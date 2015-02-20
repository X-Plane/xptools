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
#include "EnumSystem.h"
#include "XChunkyFileUtils.h"
#include "DEMDefs.h"	// For NO_DATA

#define	TOKEN(x)		#x,
#define EXTRACT_TOKENS 1

	const char * kDefaultTokens[] = {

#include "ParamDefs.h"

	NULL
};

#undef TOKEN
#undef EXTRACT_TOKENS


TokenMap		gTokens;
TokenReverseMap	gReverse;

static bool ConfirmNotNumber(const char * t)
{
	if(*t == 0) 
		return false;
	while(*t)
	{
		if(!isdigit(*t))
			return true;
		++t;
	}
	return false;
}

void InitEnumSystem()
{
	gTokens.clear();
	for (int i = 0; i < NUMBER_OF_DEFAULT_TOKENS; ++i)
		gTokens.push_back(kDefaultTokens[i]);
	BuildTokenReverseMap(gTokens, gReverse);
}

const char *	FetchTokenString(int x)
{
	if (x == DEM_NO_DATA) return "NO DATA";
	if (x < 0 || x >= gTokens.size()) return "unknown token";
	return gTokens[x].c_str();
}

int				LookupToken(const char * inString)
{
	if (gTokens.size() != gReverse.size())
		BuildTokenReverseMap(gTokens, gReverse);
	TokenReverseMap::iterator i = gReverse.find(inString);
	if (i == gReverse.end()) return -1;
	return i->second;
}

int				LookupTokenCreate(const char * inString)
{
	int n = LookupToken(inString);
	if (n != -1) return n;
	return NewToken(inString);
}

void	BuildTokenReverseMap(
						const TokenMap&		inTokens,
						TokenReverseMap&	inReverse)
{
	inReverse.clear();
	for (int i = 0; i < inTokens.size(); ++i)
	{
		inReverse.insert(TokenReverseMap::value_type(inTokens[i], i));
	}
}

void	BuildTokenConversionMap(
						TokenMap&			ioDestination,
						const TokenMap&		inSource,
						TokenConversionMap&	outConversion)
{
	TokenReverseMap	knownTokens;
	BuildTokenReverseMap(ioDestination, knownTokens);
	outConversion.clear();
	for (int src = 0; src < inSource.size(); ++src)
	{
		TokenReverseMap::iterator knownIter = knownTokens.find(inSource[src]);
		if (knownIter != knownTokens.end())
		{
			outConversion.push_back(knownIter->second);
		} else {
			outConversion.push_back(ioDestination.size());
			ioDestination.push_back(inSource[src]);
		}
	}
}

void	WriteEnumsAtomToFile(FILE * inFile, const TokenMap& inTokens, int atomCode)
{
	StAtomWriter	tertAtom(inFile, atomCode);
	for (TokenMap::const_iterator i = inTokens.begin(); i != inTokens.end(); ++i)
			fwrite(i->c_str(), i->size() + 1, 1, inFile);
}


void	ReadEnumsAtomFromFile(XAtomContainer& inAtomContainer, TokenMap& outTokens, int atomCode)
{
	XAtomStringTable	strings;
	if (inAtomContainer.GetNthAtomOfID(atomCode, 0, strings))
	{
		outTokens.clear();
		const char * i;
		for (i = strings.GetFirstString(); i; i = strings.GetNextString(i))
		{
			outTokens.push_back(string(i));
		}
	}
}

int				NewToken(const char * inString)
{
	Assert(ConfirmNotNumber(inString));
	int v = gTokens.size();
	gTokens.push_back(inString);
	gReverse[inString] = gTokens.size()-1;

	return v;
}

void EnumSystemSelfCheck(void)
{
	set<string>	dummy;
	for (int n = 0; n < gTokens.size(); ++n)
	{
		Assert(dummy.count(gTokens[n]) == 0);
		dummy.insert(gTokens[n]);
	}
}

