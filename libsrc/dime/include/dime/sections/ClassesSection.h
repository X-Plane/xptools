/**************************************************************************\
 * 
 *  FILE: ClassesSection.h
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

#ifndef DIME_CLASSESSECTION_H
#define DIME_CLASSESSECTION_H

#include <dime/sections/Section.h>
#include <dime/util/Array.h>

class DIME_DLL_API dimeClassesSection : public dimeSection
{
  friend class dimeModel;

public:
  dimeClassesSection(dimeMemHandler * const memhandler = NULL);
  virtual ~dimeClassesSection();

  virtual const char *getSectionName() const; 
  virtual dimeSection *copy(dimeModel * const model) const;
  
  virtual bool read(dimeInput * const file);
  virtual bool write(dimeOutput * const file);
  virtual int typeId() const;
  virtual int countRecords() const;
  
  int getNumClasses() const;
  class dimeClass *getClass(const int idx);
  void removeClass(const int idx);
  void insertClass(dimeClass * const myclass, const int idx = -1); 

private:
  dimeArray <dimeClass*> classes;

}; // class dimeClassesSection

#endif // ! DIME_CLASSESSECTION_H

