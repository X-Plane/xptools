/**************************************************************************\
 * 
 *  FILE: Solid.cpp
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
  \class dimeSolid dime/entities/Solid.h
  \brief The dimeSolid class handles a SOLID \e entity.
*/

#include <dime/entities/Solid.h>
#include <dime/records/Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>
#include <float.h>

static char entityName[] = "SOLID";

/*!
  Constructor.
*/

dimeSolid::dimeSolid() 
  : extrusionDir( 0, 0, 1 ), thickness( 0 )
{
}

//!

dimeEntity *
dimeSolid::copy(dimeModel * const model) const
{
  dimeSolid *f = new(model->getMemHandler())dimeSolid;
  if (!f) return NULL;
  
  f->copyCoords(this);
  f->thickness = this->thickness;
  f->extrusionDir = this->extrusionDir;
  
  if (!this->copyRecords(f, model)) {
    // check if allocated on heap.
    if (!model->getMemHandler()) delete f;
    f = NULL;
  }
  return f;
}

/*!
  Writes a SOLID entity.  
*/

bool 
dimeSolid::write(dimeOutput * const file)
{
  bool ret = true;
  if (!this->isDeleted()) {
    this->preWrite(file);
    this->writeCoords(file);
    if (this->thickness != 0.0) {
      file->writeGroupCode(39);
      file->writeDouble(this->thickness);
    }
    if (this->extrusionDir != dimeVec3f(0,0,1)) {
      file->writeGroupCode(210);
      file->writeDouble(this->extrusionDir[0]);
      file->writeGroupCode(220);
      file->writeDouble(this->extrusionDir[1]);
      file->writeGroupCode(230);
      file->writeDouble(this->extrusionDir[2]);

    }
    ret = dimeEntity::write(file);
  }
  return ret;
}

//!

bool 
dimeSolid::handleRecord(const int groupcode, 
		       const dimeParam &param,
		       dimeMemHandler * const memhandler)
{
  switch(groupcode) {
  case 210:
  case 220:
  case 230:
    this->extrusionDir[(groupcode-210)/10] = param.double_data;
    return true;
  case 39:
    this->thickness = param.double_data;
    return true;
  }
  return dimeFaceEntity::handleRecord(groupcode, param, memhandler);
}

//!

const char *
dimeSolid::getEntityName() const
{
  return entityName;
}

//!

bool 
dimeSolid::getRecord(const int groupcode,
		    dimeParam &param,
		    const int index) const
{
  switch(groupcode) {
  case 210:
  case 220:
  case 230:
    param.double_data = this->extrusionDir[(groupcode-210)/10];
    return true;
  case 39:
    param.double_data = this->thickness;
    return true;
  }
  return dimeFaceEntity::getRecord(groupcode, param, index);
}

//!

int 
dimeSolid::typeId() const
{
  return dimeBase::dimeSolidType;
}

//!

dxfdouble 
dimeSolid::getThickness() const
{
  return this->thickness;
}

//!

void 
dimeSolid::getExtrusionDir(dimeVec3f &ed) const
{
  ed = this->extrusionDir;
}

//!

bool 
dimeSolid::swapQuadCoords() const
{
  return true;
}

//!

void 
dimeSolid::setThickness(const dxfdouble &thickness)
{
  this->thickness = thickness;
}

//!

void 
dimeSolid::setExtrusionDir(const dimeVec3f &ed)
{
  this->extrusionDir = ed;
}

//!

int 
dimeSolid::countRecords() const
{
  int cnt = 0;
  if (!this->isDeleted()) {
    cnt++; // header
    if (this->thickness != 0.0) cnt++;
    if (this->extrusionDir != dimeVec3f(0,0,1)) cnt += 3; 
    cnt += dimeFaceEntity::countRecords();
  }
  return cnt;
}

