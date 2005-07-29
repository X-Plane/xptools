/**************************************************************************\
 * 
 *  FILE: UCSTable.h
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

#ifndef DIME_UCSTABLE_H
#define DIME_UCSTABLE_H

#include <dime/tables/TableEntry.h>
#include <dime/util/Linear.h>

class DIME_DLL_API dimeUCSTable : public dimeTableEntry 
{
public:
  dimeUCSTable();

  virtual dimeTableEntry *copy(dimeModel * const model) const;
  virtual const char *getTableName() const;

  const dimeVec3f &getOrigin() const;
  const dimeVec3f &getXaxis() const;
  const dimeVec3f &getYaxis() const;
  
  void setOrigin(const dimeVec3f &v);
  void setXaxis(const dimeVec3f &v);
  void setYaxis(const dimeVec3f &v);
  
  virtual bool write(dimeOutput * const out);
  virtual int typeId() const;
  virtual int countRecords() const;

protected:
  virtual bool handleRecord(const int groupcodes,
			    const dimeParam &param,
			    dimeMemHandler * const memhandler);
  
private:
  dimeVec3f origin;
  dimeVec3f xaxis;
  dimeVec3f yaxis;

}; // class dimeUCSTable

inline const dimeVec3f &
dimeUCSTable::getOrigin() const
{
  return this->origin;
}

inline const dimeVec3f &
dimeUCSTable::getXaxis() const
{
  return this->xaxis;
}

inline const dimeVec3f &
dimeUCSTable::getYaxis() const
{
  return this->yaxis;
}

inline void 
dimeUCSTable::setOrigin(const dimeVec3f &v)
{
  this->origin = v;
}

inline void 
dimeUCSTable::setXaxis(const dimeVec3f &v)
{
  this->origin = v;
}

inline void 
dimeUCSTable::setYaxis(const dimeVec3f &v)
{
  this->origin = v;
}

#endif // ! DIME_UCSTABLE_H

