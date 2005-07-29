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

//#ifdef macintosh
#if 0

#include <string.h>
#include <stdlib.h>

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
