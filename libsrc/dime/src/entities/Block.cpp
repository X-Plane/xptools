/**************************************************************************\
 * 
 *  FILE: Block.cpp
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
  \class dimeBlock dime/entities/Block.h
  \brief The dimeBlock class handles a BLOCK \e entity.

  It cannot strictly be called an entity, as you will only find BLOCKs
  in the BLOCKS section, not in the ENTITIES section. But BLOCKs share
  a lot of attributes and functionality with "real" entities, so
  in DXFLIB, a BLOCK is called an entity.
*/

#include <dime/entities/Block.h>
#include <dime/records/Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>

static char entityName[] = "BLOCK";

/*!
  \fn const dimeVec3f &dimeBlock::getBasePoint() const
  Returns the base point of this block.
*/

/*!
  \fn void dimeBlock::setBasePoint(const dimeVec3f &v)
  Sets the base point of this entity.
*/

/*!
  \fn int dimeBlock::getNumEntities() const
  Returns the number of entities in this block.
*/

/*!
  \fn dimeEntity *dimeBlock::getEntity(const int idx)
  Returns the entity at index \a idx.
  
  \sa dimeBlock::getNumEntities()
*/

/*!
  \fn const char *dimeBlock::getName() const
  Returns the name of this block (used by INSERT to reference the block).
*/

/*!
  \fn void dimeBlock::setName(const char * const name)
  Sets the name of this block. \a name Must be a static char pointer, or
  some pointer that will not be deleted before the block, as the text string
  will not be copied. It is best to avoid using this method. Use
  dimeModel::addBlock() instead.
*/

/*!
  Constructor.
*/

dimeBlock::dimeBlock(dimeMemHandler * const memhandler)
  : flags( 0 ), name( NULL ), basePoint( 0, 0, 0 ), endblock( NULL ),
    memHandler( memhandler )
{
}

/*!
  Destructor.
*/

dimeBlock::~dimeBlock()
{
  if (!this->memHandler) {
    for (int i = 0; i < this->entities.count(); i++) {
      delete this->entities[i];
    }
    delete this->endblock;
  }
}

//!

dimeEntity *
dimeBlock::copy(dimeModel * const model) const
{
  dimeMemHandler *memh = model->getMemHandler();
  dimeBlock *bl = new dimeBlock(memh);
  bool ok = true;

  int n = this->entities.count();
  if (n) {
    ok = dimeEntity::copyEntityArray((const dimeEntity**)
				     this->entities.constArrayPointer(),
				     n,
				     model,
				     bl->entities);
  }
  
  if (ok) {
    bl->basePoint = this->basePoint;
    bl->flags = this->flags;
    if (this->endblock)
      bl->endblock = this->endblock->copy(model);
    
    if (this->name) {
      bl->name = (char*)model->addBlock(this->name, bl);
      if (!bl->name) ok = false;
    }
  }
  
  if (!ok || !this->copyRecords(bl, model)) {
    if (!memh) delete bl; // delete if allocated on heap
    bl = NULL; // just return NULL
  }
  return bl;
}

/*!
  This method reads a BLOCK entity from \a file.
*/

bool
dimeBlock::read(dimeInput * const file)
{
  this->name = NULL;
  bool ret = dimeEntity::read(file);
  if (ret && this->name) {
    // see handleRecord() to understand this code. Yup, ugly :)
    char *tmp = (char*)this->name;
    this->name = file->getModel()->addBlock(tmp, this);
    delete [] tmp;
  }
  
  // got to do some reading to get all entities in the block
  if (ret) {
    dimeMemHandler *memhandler = file->getMemHandler();
    this->entities.makeEmpty(1024); // begin with a fairly large array
    ret = dimeEntity::readEntities(file, this->entities, "ENDBLK");
    if (ret) {
      this->endblock = dimeEntity::createEntity("ENDBLK", memhandler);
      // read the ENDBLOCK entity
      if (!this->endblock || !this->endblock->read(file)) ret = false;
    }
    this->entities.shrinkToFit(); // don't waste too much memory
  }

#ifndef NDEBUG
  dimeParam param;
  if (getRecord(67, param) && param.int16_data == 1) {
    fprintf(stderr,"paperspace block name: %s\n", 
	    ((dimeBlock*)this)->getName());
  }
#endif
  return ret;
}

/*!
  This methods writes a BLOCK entity to \a file.
*/

bool 
dimeBlock::write(dimeOutput * const file)
{
  this->preWrite(file);

  file->writeGroupCode(2);
  file->writeString(this->name);
  file->writeGroupCode(70);
  file->writeInt16(this->flags);
  
  file->writeGroupCode(10);
  file->writeDouble(this->basePoint[0]);
  file->writeGroupCode(20);
  file->writeDouble(this->basePoint[1]);
  file->writeGroupCode(30);
  file->writeDouble(this->basePoint[2]);

  // write unknown records.
  bool ret = dimeEntity::write(file);

  if (ret) {
    int i, n = this->entities.count();
    for (i = 0; i < n; i++) {
      if (!this->entities[i]->write(file)) break;
    }
    if (i == n) {
      if (this->endblock) {
	ret = this->endblock->write(file);
      }
      else { // just put a minimal ENDBLK there
	file->writeGroupCode(0);
	file->writeString("ENDBLK");
	file->writeGroupCode(8);
	ret = file->writeString(this->getLayerName());
      }
    }
    else ret = false;
  }
  return ret;
}

//!

int 
dimeBlock::typeId() const
{
  return dimeBase::dimeBlockType;
}

//!

bool 
dimeBlock::handleRecord(const int groupcode, 
		       const dimeParam &param,
		       dimeMemHandler * const memhandler)
{
  switch(groupcode) {
  case 2:
    {
      const char *str = param.string_data;
      if (str) {
	// this->name is used as a temporary storage space...
	// see read() to see what is done later.
	this->name = new char[strlen(str)+1];
	if (this->name) {
	  strcpy((char*)this->name, str);
	}
      }
      return true;
    }
  case 70:
    this->flags = param.int16_data;
    return true;
  case 10:
  case 20:
  case 30:
    this->basePoint[groupcode/10-1] = param.double_data;
    return true;
  }
  return dimeEntity::handleRecord(groupcode, param, memhandler);
}

//!

const char *
dimeBlock::getEntityName() const
{
  return entityName;
}

//!

bool 
dimeBlock::getRecord(const int groupcode,
		    dimeParam &param,
		    const int index) const
{
  switch(groupcode) {
  case 2:
    param.string_data = this->name;
    return true;
  case 70:
    param.int16_data = this->flags;
    return true;
  case 10:
  case 20:
  case 30:
    param.double_data = this->basePoint[groupcode/10-1];
    return true;
  }
  return dimeEntity::getRecord(groupcode, param, index);
}

//!

void 
dimeBlock::fixReferences(dimeModel * const model) 
{
  int i, n = this->entities.count();
  for (i = 0; i < n; i++) 
    this->entities[i]->fixReferences(model);
}

//!

int
dimeBlock::countRecords() const
{
  int cnt = 0;
  cnt += 3; // header
  cnt += 3; // basePoint
  
  int n = this->entities.count();
  for (int i = 0; i < n; i++)
    cnt += this->entities[i]->countRecords();
  
  return cnt + dimeEntity::countRecords();
}

/*!
  Inserts an entity in this block at position \a idx.
*/

void 
dimeBlock::insertEntity(dimeEntity * const entity, const int idx)
{
  if (idx < 0) this->entities.append(entity);
  else {
    assert(idx <= this->entities.count());
    this->entities.insertElem(idx, entity);
  }
}

/*!
  Removes the entity at position \a idx. If \a deleteIt is \e true, and 
  no memory handler is used, the entity will be deleted before 
  returing from this method.
*/

void 
dimeBlock::removeEntity(const int idx, const bool deleteIt)
{
  assert(idx >= 0 && idx < this->entities.count());
  if (!this->memHandler && deleteIt) delete this->entities[idx];
  this->entities.removeElem(idx);
}

//!

bool 
dimeBlock::traverse(const dimeState * const state, 
		   dimeCallback callback,
		   void *userdata)
{
  if (callback(state, this, userdata)) {
    //FIXME: what to do with basePoint?
    const int n = this->entities.count();
    for (int i = 0; i < n; i++) {
      if (!entities[i]->traverse(state, callback, userdata)) return false;
    }
  }
  if (this->endblock) 
    return callback(state, this->endblock, userdata);
  return true;
}

/*!
  Since a growable array is used to hold the entities, it might sometimes
  use more memory than absolutely needed. Call this method after you have 
  finished modifying a block if you want to free that overhead memory.
*/

void 
dimeBlock::fitEntities()
{
  this->entities.shrinkToFit();
}

