/**************************************************************************\
 * 
 *  FILE: TableEntry.cpp
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
  \class dimeTableEntry dime/tables/TableEntry.h
  \brief The dimeTableEntry class is the superclass for all table classes.
*/

#include <dime/tables/TableEntry.h>
#include <dime/tables/UnknownTable.h>
#include <dime/tables/LayerTable.h>
#include <dime/tables/UCSTable.h>
#include <dime/records/Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>

#include <string.h>
#include <ctype.h>

/*!
  Constructor.
*/

dimeTableEntry::dimeTableEntry()
  : dimeRecordHolder(0)  // table entries are separated by group code 0
{
}

/*!
  Destructor.
*/

dimeTableEntry::~dimeTableEntry()
{
}

/*!
  Copies the unclassified records.
*/

bool
dimeTableEntry::copyRecords(dimeTableEntry * const table,
                           dimeModel * const model) const
{
  return dimeRecordHolder::copyRecords(table, model->getMemHandler());
}

//!

bool 
dimeTableEntry::write(dimeOutput * const /* file */)
{
  // moved to preWrite()
  //return dimeRecordHolder::write(file);
  return true;
}

//!

bool 
dimeTableEntry::read(dimeInput * const file)
{
  return dimeRecordHolder::read(file);
}

/*!
  Static function that creates a table based on its name. 
*/

dimeTableEntry *
dimeTableEntry::createTableEntry(const char * const name, 
				dimeMemHandler * const memhandler)
{
  if (!strcmp(name, "LAYER")) {
    return new(memhandler) dimeLayerTable;
  }
  //if (!strcmp(name, "UCS")) // UCS is not used for the moment
  //  return new(memhandler) dimeUCSTable;

  return new(memhandler) dimeUnknownTable(name, memhandler);
}

/*!
  Returns the number of records for this table. Tables overloading 
  this function should first count the number of records they will write,
  then add the return value of this function to get the total number
  of records.
*/

int 
dimeTableEntry::countRecords() const
{
  return dimeRecordHolder::countRecords();
}

//!

bool 
dimeTableEntry::handleRecord(const int,
			     const dimeParam &,
			     dimeMemHandler * const)
{
  return false;
}

//!

bool 
dimeTableEntry::isOfType(const int thetypeid) const
{
  return thetypeid == dimeBase::dimeTableEntryType ||
    dimeRecordHolder::isOfType(thetypeid);
}

//!

bool
dimeTableEntry::preWrite(dimeOutput * const file)
{
  return file->writeGroupCode(0) && 
    file->writeString(this->getTableName()) &&
    dimeRecordHolder::write(file);
}

/*!
  \fn const char * dimeTableEntry::getTableName() const = 0
*/

/*!
  \fn dimeTableEntry * dimeTableEntry::copy(dimeModel * const model) const = 0
*/

/*!
  \fn int dimeTableEntry::typeId() const = 0
*/

