/**************************************************************************\
 * 
 *  FILE: UnknownEntity.cpp
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
  \class dimeUnknownEntity dime/entities/UnknownEntity.h
  \brief The dimeUnknownEntity class reads and writes undefined \e entity classes.
*/

#include <dime/entities/UnknownEntity.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>
#include <dime/records/Record.h>
#include <string.h>

/*!
  Constructor.
*/

dimeUnknownEntity::dimeUnknownEntity(const char * const name, 
				   dimeMemHandler * const memhandler)
{
  DXF_STRCPY(memhandler, this->entityName, name);
}

/*!
  Destructor. Should only be called if no memhandler is used.
*/

dimeUnknownEntity::~dimeUnknownEntity()
{
  delete this->entityName;
}

//!

dimeEntity *
dimeUnknownEntity::copy(dimeModel * const model) const
{
  dimeMemHandler *memh = model->getMemHandler();
  dimeUnknownEntity *u = new(memh) dimeUnknownEntity(this->entityName, memh);
  if (!this->copyRecords(u, model)) {
    // check if allocated on heap.
    if (!memh) delete u;
    u = NULL;
  }
  return u;
}

//!

bool 
dimeUnknownEntity::write(dimeOutput * const file)
{
  dimeEntity::preWrite(file);
  return dimeEntity::write(file);
}

//!

int 
dimeUnknownEntity::typeId() const
{
  return dimeBase::dimeUnknownEntityType;
}

//!

int
dimeUnknownEntity::countRecords() const
{
  return 1 + dimeEntity::countRecords();
}

//!

const char *
dimeUnknownEntity::getEntityName() const
{
  return this->entityName;
}

