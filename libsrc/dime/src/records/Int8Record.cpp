/**************************************************************************\
 * 
 *  FILE: Int8Record.cpp
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
  \class dimeInt8Record dime/records/Int8Record.h
  \brief The dimeInt8Record class is a container class for 8-bit integer records.
*/

#include <dime/records/Int8Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>

/*!
  Constructor
*/

dimeInt8Record::dimeInt8Record(const int group_code, const int8 val)
  : dimeRecord(group_code)
{
  this->setValue(val);
}

//!

dimeRecord *
dimeInt8Record::copy(dimeMemHandler * const mh) const
{
  return new(mh) dimeInt8Record(this->groupCode, this->value);
}

/*!
  Sets the value.
*/

void 
dimeInt8Record::setValue(const int8 val)
{
  this->value = val;
}

/*!
  Returns the int8 value.
*/

int8
dimeInt8Record::getValue() const
{
  return this->value;
}

//!

int 
dimeInt8Record::typeId() const
{
  return dimeBase::dimeInt8RecordType;
}

//!

bool 
dimeInt8Record::read(dimeInput * const in)
{
  return in->readInt8(this->value);
}

//!

bool 
dimeInt8Record::write(dimeOutput * const out)
{
  if (dimeRecord::write(out)) {
    return out->writeInt8(this->value);
  }
  return false;
}

//!

void 
dimeInt8Record::setValue(const dimeParam &param, dimeMemHandler * const )
{
  this->value = param.int8_data;
}

//!

void 
dimeInt8Record::getValue(dimeParam &param) const
{
  param.int8_data = this->value;
}

