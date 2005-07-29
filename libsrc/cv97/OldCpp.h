/******************************************************************
*
*	VRML library for C++
*
*	Copyright (C) Satoshi Konno 1996-1997
*
*	File:	OldCpp.h
*
******************************************************************/

#ifndef _OLDCPP_H_
#define _OLDCPP_H_

#include <string.h>

typedef unsigned char bool;
#if !true && !false
#define true ((unsigned char)1)
#define false ((unsigned char)0)
#endif

#define memcpy MemoryCopy

inline void *MemoryCopy(void *dest, const void *src, size_t count)
{
	strncpy((char *)dest, (char *)src, count);
	return dest;
}

#endif

