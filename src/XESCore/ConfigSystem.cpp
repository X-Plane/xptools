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
#include "ConfigSystem.h"
#include "MemFileUtils.h"
#include "CompGeomDefs2.h"
#include "EnumSystem.h"
#include <stdarg.h>
#include <list>
#include "AssertUtils.h"
using std::list;

#include "PlatformUtils.h"
#if APL && !defined(__MACH__)
#define __DEBUGGING__
#include "XUtils.h"
#endif

typedef pair<ProcessConfigString_f, void *>			HandlerEntry;
typedef hash_map<string, HandlerEntry>				HandlerMap;
static HandlerMap									sHandlerTable;
static set<string>									sLoadedFiles;

static list<string>									sPathStack;

#if 0
void	TokenizeOneLine(const char * begin, const char * end, vector<string>& outTokens)
{
	while (begin < end)
	{
		// First skip any white space leading this token.
		while (begin < end && (*begin == ' ' ||
							   *begin == '\t'))
			++begin;
		// If the token starts with a # or we hit the newline, we're done, bail.
		if (*begin == '#' ||
			*begin == '\r' ||
			*begin == '\n') return;

		// Mark the token start.
		const char * tokenStart = begin;
		// Scan to the next white space or # symbol.  This is the whole token.
		while (begin < end && (*begin != ' '  &&
							  *begin != '\t' &&
							  *begin != '\r' &&
							  *begin != '\n' &&
							  *begin != '#'))
			++begin;
		outTokens.push_back(string(tokenStart, begin));
	}
}
#endif

bool TokenizeFunc(const char * s, const char * e, void * ref)
{
	vector<string> * v = (vector<string> *) ref;
	v->push_back(string(s,e));
	return true;
}

static bool HandleInclude(const vector<string>& args, void * ref)
{
	if (args.size() < 2) return false;

	Assert(!sPathStack.empty());
	string full = sPathStack.back() + args[1];
	return LoadConfigFileFullPath(full.c_str());
}


bool	RegisterLineHandler(
					const char * 			inToken,
					ProcessConfigString_f 	inHandler,
					void * 					inRef)
{
	if (sHandlerTable.empty())
	{
		sHandlerTable.insert(HandlerMap::value_type("INCLUDE", HandlerEntry(HandleInclude, NULL)));
	}
	string	token(inToken);
	if (sHandlerTable.find(inToken) != sHandlerTable.end())
		return false;
	sHandlerTable.insert(HandlerMap::value_type(token, HandlerEntry(inHandler, inRef)));
	return true;
}

string	FindConfigFile(const char * inFilename)
{
	string	partial_path = string("config" DIR_STR) + inFilename;
#if APL && !defined(__MACH__)
	string	appP;
	AppPath(appP);
	string::size_type b = appP.rfind(':');
	appP.erase(b+1);
	partial_path = appP + partial_path;
	if (!strncmp(inFilename, appP.c_str(), appP.size()))
		partial_path = inFilename;
#endif
	return partial_path;
}

bool	LoadConfigFile(const char * inFilename)
{
	string partial_path = FindConfigFile(inFilename);
	return LoadConfigFileFullPath(partial_path.c_str());
}


bool	LoadConfigFileFullPath(const char * inFilename)
{
	MFMemFile *	f;
	bool ok = false;
	f = MemFile_Open(inFilename);
	if (!f) {
		printf("Unable to load config file %s\n", inFilename);
		return ok;
	}

	string	dir(inFilename);
	dir.erase(dir.find_last_of("\\/:")+1);
	sPathStack.push_back(dir);

	MFTextScanner * scanner = TextScanner_Open(f);
	if (scanner)
	{
		while (!TextScanner_IsDone(scanner))
		{
			vector<string>	tokens;
			TextScanner_TokenizeLine(scanner, " \t", "\r\n#\"", -1, TokenizeFunc, &tokens);
			if (!tokens.empty())
			{
				HandlerMap::iterator h = sHandlerTable.find(tokens[0]);
				if (h == sHandlerTable.end())
				{
					string	line(TextScanner_GetBegin(scanner), TextScanner_GetEnd(scanner));
					printf("Unable to parse line: %s\n", line.c_str());
					goto bail;
				}
				if (!h->second.first(tokens,h->second.second))
				{
					string	line(TextScanner_GetBegin(scanner), TextScanner_GetEnd(scanner));
					printf("Parse error in file %s line: %s\n", inFilename, line.c_str());
					goto bail;
				}
			}
			TextScanner_Next(scanner);
		}
		ok = true;
bail:
		TextScanner_Close(scanner);
	}
	MemFile_Close(f);
	sPathStack.pop_back();
	return ok;
}

// Same as above, except the config file is only loaded the first time
// this is called.  This is useful for lazy on-demand loading of prefs files.
bool	LoadConfigFileOnce(const char * inFilename)
{
	string	fname(inFilename);
	if (sLoadedFiles.find(fname) != sLoadedFiles.end())
		return true;

	bool	ok = LoadConfigFile(inFilename);
	if (ok)
		sLoadedFiles.insert(fname);
	return ok;
}

int					TokenizeInt(const string& s)
{
	return atoi(s.c_str());
}

float				TokenizeFloat(const string& s)
{
	float val = atof(s.c_str());
	if (s[s.length()-1] == '%') return val / 100.0;
	else return val;
}

inline  bool is_hex(char c)
{
	return ((c >= '0' && c <= '9') ||
			(c >= 'A' && c <= 'F') ||
			(c >= 'a' && c <= 'f'));
}
inline	int	hex_digit(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c + 10 - 'A';
	if (c >= 'a' && c <= 'f') return c + 10 - 'a';
	return 0;
}

bool				TokenizeColor(const string& s, RGBColor_t& c)
{
	if (s.find(',') != s.npos)
	{
		sscanf(s.c_str(), "%f,%f,%f",&c.rgb[0],&c.rgb[1], &c.rgb[2]);
		c.rgb[0] = c.rgb[0] / 255.0;
		c.rgb[1] = c.rgb[1] / 255.0;
		c.rgb[2] = c.rgb[2] / 255.0;
		return true;
	}
	unsigned short r, g, b;
	if (s.length() != 6) return false;
	if (!is_hex(s[0])) return false;
	if (!is_hex(s[1])) return false;
	if (!is_hex(s[2])) return false;
	if (!is_hex(s[3])) return false;
	if (!is_hex(s[4])) return false;
	if (!is_hex(s[5])) return false;
	r = 16 * hex_digit(s[0]) + hex_digit(s[1]);
	g = 16 * hex_digit(s[2]) + hex_digit(s[3]);
	b = 16 * hex_digit(s[4]) + hex_digit(s[5]);
	c.rgb[0] = ((float) r) / 255.0;
	c.rgb[1] = ((float) g) / 255.0;
	c.rgb[2] = ((float) b) / 255.0;
	return true;
}

bool				TokenizeEnum(const string& token, int& slot, const char * errMsg)
{
	slot = LookupToken(token.c_str());
	if (slot == -1)
		slot = NewToken(token.c_str());
//	if (slot == -1)
//		printf(errMsg, token.c_str());
	return (slot != -1);
}

bool				TokenizeEnumSet(const string& tokens, set<int>& slots)
{
	string::size_type e,s=0;
	slots.clear();
	while(1)
	{
		e=tokens.find(',', s);

		string subst;
		if(e==tokens.npos)
			subst=tokens.substr(s);
		else
			subst=tokens.substr(s,e-s);
		
		int bade;
		slots.insert(bade = LookupToken(subst.c_str()));
		if (bade == -1)
			printf("WARNING: could not find token: '%s'\n", subst.c_str());
		if(e==tokens.npos)
			break;
		else
			s=e+1;
	}
	slots.erase(NO_VALUE);
	return slots.count(-1) == 0;
}


// Format is:
// i - int
// f - float
// c - color
// e - enum
// s - STL string
// t = char **
// S - enum set
// P - Point2, splatted
//   - skip
int				TokenizeLine(const vector<string>& tokens, const char * fmt, ...)
{
	va_list	args;
	va_start(args, fmt);
	int n = 0;
	int * 			ip;
	float *			fp;
	RGBColor_t *	cp;
	string *		sp;
	set<int>*		es;
	char **			tp;
	Point2 *		pp;
	while (fmt[n] && n < tokens.size())
	{
		switch(fmt[n]) {
		case 'i':
			ip = va_arg(args, int *);
			*ip = TokenizeInt(tokens[n]);
			break;
		case 'f':
			fp = va_arg(args, float *);
			*fp = TokenizeFloat(tokens[n]);
			break;
		case 'c':
			cp = va_arg(args, RGBColor_t *);
			if (!TokenizeColor(tokens[n], *cp))
				goto bail;
			break;
		case 'e':
			ip = va_arg(args, int *);
			if (!TokenizeEnum(tokens[n], *ip, "Bad enum token '%s'"))
				goto bail;
			break;
		case 's':
			sp = va_arg(args, string *);
			*sp = tokens[n];
			break;
		case 'S':
			es = va_arg(args, set<int> *);
			if(!TokenizeEnumSet(tokens[n], *es))
				goto bail;
			break;
		case 't':
			tp = va_arg(args, char **);
			if (tokens[n] == "-")
				*tp = NULL;
			else {
				*tp = (char *) malloc(tokens[n].size() + 1);
				strcpy(*tp, tokens[n].c_str());
			}
			break;
		case 'P':
			pp = va_arg(args,Point2 *);
			if(sscanf(tokens[n].c_str(),"%lf,%lf",&pp->x_, &pp->y_) != 2)			
				pp->x_ = pp->y_ = TokenizeFloat(tokens[n]);
			break;
		case ' ':
			break;
		default:
			goto bail;
		}
		++n;
	}
bail:
	va_end(args);
	return n;
}


void	DebugPrintTokens(const vector<string>& tokens)
{
	for(int n = 0; n < tokens.size(); ++n)
	{
		if(n) printf(" ");
		printf("%s",tokens[n].c_str());
	}
	printf("\n");
}
