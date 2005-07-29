/**************************************************************************\
 * 
 *  FILE: Section.h
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

#ifndef DIME_SECTION_H
#define DIME_SECTION_H

#include <dime/Basic.h>
#include <dime/Base.h>

class dimeInput;
class dimeModel;
class dimeOutput;

class DIME_DLL_API dimeSection : public dimeBase
{
public:
  dimeSection(dimeMemHandler * const memhandler);
  virtual ~dimeSection();

  virtual const char *getSectionName() const = 0;
  virtual dimeSection *copy(dimeModel * const model) const = 0;

  virtual bool read(dimeInput * const file) = 0;
  virtual bool write(dimeOutput * const file) = 0;
  virtual int typeId() const = 0;
  virtual bool isOfType(const int thetypeid) const;
  virtual int countRecords() const = 0;
  
public:
  static dimeSection *createSection(const char * const sectionname,
				   dimeMemHandler *memhandler);

protected:
  dimeMemHandler *memHandler;

}; // class dimeSection

#endif // ! DIME_SECTION_H

