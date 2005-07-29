/**************************************************************************\
 * 
 *  FILE: UnknownClass.h
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

#ifndef DIME_UNKNOWNCLASS_H
#define DIME_UNKNOWNCLASS_H

#include <dime/classes/Class.h>

class dimeMemHandler;

class DIME_DLL_API dimeUnknownClass : public dimeClass 
{
public:
  dimeUnknownClass(const char * const name, dimeMemHandler * const memhandler);
  virtual ~dimeUnknownClass();

  virtual dimeClass *copy(dimeModel * const model) const;
  
  virtual const char *getDxfClassName() const;
  virtual bool write(dimeOutput * const out);
  virtual int typeId() const;
  virtual int countRecords() const;
  
private:
  char *dxfClassName;
  
}; // class dimeUnknownClass

#endif // ! DIME_UNKNOWNCLASS_H

