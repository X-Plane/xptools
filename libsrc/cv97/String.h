/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	String.h
*
******************************************************************/

#ifndef _CV97_STRING_H_
#define _CV97_STRING_H_

#ifdef SUPPORT_STL
#include <string>
#endif

class  String 
{

#ifdef SUPPORT_STL
	std::string	mValue;
#else
	char	*mValue;
#endif

public:

	String();
	String(char value[]);
	String(char value[], int offset, int count); 

	~String();

	void setValue(char value[]);
	void setValue(char value[], int offset, int count); 
	char *getValue();
	void deleteValue();
	int length();
	char charAt(int  index);
	int compareTo(char *anotherString);
	int compareToIgnoreCase(char *anotherString);
	void concat(char *str);
	void copyValueOf(char data[]);
	void copyValueOf(char  data[], int  offset, int count);
	int regionMatches(int toffset, char *other, int ooffset, int len);
	int regionMatchesIgnoreCase(int toffset, char *other, int ooffset, int len);
	int startsWith(char *prefix);
	int endsWith(char *suffix);
};

#endif
