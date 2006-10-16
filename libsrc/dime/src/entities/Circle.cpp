/**************************************************************************\
 * 
 *  FILE: Circle.cpp
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
  \class dimeCircle dime/entities/Circle.h
  \brief The dimeCircle class handles a CIRCLE \e entity.
*/

#include <dime/entities/Circle.h>
#include <dime/records/Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>
#include <math.h>

#ifdef _WIN32
#define M_PI 3.14159265357989
#endif

#ifdef macintosh
#define M_PI 3.14159265357989
#endif

static char entityName[] = "CIRCLE";

// FIXME: configurable

#define CIRCLE_NUMPTS 16

/*!
  Constructor.
*/

dimeCircle::dimeCircle() 
  : center(0,0,0), radius( 0.0 )
{
}

//!

dimeEntity *
dimeCircle::copy(dimeModel * const model) const
{
  dimeCircle *c = new(model->getMemHandler()) dimeCircle;
  if (!c) return NULL;
  
  if (!this->copyRecords(c, model)) {
    // check if allocated on heap.
    if (!model->getMemHandler()) delete c;
    c = NULL;
  }
  else {
    c->copyExtrusionData(this);
    c->center = this->center;
    c->radius = this->radius;
  }
  return c;  
}

/*!
  This method writes a DXF \e Circle entity to \a file.
*/

bool 
dimeCircle::write(dimeOutput * const file)
{
  dimeEntity::preWrite(file);
  
  file->writeGroupCode(10);
  file->writeDouble(this->center[0]);
  file->writeGroupCode(20);
  file->writeDouble(this->center[1]);
  file->writeGroupCode(30);
  file->writeDouble(this->center[2]);
  
  file->writeGroupCode(40);
  file->writeDouble(this->radius);

  this->writeExtrusionData(file);
  
  return dimeEntity::write(file); // write unknown records.
}

//!

int 
dimeCircle::typeId() const
{
  return dimeBase::dimeCircleType;
}

/*!
  Handles the callback from dimeEntity::readRecords().
*/

bool 
dimeCircle::handleRecord(const int groupcode,
			const dimeParam &param,
			dimeMemHandler * const memhandler)
{
  switch(groupcode) {
  case 10:
  case 20:
  case 30:
    this->center[groupcode/10-1] = param.double_data;
    return true;
  case 40:
    this->radius = param.double_data;
    return true;
  }
  return dimeExtrusionEntity::handleRecord(groupcode, param, memhandler);
}

//!

const char *
dimeCircle::getEntityName() const
{
  return entityName;
}

//!

bool 
dimeCircle::getRecord(const int groupcode,
		     dimeParam &param,
		     const int index) const
{
  switch(groupcode) {
  case 10:
  case 20:
  case 30:
    param.double_data = this->center[groupcode/10-1]; 
    return true;
  case 40:
    param.double_data = this->radius;
    return true;
  }
  return dimeExtrusionEntity::getRecord(groupcode, param, index);  
}

//!

void
dimeCircle::print() const
{
  fprintf(stderr,"CIRCLE:\n");
  fprintf(stderr, " center: %.3f %.3f %.3f\n", center[0], center[1], center[2]);
  fprintf(stderr, " radius: %f\n", radius);
}

//!

dimeEntity::GeometryType 
dimeCircle::extractGeometry(dimeArray <dimeVec3f> &verts,
			   dimeArray <int> &/*indices*/,
			   dimeVec3f &extrusionDir,
			   dxfdouble &thickness)
{
  thickness = this->thickness;
  extrusionDir = this->extrusionDir;

   // tessellate the circle/cylinder
  dxfdouble inc = (2*M_PI) / CIRCLE_NUMPTS;
  dxfdouble rad = 0.0f;
  int i;
  for (i = 0; i < CIRCLE_NUMPTS; i++) {
    verts.append(dimeVec3f(this->center.x + this->radius * cos(rad),
			   this->center.y + this->radius * sin(rad),
			   this->center.z));
    rad += inc;
  }
  
  dimeVec3f tmp = verts[0];
  verts.append(tmp); // closed line/polygon
  
  if (this->thickness == 0.0) return dimeEntity::LINES;
  else return dimeEntity::POLYGONS;
}

//!

int
dimeCircle::countRecords() const
{
  // header + center point + radius
  return 5 + dimeExtrusionEntity::countRecords();
}

