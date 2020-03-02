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
//#include "hl_types.h"
#include "XUtils.h"
#include <stdio.h>
#include <ctype.h>
#include "XObjDefs.h"
#include <stdlib.h>
#include "PlatformUtils.h"
//#include <time.h>

#if APL || IBM
//using namespace Metrowerks;
#endif

static char * my_fgets(char * s, int n, FILE * file)
{
	char *	p = s;
	int			c;

	if (--n < 0)
		return(NULL);

	if (n)
		do
		{
			c = fgetc(file);

			if (c == EOF)
			{
				if (/*feof(file) &&*/ p != s)
					break;
				else
				{
					return(NULL);
				}
			}
			*p++ = c;
		}
		while (c != '\n' && c != '\r' && --n);

	*p = 0;

	return(s);
}



StTextFileScanner::StTextFileScanner(const char * file, bool skip) :
	mFile(fopen(file, "r")),
	mDone(false),
	mSkipBlanks(skip)
{
	read_next();
}

StTextFileScanner::~StTextFileScanner()
{
	if (mFile)
		fclose(mFile);
}

bool	StTextFileScanner::done()
{
	return mDone;
}

void	StTextFileScanner::next()
{
	read_next();
}

string	StTextFileScanner::get()
{
	return mBuf;
}

void	StTextFileScanner::read_next(void)
{
	mBuf.clear();
	mDone = true;

	char	buf[4096];

	while (mFile && /*!feof(mFile) &&*/ my_fgets(buf, sizeof(buf), mFile))
	{
		int len = strlen(buf);
		while ((len > 0) && (buf[len-1] == '\r' || buf[len-1] == '\n'))
		{
			buf[len-1] = 0;
			--len;
		}

		if (buf[0] == '\r' || buf[0] == '\n')
			mBuf = buf+1;
		else
			mBuf = buf;

		if (!mBuf.empty() || !mSkipBlanks)
		{
			mDone = false;
			return;
		}
	}
}

void	BreakString(const string& line, vector<string>& words)
{
	words.clear();
	string::const_iterator i = line.begin();
	while (i < line.end())
	{
		string::const_iterator s = i;
		while (s < line.end() && isspace(*s))
			++s;

		string::const_iterator e = s;
		while (e < line.end() && !isspace(*e))
			++e;

		if (s < e)
			words.push_back(string(s,e));

		i = e;
	}
}

void	StringToUpper(string& s)
{
	for (string::iterator i = s.begin(); i != s.end(); ++i)
		*i = toupper(*i);
}

bool	HasExtNoCase(const string& inStr, const char * inExt)
{
	string s(inStr);
	string e(inExt);
	StringToUpper(s);
	StringToUpper(e);


	if (s.rfind(e) == (s.length() - e.length()))
		return true;
	return false;
}


bool	GetNextNoComments(StTextFileScanner& f, string& s)
{
	while(!f.done())
	{
		s = f.get();
		f.next();
		if (s.empty() || s[0] != '#')
			return true;
	}
	return false;
}


int		PickRandom(vector<double>& chances)
{
	double	v = (double) (rand() % RAND_MAX) / (double) RAND_MAX;

	for (int n = 0; n < chances.size(); ++n)
	{
		if (v < chances[n])
			return n;
		v -= chances[n];
	}
	return chances.size();
}

bool	RollDice(double inProb)
{
	if (inProb <= 0.0) return false;
	if (inProb >= 1.0) return true;
	double	v = (double) (rand() % RAND_MAX) / (double) RAND_MAX;
	return v < inProb;
}

double	RandRange(double mmin, double mmax)
{
	if (mmin >= mmax)
		return mmin;
	double	v = (double) rand() / (double) RAND_MAX;
	return mmin + ((mmax - mmin) * v);
}

double	RandRangeBias(double mmin, double mmax, double biasRatio, double randomAmount)
{
	double	span = mmax - mmin;
	double lower_rat = biasRatio * (1.0 - randomAmount);
	double upper_rat = lower_rat + randomAmount;
	return RandRange(mmin + span * lower_rat,mmin + span * upper_rat);
}


void		StripPath(string& ioPath)
{
	string::size_type sep = ioPath.rfind(DIR_CHAR);
	if (sep != ioPath.npos)
		ioPath = ioPath.substr(sep+1,ioPath.npos);
}

void		StripPathCP(string& ioPath)
{
	string::size_type sep;
	sep = ioPath.rfind(':');
	if (sep != ioPath.npos)
		ioPath = ioPath.substr(sep+1,ioPath.npos);
	sep = ioPath.rfind('\\');
	if (sep != ioPath.npos)
		ioPath = ioPath.substr(sep+1,ioPath.npos);
	sep = ioPath.rfind('/');
	if (sep != ioPath.npos)
		ioPath = ioPath.substr(sep+1,ioPath.npos);
}

void		ExtractPath(string& ioPath)
{
	string::size_type sep = ioPath.rfind(DIR_CHAR);
	if (sep != ioPath.npos)
		ioPath = ioPath.substr(0, sep);
}


void	ExtractFixedRecordString(
				const string&		inLine,
				int					inBegin,
				int					inEnd,
				string&				outString)
{
	int	sp = inBegin-1;
	int ep = inEnd;
	if (ep > inLine.length()) ep = inLine.length();
	if (sp > inLine.length()) sp = inLine.length();

	while ((sp < ep) && (inLine[sp] == ' '))
		++sp;

	while ((ep > sp) && (inLine[ep-1] == ' '))
		--ep;

	outString = inLine.substr(sp, ep - sp);
}

bool	ExtractFixedRecordLong(
				const string&		inLine,
				int					inBegin,
				int					inEnd,
				long&				outLong)
{
	string	foo;
	ExtractFixedRecordString(inLine, inBegin, inEnd, foo);
	if (foo.empty())	return false;
	outLong = strtol(foo.c_str(), NULL, 10);
	return true;
}

bool	ExtractFixedRecordUnsignedLong(
				const string&		inLine,
				int					inBegin,
				int					inEnd,
				unsigned long&		outUnsignedLong)
{
	string	foo;
	ExtractFixedRecordString(inLine, inBegin, inEnd, foo);
	if (foo.empty())	return false;
	outUnsignedLong = strtoul(foo.c_str(), NULL, 10);
	return true;
}

#pragma mark -

struct	XPointPool::XPointPoolImp {

	struct	p_info {
		float xyz[3];
		float st[2];
	};
	vector<p_info>			pts;
	map<string, int>	index;

	void	clear()
	{
		pts.clear();
		index.clear();
	}

	int		count(void)
	{
		return pts.size();
	}

	int		accumulate(const float xyz[3], const float st[2])
	{
		static	char	buf[256];
		sprintf(buf,"%08X%08X%08X|%08x%08x",
			*(reinterpret_cast<const int*>(xyz+0)),
			*(reinterpret_cast<const int*>(xyz+1)),
			*(reinterpret_cast<const int*>(xyz+2)),
			*(reinterpret_cast<const int*>(st +0)),
			*(reinterpret_cast<const int*>(st +1)));
		string	key(buf);
		map<string, int>::iterator i = index.find(key);
		if (i != index.end()) return i->second;
		p_info	p;
		memcpy(p.xyz, xyz, sizeof(p.xyz));
		memcpy(p.st, st, sizeof(p.st));
		pts.push_back(p);
		index.insert(map<string,int>::value_type(key, (int)pts.size()));
		pts.push_back(p);
		return pts.size()-1;
	}

	void	get(int i, float xyz[3], float st[2])
	{
		p_info& p = pts[i];
		memcpy(xyz,p.xyz,sizeof(p.xyz));
		memcpy(st,p.st,sizeof(p.st));
	}
};

XPointPool::XPointPool()
{
	mImp = new XPointPoolImp;
}

XPointPool::~XPointPool()
{
	delete mImp;
}

void	XPointPool::clear()
{
	mImp->clear();
}

int		XPointPool::accumulate(const float xyz[3], const float st[2])
{
	return mImp->accumulate(xyz, st);
}

void	XPointPool::get(int index, float xyz[3], float st[2])
{
	mImp->get(index,xyz,st);
}

int		XPointPool::count(void)
{
	return mImp->count();
}

