/**************************************************************************\
 * 
 *  FILE: Object.cpp
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
  \class dimeObject dime/objects/Object.h
  \brief The dimeObject class is the superclass for the \e object classes.
*/

#include <dime/objects/Object.h>
#include <dime/objects/UnknownObject.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>

#include <string.h>

/*!
  \fn const char * dimeObject::getObjectName() const = 0
*/

/*!
  \fn dimeObject * dimeObject::copy(dimeModel * const model) const = 0
*/

/*!
  \fn int dimeObject::typeId() const = 0
*/

/*!
  Constructor.
*/

dimeObject::dimeObject() 
  : dimeRecordHolder(0) // objects are separated by group code 0
{
}

/*!
  Destructor.
*/

dimeObject::~dimeObject()
{
}

/*!
  Copies the common and unobjectified records.
*/

bool
dimeObject::copyRecords(dimeObject * const myobject, dimeModel * const model) const
{
  dimeMemHandler *memh = model->getMemHandler();
  bool ok = dimeRecordHolder::copyRecords(myobject, memh);
  return ok;
}

/*!
  Writes common and unknown object records to file.
*/

bool 
dimeObject::write(dimeOutput * const file)
{
  return dimeRecordHolder::write(file);
}

/*!
  Static function which creates an object based on its name. 
*/

dimeObject *
dimeObject::createObject(const char * const name, 
		      dimeMemHandler * const memhandler)
{
  return new(memhandler) dimeUnknownObject(name, memhandler);
}

//!

int 
dimeObject::countRecords() const
{
  return dimeRecordHolder::countRecords();
}

//!

bool 
dimeObject::isOfType(const int thetypeid) const
{
  return thetypeid == dimeObjectType ||
    dimeRecordHolder::isOfType(thetypeid);
}

/*!
  Reads an object from \a in. Can be overloaded by subobjects, but in most
  cases this will not be necessary.

  \sa dimeObject::handleRecord().
*/

bool 
dimeObject::read(dimeInput * const file)
{
  return dimeRecordHolder::read(file);
}

//!

bool 
dimeObject::handleRecord(const int /*groupcode*/,
			const dimeParam &/*param*/,
			dimeMemHandler * const /*memhandler*/)
{
  // no groupcodes supported yet...
  return false;
}

/*!
  \fn void dimeObject::print() const
*/

