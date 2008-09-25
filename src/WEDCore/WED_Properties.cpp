/*
 * Copyright (c) 2007, Laminar Research.
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

/*
 *  WED_Properties.cpp
 *  SceneryTools
 *
 *  Created by Benjamin Supnik on 1/24/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "WED_Properties.h"


WED_Properties::WED_Properties(sqlite3 * db)	:
	mMakeTable(db,"CREATE TABLE properties(key VARCHARY PRIMARY KEY,value VARCHAR);"),
	mMakeIndex(db,"CREATE INDEX properties_idx ON properties(key);"),
	mCountKeys		(db,"SELECT COUNT(*) FROM properties WHERE key=@key;","@key"),
	mGetKey			(db,"SELECT value FROM properties WHERE key=@key;","@key"),
	mSetKey			(db,"UPDATE properties SET value=@value WHERE key=@key;","@key,@value")
{
}


WED_Properties::~WED_Properties()
{
}

int	WED_Properties::exists(const char * key)
{
	sql_row1<string> binding(key);
	sql_row1<int> result;
	mCountKeys.simple_exec(binding, result);
	return result.a;
}

double	WED_Properties::getd(const char * key)
{
	sql_row1<string> binding(key);
	sql_row1<double> result;
	mGetKey.simple_exec(binding, result);
	return result.a;
}

double	WED_Properties::geti(const char * key)
{
	sql_row1<string> binding(key);
	sql_row1<int> result;
	mGetKey.simple_exec(binding, result);
	return result.a;
}

string	WED_Properties::gets(const char * key)
{
	sql_row1<string> binding(key);
	sql_row1<string> result;
	mGetKey.simple_exec(binding, result);
	return result.a;
}

void	WED_Properties::setd(const char * key, double		 v)
{
	sql_row2<string,double> binding(key, v);
	mSetKey.simple_exec(binding);
}

void	WED_Properties::seti(const char * key, int			 v)
{
	sql_row2<string,int> binding(key, v);
	mSetKey.simple_exec(binding);
}

void	WED_Properties::sets(const char * key, const string& v)
{
	sql_row2<string,string> binding(key, v);
	mSetKey.simple_exec(binding);
}


