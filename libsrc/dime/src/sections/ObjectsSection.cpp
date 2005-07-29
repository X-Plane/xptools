/**************************************************************************\
 * 
 *  FILE: ObjectsSection.cpp
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
  \class dimeObjectsSection dime/sections/ObjectsSection.h
  \brief The dimeObjectsSection object handles an OBJECTS \e section.
*/

#include <dime/sections/ObjectsSection.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>
#include <dime/objects/Object.h>
#include <dime/Model.h>
#include <string.h>

static const char sectionName[] = "OBJECTS";

/*!
  Constructor.
*/

dimeObjectsSection::dimeObjectsSection(dimeMemHandler * const memhandler)
  : dimeSection(memhandler)
{
}

/*!
  Destructor.
*/

dimeObjectsSection::~dimeObjectsSection()
{
  if (!this->memHandler) {
    for (int i = 0; i < this->objects.count(); i++)
      delete this->objects[i];
  }
}

//!

dimeSection *
dimeObjectsSection::copy(dimeModel * const model) const
{
  dimeMemHandler *memh = model->getMemHandler();
  dimeObjectsSection *os = new dimeObjectsSection(memh); 
  bool ok = os != NULL;

  int num  = this->objects.count();
  if (ok && num) {
    os->objects.makeEmpty(num);
    for (int i = 0; i < num; i++) {
      os->objects.append(this->objects[i]->copy(model));
      if (os->objects[i] == NULL) {
	ok = false;
	break;
      }
    }
  }
  
  if (!ok) {
    if (!memh) delete os;
    os = NULL;
  }
//  sim_trace("objects section copy: %p\n", os);
  return os;
}

//!

bool 
dimeObjectsSection::read(dimeInput * const file)
{
  int32 groupcode;
  const char *string;
  bool ok = true;
  dimeObject *object = NULL;
  dimeMemHandler *memhandler = file->getMemHandler();
  this->objects.makeEmpty(64);

//  sim_trace("Reading section: OBJECTS\n");

  while (true) {
    if (!file->readGroupCode(groupcode) || groupcode != 0) {
      fprintf(stderr,"Error reading objects groupcode: %d.\n", groupcode);
//      sim_warning("Error reading objects groupcode: %d.\n", groupcode);
      ok = false;
      break;
    }
    string = file->readString();
    if (!strcmp(string, "ENDSEC")) break; 
    object = dimeObject::createObject(string, memhandler);
    if (object == NULL) {
      fprintf(stderr,"error creating object: %s.\n", string);
//      sim_warning("error creating object: %s.\n", string);
      ok = false;
      break;
    }
    if (!object->read(file)) {
      fprintf(stderr,"error reading object: %s.\n", string);
//      sim_warning("error reading object: %s.\n", string);
      ok = false;
      break;
    }
    this->objects.append(object);
  }
  return ok;
}

//!

bool 
dimeObjectsSection::write(dimeOutput * const file)
{
//  sim_trace("Writing section: OBJECTS\n");

  file->writeGroupCode(2);
  file->writeString(sectionName);
 
  int i, n = this->objects.count();
  for (i = 0; i < n; i++) {
    if (!this->objects[i]->write(file)) break;
  }
  if (i == n) {
    file->writeGroupCode(0);
    file->writeString("ENDSEC");
    return true;
  }
  return false;
}

//!

int 
dimeObjectsSection::typeId() const
{
  return dimeBase::dimeObjectsSectionType;
}

//!

int
dimeObjectsSection::countRecords() const
{
  int cnt = 0;
  int n = this->objects.count();
  for (int i = 0; i < n; i++)
    cnt += this->objects[i]->countRecords();
  return cnt + 2; // two additional records are written in write()
}

//!

const char *
dimeObjectsSection::getSectionName() const
{
  return sectionName;
}

/*!
  Returns the number of objects in this section. 
*/

int 
dimeObjectsSection::getNumObjects() const
{
  return this->objects.count();
}

/*!
  Returns the object at index \a idx.
*/

dimeObject *
dimeObjectsSection::getObject(const int idx)
{
  assert(idx >= 0 && idx < this->objects.count());
  return this->objects[idx];
}

/*!
  Removes (and deletes if no memhandler is used) the object at index \a idx.
*/

void 
dimeObjectsSection::removeObject(const int idx)
{
  assert(idx >= 0 && idx < this->objects.count());
  if (!this->memHandler) delete this->objects[idx];
  this->objects.removeElem(idx);
}

/*!
  Inserts a new object at index \a idx. If \a idx is negative, the
  object will be inserted at the end of the list of objects.
*/

void 
dimeObjectsSection::insertObject(dimeObject * const object, const int idx)
{
  if (idx < 0) this->objects.append(object);
  else {
    assert(idx <= this->objects.count());
    this->objects.insertElem(idx, object);
  }
}

