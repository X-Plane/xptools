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


