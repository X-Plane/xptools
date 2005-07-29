/**************************************************************************\
 * 
 *  FILE: EntitiesSection.cpp
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
  \class dimeEntitiesSection dime/sections/EntitiesSection.h
  \brief The dimeEntitiesSection class handles an ENTITIES \e section.
*/

#include <dime/sections/EntitiesSection.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>
#include <dime/util/Array.h>
#include <dime/entities/Entity.h>
#include <dime/Model.h>
#include <dime/entities/3DFace.h>
#include <dime/entities/Insert.h>
#include <dime/entities/Block.h>

#include <string.h>

static const char sectionName[] = "ENTITIES";

/*!
  Constructor.
*/

dimeEntitiesSection::dimeEntitiesSection(dimeMemHandler * const memhandler)
  : dimeSection(memhandler)
{
}

/*!
  Destructor.
*/

dimeEntitiesSection::~dimeEntitiesSection()
{
  if (!this->memHandler) {
    for (int i = 0; i < this->entities.count(); i++)
      delete this->entities[i];
  }
}

//!

dimeSection *
dimeEntitiesSection::copy(dimeModel * const model) const
{
  dimeMemHandler *memh = model->getMemHandler();
  dimeEntitiesSection *es = new dimeEntitiesSection(memh); 
  bool ok = es != NULL;

  int num  = this->entities.count();
  if (ok && num) {
    ok = dimeEntity::copyEntityArray((const dimeEntity**)
				     this->entities.constArrayPointer(), 
				     num,
				     model,
				     es->entities);
    if (!ok) {
//      sim_trace("copy entities array failed\n");
    }
  }

  if (!ok) {
    if (!memh) delete es;
    es = NULL;
  }
  return es;
}

//!

bool 
dimeEntitiesSection::read(dimeInput * const file)
{
  int32 groupcode;
  const char *string;
  bool ok = true;
  dimeEntity *entity = NULL;
  dimeMemHandler *memhandler = file->getMemHandler();
  this->entities.makeEmpty(1024);

  while (true) {
    if (!file->readGroupCode(groupcode) || groupcode != 0) {
      fprintf( stderr, "Error reading groupcode: %d.\n", groupcode);
      ok = false;
      break;
    }
    string = file->readString();
    if (!strcmp(string, "ENDSEC")) break;

    entity = dimeEntity::createEntity(string, memhandler);
    if (entity == NULL) {
      fprintf( stderr, "Error creating entity: %s.\n", string);
      ok = false;
      break;
    }
    if (!entity->read(file)) {
      fprintf( stderr, "Error reading entity: %s.\n", string);
      ok = false;
      break;
    }
    this->entities.append(entity);
  }
  return ok;
}

//!

bool 
dimeEntitiesSection::write(dimeOutput * const file)
{
//  sim_trace("Writing section: ENTITIES\n");

  file->writeGroupCode(2);
  file->writeString(sectionName);
 
  int i, n = this->entities.count();
  for (i = 0; i < n; i++) {
    if (!this->entities[i]->write(file)) break;
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
dimeEntitiesSection::typeId() const
{
  return dimeBase::dimeEntitiesSectionType;
}

/*!
  This function should be called after loading has ended, and will
  find all forward BLOCK references.
*/

void
dimeEntitiesSection::fixReferences(dimeModel * const model)
{
  int i, n = this->entities.count();
  for (i = 0; i < n; i++) 
    this->entities[i]->fixReferences(model);
}

//!

int
dimeEntitiesSection::countRecords() const
{
  int cnt = 0;
  int n = this->entities.count();
  for (int i = 0; i < n; i++)
    cnt += this->entities[i]->countRecords();
  return cnt + 2; // two records are written in write()
}

//!

const char *
dimeEntitiesSection::getSectionName() const
{
  return sectionName;
}

/*!
  Returns the number of entities in this section. Be aware that a POLYLINE
  entity with attached VERTEX entities will count as a single entity.
*/

int 
dimeEntitiesSection::getNumEntities() const
{
  return this->entities.count();
}

/*!
  Returns the entity at index \a idx.
*/

dimeEntity *
dimeEntitiesSection::getEntity(const int idx)
{
  assert(idx >= 0 && idx < this->entities.count());
  return this->entities[idx];
}

/*!
  Removes (and deletes if no memhandler is used) the entity at index \a idx.
*/

void 
dimeEntitiesSection::removeEntity(const int idx)
{
  assert(idx >= 0 && idx < this->entities.count());
  if (!this->memHandler) delete this->entities[idx];
  this->entities.removeElem(idx);
}

/*!
  Inserts a new entity at index \a idx. If \a idx is negative, the
  entity will be inserted at the end of the list of entities.

  Entities should never be allocated on the stack. Use the
  new and delete operators to create/destroy entities.
*/

void 
dimeEntitiesSection::insertEntity(dimeEntity * const entity, const int idx)
{
  if (idx < 0) this->entities.append(entity);
  else {
    assert(idx <= this->entities.count());
    this->entities.insertElem(idx, entity);
  }
}

