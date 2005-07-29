/**************************************************************************\
 * 
 *  FILE: UnknownObject.cpp
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
  \class dimeUnknownObject dime/objects/UnknownObject.h
  \brief The dimeUnknownObject class reads and writes undefined objects.
*/

#include <dime/objects/UnknownObject.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>
#include <string.h> 

/*!
  Constructor.
*/

dimeUnknownObject::dimeUnknownObject(const char * const name, 
				 dimeMemHandler * const memhandler)
{
  DXF_STRCPY(memhandler, this->objectName, name);
}

/*!
  Destructor. Should only be called if no memhandler is used.
*/

dimeUnknownObject::~dimeUnknownObject()
{
  delete this->objectName;
}

//!

dimeObject *
dimeUnknownObject::copy(dimeModel * const model) const
{
  dimeMemHandler *memh = model->getMemHandler();
  dimeUnknownObject *u = new(memh) dimeUnknownObject(this->objectName, memh);
  if (!this->copyRecords(u, model)) {
    // check if allocated on heap.
    if (!memh) delete u;
    u = NULL;
  }
  return u;
}

//!

bool 
dimeUnknownObject::write(dimeOutput * const file)
{
  if (file->writeGroupCode(0) && file->writeString(this->objectName))
    return dimeObject::write(file);
  return false;
}

//!

int 
dimeUnknownObject::typeId() const
{
  return dimeBase::dimeUnknownObjectType;
}

//!

int
dimeUnknownObject::countRecords() const
{
  return 1 + dimeObject::countRecords();
}

//!

const char *
dimeUnknownObject::getObjectName() const
{
  return this->objectName;
}

