/**************************************************************************\
 * 
 *  FILE: 3DFace.cpp
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
  \class dime3DFace dime/entities/3DFace.h
  \brief The dime3DFace class handles a 3DFACE \e entity.
*/

#include <dime/entities/3DFace.h>
#include <dime/records/Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>

static char entityName[] = "3DFACE";

/*!
  Constructor.
*/

dime3DFace::dime3DFace()
  : flags( 0 )
{
#ifndef NO_RR_DATA
  this->block = NULL;
#endif
}

//!

dimeEntity *
dime3DFace::copy(dimeModel * const model) const
{
  dime3DFace *f = new(model->getMemHandler()) dime3DFace;
  if (!f) return NULL;
  
  f->copyCoords(this);
  f->flags = this->flags;
  
  if (!this->copyRecords(f, model)) {
    // check if allocated on heap.
    if (!model->getMemHandler()) delete f;
    f = NULL;
  }
  return f;
}

//!

const char *
dime3DFace::getEntityName() const
{
  return entityName;
}

//!

bool 
dime3DFace::write(dimeOutput * const file)
{
  bool ret = true;
  if (!this->isDeleted()) {
    this->preWrite(file);
    this->writeCoords(file);
    if (flags != 0) {
      file->writeGroupCode(70);
      file->writeInt16(flags);
    }
    ret = dimeEntity::write(file);
  }
  return ret;
}

//!

int 
dime3DFace::typeId() const
{
  return dimeBase::dime3DFaceType;
}

//!

bool 
dime3DFace::handleRecord(const int groupcode, 
			const dimeParam &param,
			dimeMemHandler * const memhandler)
{
  if (groupcode == 70) {
    this->flags = param.int16_data;
    return true;
  }
  else {
    return dimeFaceEntity::handleRecord(groupcode, param, memhandler);
  }
}

//!

bool 
dime3DFace::getRecord(const int groupcode,
		     dimeParam &param,
		     const int index) const
{
  if (groupcode == 70) {
    param.int16_data = this->flags;
    return true;
  }
  return dimeFaceEntity::getRecord(groupcode, param, index);
}

//!

void
dime3DFace::print() const
{
  fprintf(stderr,"3DFACE:\n");
  int stop = this->isQuad() ? 4 : 3;
  for (int i = 0; i < stop; i++) { 
    fprintf(stderr,"coord: %f %f %f\n", coords[i][0], 
	    coords[i][1], coords[i][2]); 
    
  } 
}

//!

int
dime3DFace::countRecords() const
{
  int cnt = 0;
  if (!this->isDeleted()) {
    cnt++; // header
    if (this->flags != 0) cnt++;
    cnt += dimeFaceEntity::countRecords();
  }
  return cnt;   
}

void 
dime3DFace::setFlags(const int16 flags)
{
  this->flags = flags;
}

int16 
dime3DFace::getFlags() const
{
  return this->flags;
}

