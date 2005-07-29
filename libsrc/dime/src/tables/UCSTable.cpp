/**************************************************************************\
 * 
 *  FILE: UCSTable.cpp
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
  \class dimeUCSTable dime/tables/UCSTable.h
  \brief The dimeUCSTable class reads and writes UCS tables.
*/

#include <dime/tables/UCSTable.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>
#include <dime/records/Record.h>
#include <string.h>

static const char tableName[] = "UCS";

/*!
  Constructor.
*/

dimeUCSTable::dimeUCSTable()
  : origin(0,0,0), xaxis(1,0,0), yaxis(0,1,0)
{
}

//!

dimeTableEntry *
dimeUCSTable::copy(dimeModel * const model) const
{
  dimeMemHandler *memh = model->getMemHandler();
  dimeUCSTable *u = new(memh) dimeUCSTable;
  u->xaxis = this->xaxis;
  u->yaxis = this->yaxis;
  u->origin = this->origin;
  if (!this->copyRecords(u, model)) {
    // check if allocated on heap.
    if (!memh) delete u;
    u = NULL;
  }
  return u;
}

//!

const char *
dimeUCSTable::getTableName() const
{
  return tableName;
}

//!

bool 
dimeUCSTable::write(dimeOutput * const file)
{
  bool ret = true;
  file->writeGroupCode(0);
  file->writeString(tableName);
  file->writeGroupCode(10);
  file->writeDouble(this->origin[0]);
  file->writeGroupCode(20);
  file->writeDouble(this->origin[1]);
  file->writeGroupCode(30);
  file->writeDouble(this->origin[2]);

  file->writeGroupCode(11);
  file->writeDouble(this->xaxis[0]);
  file->writeGroupCode(21);
  file->writeDouble(this->xaxis[1]);
  file->writeGroupCode(31);
  file->writeDouble(this->xaxis[2]);

  file->writeGroupCode(12);
  file->writeDouble(this->yaxis[0]);
  file->writeGroupCode(22);
  file->writeDouble(this->yaxis[1]);
  file->writeGroupCode(32);
  file->writeDouble(this->yaxis[2]);
  
  ret = dimeTableEntry::write(file);
  return ret;
}

//!

int 
dimeUCSTable::typeId() const
{
  return dimeBase::dimeUCSTableType;
}

//!

bool 
dimeUCSTable::handleRecord(const int groupcode,
			  const dimeParam &param,
			  dimeMemHandler * const memhandler)
{
  switch(groupcode) {
  case 10:
  case 20:
  case 30:
    this->origin[(groupcode/10)-1] = param.double_data;
    return true;
  case 11:
  case 21:
  case 31:
    this->xaxis[(groupcode/10)-1] = param.double_data;
    return true;
  case 12:
  case 22:
  case 32:
    this->yaxis[(groupcode/10)-1] = param.double_data;
    return true;
  }
  return dimeTableEntry::handleRecord(groupcode, param, memhandler);
}

//!

int
dimeUCSTable::countRecords() const
{
  int cnt = 1 + 3 + 3 + 3; // header + origin + xaxis + yaxis
  return cnt + dimeTableEntry::countRecords();
}

