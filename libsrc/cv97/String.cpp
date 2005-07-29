/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	String.cpp
*
******************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "String.h"

////////////////////////////////////////////////////
// STL Version
////////////////////////////////////////////////////

#ifdef SUPPORT_STL

String::String() 
{
}

String::String(char value[]) 
{
	setValue(value);
}

String::String(char value[], int offset, int count) 
{
	setValue(value, offset, count);
}

String::~String() 
{
}

void String::setValue(char value[]) 
{
	deleteValue();
	if (!value)
		return;
	if (strlen(value) <= 0)
		return;
	mValue = value;
}

void String::setValue(char value[], int offset, int count) 
{ 
	if (!value && (int)strlen(value) < (offset + count)) 
		return;
	std::string orgValue = value;
	mValue = orgValue.substr(offset, count);
}

char *String::getValue() 
{
	return (char *)mValue.c_str();
}

void String::deleteValue() 
{
	mValue = "";
}

int String::length() 
{
	return mValue.length();
}

char String::charAt(int  index) 
{
	return mValue[index];
}

#endif

////////////////////////////////////////////////////
// Normal Version
////////////////////////////////////////////////////

#ifndef SUPPORT_STL

String::String() 
{
	mValue = NULL;
}

String::String(char value[]) 
{
	mValue = NULL;
	setValue(value);
}

String::String(char value[], int offset, int count) 
{
	mValue = NULL;
	setValue(value, offset, count);
}

String::~String() 
{
	deleteValue();
}

void String::setValue(char value[]) 
{
	deleteValue();
	if (!value)
		return;
	if (strlen(value) <= 0)
		return;
	mValue = new char[strlen(value)+1];
	strcpy(mValue, value);
}

void String::setValue(char value[], int offset, int count) 
{ 
	deleteValue();
	if (!value && (int)strlen(value) < (offset + count)) 
		return;
	mValue = new char[count+1];
	strncpy(mValue, &value[offset], count);
}

char *String::getValue() 
{
	return mValue;
}

void String::deleteValue() 
{
	delete[] mValue;
	mValue = NULL;
}

int String::length() 
{
	if (!mValue)
		return 0;
	return strlen(mValue);
}

char String::charAt(int  index) 
{
	return mValue[index];
}

#endif

////////////////////////////////////////////////////
// Common
////////////////////////////////////////////////////

int String::compareTo(char *anotherString) 
{
	char *value = getValue();
	if (!value || !anotherString)
		return -1;
	return strcmp(value, anotherString);
}

void String::copyValueOf(char data[]) 
{
	if (!data)
		return;
	strcpy(data, getValue());
}

void String::copyValueOf(char  data[], int  offset, int count) 
{
	if (!data)
		return;
	char *value = getValue();
	strncpy(data, &value[offset], count);
}

int String::startsWith(char *prefix) 
{
	if (!prefix)
		return -1;
	return regionMatches(0, prefix, 0, strlen(prefix));
}

int String::endsWith(char *suffix) 
{
	if (!suffix)
		return -1;
	return regionMatches(length()-strlen(suffix), suffix, 0, strlen(suffix));
}

int String::regionMatchesIgnoreCase(int toffset, char *other, int ooffset, int len)
{
	if (!other)
		return -1;
	
	int n;

	int value1Len = length();
	char *value1 = new char[value1Len+1]; 
	strcpy(value1, getValue());
	for (n=0; n<value1Len; n++)
		value1[n] = (char)toupper(value1[n]);

	int value2Len = strlen(other);
	char *value2 = new char[value2Len+1]; 
	strcpy(value2, other);
	for (n=0; n<value2Len; n++)
		value2[n] = (char)toupper(value2[n]);
		
	int ret = regionMatches(toffset, other, ooffset, len);

	delete value1;
	delete value2;

	return ret;
}

int String::regionMatches(int toffset, char *other, int ooffset, int len) 
{
	char *value = getValue();
	if (!value || !other)
		return -1;
	if (length() < toffset)
		return -1;
	if ((int)strlen(other) < ooffset + len)
		return -1;
	if (toffset<0 || ooffset<0)
		return -1;
	return strncmp(&value[toffset], &other[ooffset], len);
}
