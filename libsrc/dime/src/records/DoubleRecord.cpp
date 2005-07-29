/**************************************************************************\
 * 
 *  FILE: DoubleRecord.cpp
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
  \class dimeDoubleRecord dime/records/DoubleRecord.h
  \brief The dimeDoubleRecord class is a container class for double records.
*/

#include <dime/records/DoubleRecord.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <stdio.h>

/*!
  Constructor.
*/

dimeDoubleRecord::dimeDoubleRecord(const int group_code, const dxfdouble val)
  : dimeRecord(group_code)
{
  this->setValue(val);
}

//!

dimeRecord *
dimeDoubleRecord::copy(dimeMemHandler * const mh) const
{
  return new(mh) dimeDoubleRecord(this->groupCode, this->value);
}

/*!
  Sets the floating point value.
*/

void 
dimeDoubleRecord::setValue(const dxfdouble val)
{
  this->value = val;
}

/*!
  Returns the floating point value of this record.
*/

dxfdouble
dimeDoubleRecord::getValue() const
{
  return this->value;
}

//!

int 
dimeDoubleRecord::typeId() const
{
  return dimeBase::dimeDoubleRecordType;
}

//!

bool 
dimeDoubleRecord::read(dimeInput * const in)
{
  return in->readDouble(this->value);
}

//!

bool 
dimeDoubleRecord::write(dimeOutput * const out)
{
  if (dimeRecord::write(out)) {
    return out->writeDouble(this->value);
  }
  return false;
}

//!

void 
dimeDoubleRecord::setValue(const dimeParam &param, dimeMemHandler * const)
{
  this->value = param.double_data;
}

//!

void 
dimeDoubleRecord::getValue(dimeParam &param) const
{
  param.double_data = this->value;
}

