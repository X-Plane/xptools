/**************************************************************************\
 * 
 *  FILE: Line.cpp
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
  \class dimeLine dime/entities/Line.h
  \brief The dimeLine class handles a LINE \e entity.
*/

#include <dime/entities/Line.h>
#include <dime/records/Record.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>

static char entityName[] = "LINE";

/*!
  Constructor.
*/

dimeLine::dimeLine() 
{
  coords[0].setValue(0,0,0);
  coords[1].setValue(0,0,0);
}

//!

dimeEntity *
dimeLine::copy(dimeModel * const model) const
{
  dimeLine *l = new(model->getMemHandler()) dimeLine;
  if (!l) return NULL;

  for (int i = 0; i < 2; i++)
    l->coords[i] = this->coords[i];
  
  if (!this->copyRecords(l, model)) {
    // check if allocated on heap.
    if (!model->getMemHandler()) delete l;
    l = NULL;
  }
  else {
    l->copyExtrusionData(this);
  }
  return l;  
}

/*!
  Writes a \e Line entity.  
*/

bool 
dimeLine::write(dimeOutput * const file)
{
  this->preWrite(file);

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 3; j++) {
      file->writeGroupCode((j+1)*10+i);
      file->writeDouble(this->coords[i][j]);
    }
  }
  return this->writeExtrusionData(file) && 
    dimeEntity::write(file);
}

//!

int 
dimeLine::typeId() const
{
  return dimeBase::dimeLineType;
}

/*!
  Handles the callback from dimeEntity::readRecords().
*/

bool 
dimeLine::handleRecord(const int groupcode,
		      const dimeParam &param,
		      dimeMemHandler * const memhandler)
{
  switch(groupcode) {
  case 10:
  case 20:
  case 30:
  case 11:
  case 21:
  case 31:
    this->coords[groupcode % 10][groupcode / 10 - 1] = param.double_data;
    return true;
  }
  return dimeExtrusionEntity::handleRecord(groupcode, param, memhandler);
}

//!

const char *
dimeLine::getEntityName() const
{
  return entityName;
}

//!

bool 
dimeLine::getRecord(const int groupcode,
		   dimeParam &param,
		   const int index) const
{
  switch(groupcode) {
  case 10:
  case 20:
  case 30:
  case 11:
  case 21:
  case 31:
    param.double_data = this->coords[groupcode % 10][groupcode / 10 - 1];
    return true;
  }
  return dimeExtrusionEntity::getRecord(groupcode, param, index);
}

//!

void
dimeLine::print() const
{
  fprintf(stderr,"LINE:\n");
  for (int i = 0; i < 2; i++) { 
    fprintf(stderr,"coord: %f %f %f\n", coords[i][0], 
	    coords[i][1], coords[i][2]); 
    
  } 
}

//!

dimeEntity::GeometryType 
dimeLine::extractGeometry(dimeArray <dimeVec3f> &verts,
			 dimeArray <int> &/*indices*/,
			 dimeVec3f &extrusionDir,
			 dxfdouble &thickness)
{
  thickness = this->thickness;
  extrusionDir = this->extrusionDir;

  verts.append(coords[0]);
  verts.append(coords[1]);
  return dimeEntity::LINES;
}

//!

int
dimeLine::countRecords() const
{
  int cnt = 1; // header
  cnt += 6; // coordinates
  return cnt + dimeExtrusionEntity::countRecords();
}

