/**************************************************************************\
 * 
 *  FILE: Basic.cpp
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

/*!
  \class dimeParam dime/Basic.h
  \brief The dimeParam class is a union of the different parameter types.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <math.h> /* isinf(), isnan(), finite() */
#include <float.h> /* _fpclass(), _isnan(), _finite() */

#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif /* HAVE_IEEEFP_H */

#ifdef macintosh
char *strdup(const char *istr)
{
  int len;
  char* rstr;
  len = strlen( istr ) + 1;
  rstr = (char*)malloc( (sizeof(char) * len) );
  strcpy(rstr, istr);
  rstr[len]='\0';
  return rstr;
}
#endif // macintosh

/**************************************************************************/
// copied from Coin/src/tdibits.c
//

/* Returns -1 if value equals negative infinity, +1 if it is equal to
   positive infinity, or 0 if the number is not infinite.
   
   Note that on some systems, this method will always return 0
   (i.e. false positives).
*/

int
dime_isinf(double value)
{
#ifdef HAVE_ISINF
  return isinf(value);
#elif defined(HAVE_FPCLASS)
  if (fpclass(value) == FP_NINF) { return -1; }
  if (fpclass(value) == FP_PINF) { return +1; }
  return 0;
#elif defined(HAVE__FPCLASS)
  if (_fpclass(value) == _FPCLASS_NINF) { return -1; }
  if (_fpclass(value) == _FPCLASS_PINF) { return +1; }
  return 0;
#else
  /* FIXME: it might be possible to investigate the fp bits and decide
     in a portable manner whether or not they represent an infinite. A
     groups.google.com search turned up inconclusive. 20030919
     mortene. */
  return 0;
#endif
}

/* Returns 0 if the bitpattern of the \a value argument is not a valid
   floating point number, otherwise returns non-zero.

   Note that on some systems, this method will always return 0
   (i.e. false positives).
*/
int
dime_isnan(double value)
{
#ifdef HAVE_ISNAN
  return isnan(value);
#elif defined(HAVE__ISNAN)
  return _isnan(value);
#elif defined(HAVE_FPCLASS)
  if (fpclass(value) == FP_SNAN) { return 1; }
  if (fpclass(value) == FP_QNAN) { return 1; }
  return 0;
#elif defined(HAVE__FPCLASS)
  if (_fpclass(value) == _FPCLASS_SNAN) { return 1; }
  if (_fpclass(value) == _FPCLASS_QNAN) { return 1; }
  return 0;
#else
  /* FIXME: it might be possible to investigate the fp bits and decide
     in a portable manner whether or not they represent a NaN. A
     groups.google.com search turned up inconclusive. 20030919
     mortene. */
  return 0;
#endif
}

/* Returns 0 if the bitpattern of the \a value argument is not a valid
   floating point number, or an infinite number, otherwise returns
   non-zero.

   Note that on some systems, this method will always return 1
   (i.e. false positives).
*/
int
dime_finite(double value)
{
#ifdef HAVE_FINITE
  return finite(value);
#elif defined(HAVE__FINITE)
  return _finite(value);
#else
  return !dime_isinf(value) && !dime_isnan(value);
#endif
}
