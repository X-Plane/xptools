/**************************************************************************\
 * 
 *  FILE: TableEntry.h
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

#ifndef DIME_TABLEENTRY_H
#define DIME_TABLEENTRY_H

#include <dime/Base.h>
#include <dime/RecordHolder.h>

class dimeModel;

class DIME_DLL_API dimeTableEntry : public dimeRecordHolder
{
  friend class dimeUnknownTable;
  friend class dimeLayerTable;
  
public:
  dimeTableEntry();
  virtual ~dimeTableEntry();

  virtual const char *getTableName() const = 0;
  virtual bool read(dimeInput * const in);
  virtual bool write(dimeOutput * const out);
  virtual dimeTableEntry *copy(dimeModel * const model) const = 0;
  virtual int typeId() const = 0;
  virtual bool isOfType(const int thetypeid) const;
  virtual int countRecords() const;

  static dimeTableEntry *createTableEntry(const char * const name,
					 dimeMemHandler * const memhandler = NULL);
  
protected:
  bool preWrite(dimeOutput * const output);

  virtual bool handleRecord(const int groupcode,
			    const dimeParam &param,
			    dimeMemHandler * const memhandler);
  
  bool copyRecords(dimeTableEntry * const table, dimeModel * const model) const;

}; // class dimeTableEntry

#endif // ! DIME_TABLEENTRY_H

