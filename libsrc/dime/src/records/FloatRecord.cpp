/**************************************************************************\
 * 
 *  FILE: FloatRecord.cpp
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
  \class dimeFloatRecord dime/records/FloatRecord.h
  \brief The dimeFloatRecord class is a container class for float records.
*/

#include <dime/records/FloatRecord.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>

/*!
  Constructor
*/

dimeFloatRecord::dimeFloatRecord(const int group_code, const float val)
  : dimeRecord(group_code)
{
  this->setValue(val);
}

//!

dimeRecord *
dimeFloatRecord::copy(dimeMemHandler * const mh) const
{
  return new(mh) dimeFloatRecord(this->groupCode, this->value);
}

/*!
  Sets the float value.
*/

void 
dimeFloatRecord::setValue(const float val)
{
  this->value = val;
}

//!

float
dimeFloatRecord::getValue() const
{
  return this->value;
}

//!

int 
dimeFloatRecord::typeId() const
{
  return dimeBase::dimeFloatRecordType;
}

//!

bool 
dimeFloatRecord::read(dimeInput * const in)
{
  return in->readFloat(this->value);
}

//!

bool 
dimeFloatRecord::write(dimeOutput * const out)
{
  if (dimeRecord::write(out)) {
    return out->writeFloat(this->value);
  }
  return false;
}

//!

void 
dimeFloatRecord::setValue(const dimeParam &param, dimeMemHandler * const)
{
  this->value = param.float_data;
}

//!

void 
dimeFloatRecord::getValue(dimeParam &param) const
{
  param.float_data = this->value;
}

