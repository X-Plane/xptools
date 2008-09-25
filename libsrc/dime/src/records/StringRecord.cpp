/**************************************************************************\
 *
 *  FILE: StringRecord.cpp
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
  \class dimeStringRecord dime/records/StringRecord.h
  \brief The dimeStringRecord class is a container class for string records.
*/

#include <dime/records/StringRecord.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <string.h>

/*!
  Constructor.
*/

dimeStringRecord::dimeStringRecord(const int group_code)
  : dimeRecord(group_code)
{
  this->string = NULL;
}

/*!
  Destructor.
*/

dimeStringRecord::~dimeStringRecord()
{
  delete[] this->string;
}

//!

dimeRecord *
dimeStringRecord::copy(dimeMemHandler * const mh) const
{
  dimeStringRecord *s= new(mh) dimeStringRecord(this->groupCode);
  if (s) {
    s->setString(this->string, mh);
  }
  return s;
}

/*!
  Will store a copy of string \a s. If \a memhandler != NULL, it
  will be used to allocate the needed memory. If \a memhandler == NULL,
  the memory will be allocated from the heap.
*/

bool
dimeStringRecord::setString(const char * const s,
			   dimeMemHandler * const memhandler)
{
  DXF_STRCPY(memhandler, this->string, s);
  return this->string != NULL;
}

/*!
  Sets the objects string pointer to point to 's'. Be aware that if
  the destructor is called for this object, the object will attempt to
  delete the string. \sa dimeStringRecord::setString().
*/

void
dimeStringRecord::setStringPointer(char * const s)
{
  this->string = s;
}

/*!
  Returns a pointer to the string.
*/

char *
dimeStringRecord::getString()
{
  return string;
}

//!

bool
dimeStringRecord::isEndOfSectionRecord() const
{
  return (this->groupCode == 0) && !strcmp(string, "ENDSEC");
}

//!

bool
dimeStringRecord::isEndOfFileRecord() const
{
  return (this->groupCode == 0) && !strcmp(string, "EOF");
}

//!

int
dimeStringRecord::typeId() const
{
  return dimeBase::dimeStringRecordType;
}

//!

bool
dimeStringRecord::read(dimeInput * const in)
{
  this->string = NULL;
  const char *ptr = in->readString();
  if (ptr) return this->setString(ptr, in->getMemHandler());
  else return false;
}

//!

bool
dimeStringRecord::write(dimeOutput * const out)
{
  if (dimeRecord::write(out)) { // write group code
    return out->writeString(this->string);
  }
  return false;
}

//!

void
dimeStringRecord::setValue(const dimeParam &param, dimeMemHandler * const memhandler)
{
  if (memhandler) {
    this->string = memhandler->stringAlloc(param.string_data);
  }
  else {
    this->string = new char[strlen(param.string_data)+1];
    if (this->string) strcpy(this->string, param.string_data);
  }
}

//!

void
dimeStringRecord::getValue(dimeParam &param) const
{
  param.string_data = this->string;
}

