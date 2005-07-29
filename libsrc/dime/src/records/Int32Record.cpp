/**************************************************************************\
 * 
 *  FILE: Int32Record.cpp
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
  \class dimeInt32Record dime/records/Int32Record.h
  \brief The dimeInt32Record class is a container class for 32-bit integer records.
*/

#include <dime/records/Int32Record.h>
#include <dime/Input.h>
#include <dime/Output.h>

/*!
  Constructor
*/

dimeInt32Record::dimeInt32Record(const int group_code, const int32 val)
  : dimeRecord(group_code)
{
  this->setValue(val);
}

//!

dimeRecord *
dimeInt32Record::copy(dimeMemHandler * const mh) const
{
  return new(mh) dimeInt32Record(this->groupCode, this->value);
}

/*!
  Sets the int32 value to \a val.
*/

void 
dimeInt32Record::setValue(const int32 val)
{
  this->value = val;
}

/*!
  Returns the int32 value.
*/

int32
dimeInt32Record::getValue() const
{
  return this->value;
}

//!

int 
dimeInt32Record::typeId() const
{
  return dimeBase::dimeInt32RecordType;
}

//!

bool 
dimeInt32Record::read(dimeInput * const in)
{
  return in->readInt32(this->value);
}

//!

bool 
dimeInt32Record::write(dimeOutput * const out)
{
  if (dimeRecord::write(out)) {
    return out->writeInt32(this->value);
  }
  return false;
}

//!

void 
dimeInt32Record::setValue(const dimeParam &param, dimeMemHandler * const )
{
  this->value = param.int32_data;
}

//!

void 
dimeInt32Record::getValue(dimeParam &param) const
{
  param.int32_data = this->value;
}

