/**************************************************************************\
 * 
 *  FILE: Int16Record.cpp
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
  \class dimeInt16Record dime/records/Int16Record.h
  \brief The dimeInt16Record class is a container class for 16-bit integer records.
*/

#include <dime/records/Int16Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>

/*!
  Constructor
*/

dimeInt16Record::dimeInt16Record(const int group_code, const int16 val)
  :dimeRecord(group_code)
{
  this->setValue(val);
}

//!

dimeRecord *
dimeInt16Record::copy(dimeMemHandler * const mh) const
{
  return new(mh) dimeInt16Record(this->groupCode, this->value);
}

/*!
  Sets the int16 value.
*/

void 
dimeInt16Record::setValue(const int16 val)
{
  this->value = val;
}

/*!
  Returns the int16 value.
*/

int16
dimeInt16Record::getValue() const
{
  return this->value;
}

//!

int 
dimeInt16Record::typeId() const
{
  return dimeBase::dimeInt16RecordType;
}

//!

bool 
dimeInt16Record::read(dimeInput * const in)
{
  return in->readInt16(this->value);
}

//!

bool 
dimeInt16Record::write(dimeOutput * const out)
{
  if (dimeRecord::write(out)) {
    return out->writeInt16(this->value);
  }
  return false;
}

//!

void 
dimeInt16Record::setValue(const dimeParam &param, dimeMemHandler * const )
{
  this->value = param.int16_data;
}

//!

void 
dimeInt16Record::getValue(dimeParam &param) const
{
  param.int16_data = this->value;
}

