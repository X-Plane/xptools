/**************************************************************************\
 *
 *  FILE: Basic.h
 *
 *  This source file is part of DIME.
 *  Copyright (C) 1998-1999 by Systems In Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License (the accompanying file named COPYING) for more
 *  details.
 *
 **************************************************************************
 *
 *  If you need DIME for a non-GPL project, contact Systems In Motion
 *  to acquire a Professional Edition License:
 *
 *  Systems In Motion                                   http://www.sim.no/
 *  Prof. Brochs gate 6                                       sales@sim.no
 *  N-7030 Trondheim                                   Voice: +47 22114160
 *  NORWAY                                               Fax: +47 67172912
 *
\**************************************************************************/

#ifndef DIME_BASIC_H
#define DIME_BASIC_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// we prefer to use floats to save mem. Applications needing
// scientific calculations should typedef this to double
typedef float dxfdouble;
// typedef double dxfdouble;

#include <float.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif // !M_PI

#define DXFABS(x) ((x)<0?-(x):(x))
#define DXFMAX(x,y) ((x)>(y)?(x):(y))
#define DXFMIN(x,y) ((x)<(y)?(x):(y))
#define DXFDEG2RAD(x) (M_PI*(x)/180.0)
#define DXFRAD2DEG(x) (180.0*(x)/M_PI)


#ifdef __sgi
#define bool int
#define true 1
#define false 0
#endif // __sgi


template <class T> inline
T DXFSQR(const T x)
{
  return x*x;
}

#if defined(__BEOS__)
#include <support/SupportDefs.h>
#else // ! defined(__BEOS__)
typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
#ifdef _WIN32
typedef long int32;
#else // ! defined(_WIN32)
typedef signed int int32;
#endif // ! defined(_WIN32)
typedef unsigned int uint32;
#endif // ! defined(__BEOS__)

#ifdef macintosh
 char* strdup( const char* );
#endif

#define ARRAY_NEW(memh, type, num) \
memh ? (type*) memh->allocMem((num)*sizeof(type)) : new type[num]

#define DXF_STRCPY(mh, d, s) \
mh ? d = mh->stringAlloc(s) : d = new char[strlen(s)+1]; if (d) strcpy(d,s)

typedef bool dimeCallbackFunc(const class dimeState * const, class dimeEntity *, void *);
typedef dimeCallbackFunc * dimeCallback;

typedef union {
  int8  int8_data;
  int16 int16_data;
  int32 int32_data;
  float float_data;
  dxfdouble double_data;
  const char *string_data;
  const char *hex_data;
} dimeParam;

/* ********************************************************************** */
/* Precaution to avoid an some errors easily made by the application
   programmer. */

#ifdef DIME_DLL_API
# error Leave the internal DIME_DLL_API define alone.
#endif /* DIME_DLL_API */
#ifdef DIME_INTERNAL
# ifdef DIME_NOT_DLL
#  error The DIME_NOT_DLL define is not supposed to be used when building the library, only when building Win32 applications.
# endif /* DIME_INTERNAL && DIME_NOT_DLL */
# ifdef DIME_DLL
#  error The DIME_DLL define is not supposed to be used when building the library, only when building Win32 applications.
# endif /* DIME_INTERNAL && DIME_DLL */
#endif /* DIME_INTERNAL */

/*
  On MSWindows platforms, one of these defines must always be set when
  building application programs:

   - "DIME_DLL", when the application programmer is using the library
     in the form of a dynamic link library (DLL)

   - "DIME_NOT_DLL", when the application programmer is using the
     library in the form of a static object library (LIB)

  Note that either DIME_DLL or DIME_NOT_DLL _must_ be defined by the
  application programmer on MSWindows platforms, or else the #error
  statement will hit. Set up one or the other of these two defines in
  your compiler environment according to how the library was built --
  as a DLL (use "DIME_DLL") or as a LIB (use "DIME_NOT_DLL").

  (Setting up defines for the compiler is typically done by either
  adding something like "/DDIME_DLL" to the compiler's argument line
  (for command-line build processes), or by adding the define to the
  list of preprocessor symbols in your IDE GUI (in the MSVC IDE, this
  is done from the "Project"->"Settings" menu, choose the "C/C++" tab,
  then "Preprocessor" from the dropdown box and add the appropriate
  define)).

  It is extremely important that the application programmer uses the
  correct define, as using "DIME_NOT_DLL" when "DIME_DLL" is correct
  will cause mysterious crashes.
 */
/* FIXME: use a feature check to see if this is a platform which can
   recognize the __declspec keyword instead of the crap #if below.
   20011201 mortene. */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
# ifdef DIME_INTERNAL
#  ifdef DIME_MAKE_DLL
#   define DIME_DLL_API __declspec(dllexport)
#  endif /* DIME_MAKE_DLL */
# else /* !DIME_INTERNAL */
#  ifdef DIME_DLL
#   ifdef DIME_NOT_DLL
#    error Do not define both DIME_DLL and DIME_NOT_DLL at the same time
#   endif
#   define DIME_DLL_API __declspec(dllimport)
#  else /* !DIME_DLL */
#   ifndef DIME_NOT_DLL
#    error Define either DIME_DLL or DIME_NOT_DLL as appropriate for your linkage! See dime/Basic.h for further instructions.
#   endif /* DIME_NOT_DLL */
#  endif /* !DIME_DLL */
# endif /* !DIME_INTERNAL */
#endif /* Microsoft Windows */

/* Empty define to avoid errors when _not_ compiling an MSWindows DLL. */
#ifndef DIME_DLL_API
# define DIME_DLL_API
#endif /* !DIME_DLL_API */

int DIME_DLL_API dime_isnan(double value);
int DIME_DLL_API dime_isinf(double value);
int DIME_DLL_API dime_finite(double value);

/* ********************************************************************** */

#endif // !DIME_BASIC_H
