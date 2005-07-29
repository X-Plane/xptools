/**************************************************************************\
 * 
 *  FILE: Insert.cpp
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
  \class dimeInsert dime/entities/Insert.h
  \brief The dimeInsert class handles an INSERT \e entity.
*/

#include <dime/entities/Insert.h>
#include <dime/entities/Block.h>
#include <dime/records/Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>
#include <dime/State.h>

static char entityName[] = "INSERT";

/*!
  Constructor.
*/

dimeInsert::dimeInsert()
{
  this->attributesFollow = 0;
  this->blockName = NULL;
  this->insertionPoint.setValue(0, 0, 0);
  this->extrusionDir.setValue(0,0,1);
  this->scale.setValue(1, 1, 1);
  this->rotAngle = 0.0;
  this->entities = NULL;
  this->numEntities = 0;
  this->seqend = NULL;
  this->block = NULL;
  this->rowCount = 1;
  this->columnCount = 1;
  this->rowSpacing = 0.0;
  this->columnSpacing = 0.0;
}

/*!
  Destructor.
*/

dimeInsert::~dimeInsert()
{
  delete this->seqend;
  for (int i = 0; i < this->numEntities; i++)
    delete this->entities[i];
  delete [] this->entities;
}

//!

dimeEntity *
dimeInsert::copy(dimeModel * const model) const
{
  dimeMemHandler *memh = model->getMemHandler();
  dimeInsert *inst = new(memh) dimeInsert;

  bool ok = true;
  if (this->numEntities) {
    int realnum = this->numEntities;
    inst->entities = dimeEntity::copyEntityArray((const dimeEntity**)
						 this->entities,
						 realnum,
						 model);
    if (realnum > 0 && inst->entities == NULL) {
      ok = false;
    }
    else inst->numEntities = realnum;
  }
  
  inst->attributesFollow = this->attributesFollow;
  inst->insertionPoint = this->insertionPoint;
  inst->scale = this->scale;
  inst->rotAngle = this->rotAngle;
  inst->rowCount = this->rowCount;
  inst->columnCount = this->columnCount;
  inst->rowSpacing = this->rowSpacing;
  inst->columnSpacing = this->columnSpacing;
  inst->extrusionDir = this->extrusionDir;

  if (ok && this->seqend) {
    inst->seqend = this->seqend->copy(model);
    if (!inst->seqend) ok = false;
  }

  if (ok && this->blockName) {
    inst->blockName = model->findRefStringPtr(this->blockName);
    if (inst->blockName) {
      inst->block = (dimeBlock*) model->findReference(inst->blockName);
    }
    else {
      // probably a forward reference, fixed during fixReferences()
      inst->blockName = (char*)model->addReference(this->blockName, NULL);
      inst->block = NULL;
    }
  }

  if (!ok || !this->copyRecords(inst, model)) {
    if (!memh) delete inst; // delete if allocated on heap
    inst = NULL; // just return NULL
  }
  return inst;
}

/*!
  Reads an INSERT entity.
*/

bool
dimeInsert::read(dimeInput * const file)
{
  // see handleRecord() to understand what is done with
  // blockName here... Ugly code, but who cares :-)
  this->blockName = NULL;
  bool ret = dimeEntity::read(file);
  if (ret && this->blockName) {
    char *tmp = (char*)this->blockName;
    this->blockName = file->getModel()->findRefStringPtr(tmp);
    if (this->blockName) {
      this->block = (dimeBlock*)file->getModel()->findReference(tmp);
    }
    else {
      // probably a forward reference, just add as reference
      this->blockName = file->getModel()->addReference(tmp, NULL);
    }
    delete [] tmp;
  }

  if (ret && this->attributesFollow) {
    dimeMemHandler *memhandler = file->getMemHandler();
    // read following entities.
    dimeArray <dimeEntity*> array;
    ret = dimeEntity::readEntities(file, array, "SEQEND");
    if (ret) {
      this->seqend = dimeEntity::createEntity("SEQEND", memhandler);
      // read the SEQEND entity
      if (!this->seqend || !this->seqend->read(file)) ret = false;
    }
    int n = array.count();
    if (ret && n) {
      this->entities = ARRAY_NEW(memhandler, dimeEntity*, n);
      if (this->entities) {
	this->numEntities = n;
	for (int i = 0; i < n; i++) {
	  this->entities[i] = array[i];
	}
      }
      else ret = false;
    }
  }  
  return ret;
}

/*!
  Writes an INSERT entity.  
*/

bool 
dimeInsert::write(dimeOutput * const file)
{
  this->preWrite(file);
  
  if (this->attributesFollow) {
    file->writeGroupCode(66);
    file->writeInt16(1);
  }
  file->writeGroupCode(2);
  file->writeString(this->blockName);

  file->writeGroupCode(10);
  file->writeDouble(insertionPoint[0]);
  file->writeGroupCode(20);
  file->writeDouble(insertionPoint[1]);
  file->writeGroupCode(30);
  file->writeDouble(insertionPoint[2]);

  if (this->scale != dimeVec3f(1, 1, 1)) {
    file->writeGroupCode(41);
    file->writeDouble(this->scale[0]);
    file->writeGroupCode(42);
    file->writeDouble(this->scale[1]);
    file->writeGroupCode(43);
    file->writeDouble(this->scale[2]);
  }
  if (this->rotAngle != 0.0) {
    file->writeGroupCode(50);
    file->writeDouble(this->rotAngle);
  }

  if (this->columnCount != 1) {
    file->writeGroupCode(70);
    file->writeInt16(this->columnCount);
  }
  if (this->rowCount != 1) {
    file->writeGroupCode(71);
#ifdef DIME_FIXBIG
    file->writeInt32(this->rowCount);
#else
    file->writeInt16(this->rowCount);
#endif
  }
  if (this->columnSpacing != 0.0) {
    file->writeGroupCode(44);
    file->writeDouble(this->columnSpacing);
  }
  if (this->rowSpacing != 0.0) {
    file->writeGroupCode(45);
    file->writeDouble(this->rowSpacing);
  }

  bool ret = dimeEntity::write(file); // write unknown records

  if (this->extrusionDir != dimeVec3f(0,0,1)) {
    file->writeGroupCode(210);
    file->writeDouble(this->extrusionDir[0]);
    file->writeGroupCode(220);
    file->writeDouble(this->extrusionDir[1]);
    file->writeGroupCode(230);
    ret = file->writeDouble(this->extrusionDir[2]);
  }
  
  if (this->attributesFollow && this->numEntities) {
    int i;
    for (i = 0; i < this->numEntities; i++) {
      if (!this->entities[i]->write(file)) break;
    }
    if (this->seqend) ret = this->seqend->write(file);
    else {
      file->writeGroupCode(0);
      ret = file->writeString("SEQEND");
    }
  }
  return ret;
}

//!

int 
dimeInsert::typeId() const
{
  return dimeBase::dimeInsertType;
}

//!

bool 
dimeInsert::handleRecord(const int groupcode,
			const dimeParam &param,
			dimeMemHandler * const memhandler)
{
  switch (groupcode) {
  case 66: 
    this->attributesFollow = param.int16_data;
    return true;
  case 2: 
    {
      // will only arrive here during read(). Allocate a temporary buffer
      // to store the blockname. Will be deleted in dimeInsert::read() 
      const char *str = param.string_data;
      if (str) {
	this->blockName = new char[strlen(str)+1];
	if (this->blockName) {
	  strcpy((char*)this->blockName, str);
	}
      }
      return true;
    }
  case 10:
  case 20:
  case 30:
    this->insertionPoint[groupcode/10-1] = param.double_data;
    return true;
  case 210:
  case 220:
  case 230:
    this->extrusionDir[(groupcode-210)/10] = param.double_data;
    return true;
  case 41:
  case 42:
  case 43:
    this->scale[groupcode-41] = param.double_data;
    return true;
  case 44:
    this->columnSpacing = param.double_data;
    return true;
  case 45:
    this->rowSpacing = param.double_data;
    return true;
  case 50:
    this->rotAngle = param.double_data;
    return true;
  case 70:
    this->columnCount = param.int16_data;
    return true;
  case 71:
#ifdef DIME_FIXBIG
    this->rowCount = param.int32_data;
#else
    this->rowCount = param.int16_data;
#endif
    return true;
  }
  return dimeEntity::handleRecord(groupcode, param, memhandler);
}

//!

const char *
dimeInsert::getEntityName() const
{
  return entityName;
}

//!

bool 
dimeInsert::getRecord(const int groupcode,
		     dimeParam &param,
		     const int index) const
{
  switch (groupcode) {
  case 66: 
    param.int16_data = this->attributesFollow;
    return true;
  case 2: 
    param.string_data = this->blockName;
    return true;
  case 10:
  case 20:
  case 30:
    param.double_data = this->insertionPoint[groupcode/10-1];
    return true;
  case 210:
  case 220:
  case 230:
    param.double_data = this->extrusionDir[(groupcode-210)/10];
    return true;
  case 41:
  case 42:
  case 43:
    param.double_data = this->scale[groupcode-41];
    return true;
  case 44:
    param.double_data = this->columnSpacing;
    return true;
  case 45:
    param.double_data = this->rowSpacing;
    return true;
  case 50:
    param.double_data = this->rotAngle;
    return true;
  case 70:
    param.int16_data = this->columnCount;
    return true;
  case 71:
#ifdef DIME_FIXBIG
    param.int32_data = this->rowCount;
#else
    param.int16_data = this->rowCount;
#endif
    return true;
  }
  return dimeEntity::getRecord(groupcode, param, index);
}

//!

bool 
dimeInsert::traverse(const dimeState * const state, 
		    dimeCallback callback,
		    void *userdata)
{
  dimeState newstate = *state;
  newstate.currentInsert = this;
  
  if (this->block && (state->getFlags() & dimeState::EXPLODE_INSERTS)) {
    for (int i = 0; i < this->rowCount; i++) {
      for (int j = 0; j < this->columnCount; j++) {
	dimeMatrix m = state->getMatrix();
	dimeMatrix m2 = dimeMatrix::identity();
	m2.setTranslate(dimeVec3f(j*this->columnSpacing, 
				  i*this->rowSpacing, 
				  0));
	m.multRight(m2);
	this->makeMatrix(m);
	newstate.setMatrix(m);
	if (!block->traverse(&newstate, callback, userdata)) return false;
      }
    }
  }
  else if (!this->isDeleted()) {
    if (!callback(state, this, userdata)) return false;
  }

  dimeMatrix m = state->getMatrix();
  this->makeMatrix(m);
  newstate.setMatrix(m);
  
  // extract internal INSERT entities
  for (int i = 0; i < this->numEntities; i++) {
    if (!this->entities[i]->traverse(&newstate, callback, userdata)) return false;
  }
  return true;
}

//!

void 
dimeInsert::fixReferences(dimeModel * const model) 
{
  if (this->block == NULL && this->blockName) {
    this->block = (dimeBlock*)model->findReference(this->blockName);
    if (this->block == NULL) {
      fprintf(stderr,"BLOCK %s not found!\n", blockName);
    }
  }
  for (int i = 0; i < this->numEntities; i++)
    this->entities[i]->fixReferences(model);
}

//!

void 
dimeInsert::makeMatrix(dimeMatrix &m) const
{
  if (!this->block) {
    m.makeIdentity();
    return;
  }
  dimeMatrix m2;

  if (this->extrusionDir != dimeVec3f(0,0,1)) {
    // this block has its own coordinate system
    // generated from one vector (the z-vector)
    dimeEntity::generateUCS(this->extrusionDir, m2);
    m.multRight(m2);
  }  

  m2.makeIdentity();
  dimeVec3f tmp = this->insertionPoint;

  // disabled for the moment
  // dimeModel::fixDxfCoords(tmp);

  m2.setTranslate(tmp);
  m.multRight(m2);

  m2.setTransform(dimeVec3f(0,0,0),
		  this->scale,
		  dimeVec3f(0, 0, this->rotAngle));
  m.multRight(m2); 
  
  m2.makeIdentity();
  m2.setTranslate(-block->getBasePoint());
  m.multRight(m2);
}

//!

int
dimeInsert::countRecords() const
{
  int cnt = 5; // header + blockName + insertionPoint

  if (this->attributesFollow) cnt++;
  
  if (this->scale != dimeVec3f(1.0, 1.0, 1.0)) cnt += 3;
  if (this->rotAngle != 0.0) cnt++;
  if (this->columnCount != 1) cnt++;
  if (this->rowCount != 1) cnt++;
  if (this->columnSpacing != 0.0) cnt++;
  if (this->rowSpacing != 0.0) cnt++;
  if (this->extrusionDir != dimeVec3f(0,0,1)) cnt += 3;
  
  if (this->attributesFollow && this->numEntities) {
    int i, n = this->numEntities;
    for (i = 0; i < n; i++) {
      cnt += this->entities[i]->countRecords();
    }
    cnt++; // seqend
  }
  return cnt + dimeEntity::countRecords();
}

/*!
  Sets the block for this INSERT entity. This will change the record
  with group code 2.
*/

void 
dimeInsert::setBlock(dimeBlock * const block)
{
  this->block = block;
  this->blockName = block->getName();
}

