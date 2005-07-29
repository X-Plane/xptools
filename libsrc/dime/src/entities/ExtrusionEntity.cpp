/**************************************************************************\
 * 
 *  FILE: ExtrusionEntity.cpp
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
  \class dimeExtrusionEntity dime/entities/ExtrusionEntity.h
  \brief The dimeExtrusionEntity class is the superclass of all \e entity classes with extrusion data.
*/

#include <dime/entities/ExtrusionEntity.h>
#include <dime/Output.h>

/*!
  \fn void dimeExtrusionEntity::setExtrusionDir(const dimeVec3f &v)
  Sets the extrusion direction. Default value is \e (0,0,1).
*/

/*!
  \fn const dimeVec3f &dimeExtrusionEntity::getExtrusionDir() const
  Returns the extrusion direction.
*/

/*! 
  \fn void dimeExtrusionEntity::setThickness(const dxfdouble val)
  Sets the extrusion thickness. Default value is \e 0.0.
*/

/*!
  \fn dxfdouble dimeExtrusionEntity::getThickness() const
  Returns the extrusion thickness.
*/

/*!
  Constructor. Will initialize the extrusion direction to \e (0,0,1) and 
  the thickness to \e 0.0.
*/

dimeExtrusionEntity::dimeExtrusionEntity()
  : extrusionDir(0,0,1), thickness( 0.0 )
{
}

/*!
  Will write the extrusion and thickness records.
*/

bool 
dimeExtrusionEntity::writeExtrusionData(dimeOutput * const file)
{
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
  return true;
}

//!

int 
dimeExtrusionEntity::typeId() const
{
  return dimeBase::dimeExtrusionEntityType;
}

//!

bool 
dimeExtrusionEntity::isOfType(const int thetypeid) const
{
  return thetypeid == dimeExtrusionEntityType ||
    dimeEntity::isOfType(thetypeid);
}

//!

int 
dimeExtrusionEntity::countRecords() const
{
  int cnt = 0;
  if (this->thickness != 0.0) cnt++;
  if (this->extrusionDir != dimeVec3f(0,0,1)) cnt+=3;
  return cnt + dimeEntity::countRecords();
}

/*!
  Copies all extrusion data from \a entity.
*/

void 
dimeExtrusionEntity::copyExtrusionData(const dimeExtrusionEntity * const entity)
{
  this->extrusionDir = entity->extrusionDir;
  this->thickness = entity->thickness;
}

//!

bool 
dimeExtrusionEntity::handleRecord(const int groupcode,
				 const dimeParam &param,
				 dimeMemHandler * const memhandler)
{
  switch(groupcode) {
  case 39:
    this->thickness = param.double_data;
    return true;
  case 210:
  case 220:
  case 230:
    this->extrusionDir[(groupcode-210)/10] = param.double_data;
    return true;
  }
  return dimeEntity::handleRecord(groupcode, param, memhandler);
}

//!

bool 
dimeExtrusionEntity::getRecord(const int groupcode,
			      dimeParam &param,
			      const int index) const
{
  switch(groupcode) {
  case 39:
    param.double_data = this->thickness; 
    return true;
  case 210:
  case 220:
  case 230:
    param.double_data = this->extrusionDir[(groupcode-210)/10]; 
    return true;
  }
  return dimeEntity::getRecord(groupcode, param, index);
}

