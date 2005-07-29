/**************************************************************************\
 * 
 *  FILE: Vertex.cpp
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
  \class dimeVertex dime/entities/Vertex.h
  \brief The dimeVertex class handles a VERTEX \e entity.
*/

#include <dime/entities/Vertex.h>
#include <dime/records/Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>

static char entityName[] = "VERTEX";

/*!
  Constructor.
*/

dimeVertex::dimeVertex()
{
  this->polyline = NULL;
  this->flags = 0;
  this->indices[0] = 0;
  this->indices[1] = 0;
  this->indices[2] = 0;
  this->indices[3] = 0;
  this->coords.setValue(0.0f, 0.0f, 0.0f);
}

//!

dimeEntity *
dimeVertex::copy(dimeModel * const model) const
{
  dimeVertex *v = new(model->getMemHandler()) dimeVertex;
  
  v->flags = this->flags;
  v->indices[0] = this->indices[0];
  v->indices[1] = this->indices[1];
  v->indices[2] = this->indices[2];
  v->indices[3] = this->indices[3];
  v->coords = this->coords;
  v->polyline = this->polyline;

  if (!this->copyRecords(v, model)) {
    // check if allocated on heap.
    if (!model->getMemHandler()) delete v;
    v = NULL;
  }
  return v;
}

//!

bool 
dimeVertex::write(dimeOutput * const file)
{
  bool ret = true;
  if (!this->isDeleted()) {
    this->preWrite(file);

    file->writeGroupCode(70);
    file->writeInt16(this->flags);
    file->writeGroupCode(10);
    file->writeDouble(this->coords[0]);
    file->writeGroupCode(20);
    file->writeDouble(this->coords[1]);
    file->writeGroupCode(30);
    file->writeDouble(this->coords[2]);

    for (int i = 0; i < this->numIndices(); i++) {
      file->writeGroupCode(i+71);
#ifdef DIME_FIXBIG
      file->writeInt32(this->indices[i]);
#else
      file->writeInt16(this->indices[i]);
#endif
    }
    ret = dimeEntity::write(file);
  }
  return ret;
}

/*!
  Returns the number of indices stored in this vertex;
*/

int 
dimeVertex::numIndices() const
{
  int cnt = 0;
  if ((flags & 128) && !(flags & 64)) { // does vertex store indices?
    for (int i = 0; this->indices[i] && i < 4; i++, cnt++);
  }
  return cnt;
}

/*!
  Returns index number \a num;
*/

int 
dimeVertex::getIndex(const int num) const
{
  return this->indices[num];
}

//!

int 
dimeVertex::typeId() const
{
  return dimeBase::dimeVertexType;
}

//!

bool 
dimeVertex::handleRecord(const int groupcode,
			const dimeParam &param,
			dimeMemHandler * const memhandler)
{
  switch(groupcode) {
  case 70:
    this->flags = param.int16_data;
    return true;
  case 10:
  case 20:
  case 30:
    this->coords[groupcode/10-1] = param.double_data;
    return true;
  case 71:
  case 72:
  case 73:
  case 74:
#ifdef DIME_FIXBIG
    this->indices[groupcode-71] = param.int32_data;
#else
    this->indices[groupcode-71] = param.int16_data;
#endif
    return true;
  }
  return dimeEntity::handleRecord(groupcode, param, memhandler);
}

//!

const char *
dimeVertex::getEntityName() const
{
  return entityName;
}

//!

bool 
dimeVertex::getRecord(const int groupcode,
		     dimeParam &param,
		     const int index) const
{
  switch(groupcode) {
  case 70:
    param.int16_data = this->flags;
    return true;
  case 10:
  case 20:
  case 30:
    param.double_data = this->coords[groupcode/10-1];
    return true;
  case 71:
  case 72:
  case 73:
  case 74:
#ifdef DIME_FIXBIG
    param.int16_data = this->indices[groupcode-71];
#else
    param.int32_data = this->indices[groupcode-71];
#endif
    return true;
  }
  return dimeEntity::getRecord(groupcode, param, index);
}

//!

int
dimeVertex::countRecords() const
{
  int cnt = 0;
  if (!this->isDeleted()) {
    cnt += 5; // header + flags + coords
    cnt += this->numIndices();
    cnt += dimeEntity::countRecords();
  }  
  return cnt;  
}

