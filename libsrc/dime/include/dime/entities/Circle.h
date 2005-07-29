/**************************************************************************\
 * 
 *  FILE: Circle.h
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

#ifndef DIME_CIRCLE_H
#define DIME_CIRCLE_H

#include <dime/Basic.h>
#include <dime/entities/ExtrusionEntity.h>
#include <dime/util/Linear.h>

class dimeModel;
class dimeMemHandler;

class DIME_DLL_API dimeCircle : public dimeExtrusionEntity
{
public:
  dimeCircle();

  const dimeVec3f &getCenter() const;
  void setCenter(const dimeVec3f &c);

  void setRadius(const dxfdouble val);
  dxfdouble getRadius() const;

  virtual dimeEntity *copy(dimeModel * const model) const;
  virtual bool getRecord(const int groupcode,
			 dimeParam &param,
			 const int index = 0) const;
  virtual const char *getEntityName() const;
  virtual void print() const;
  virtual bool write(dimeOutput * const out);
  virtual int typeId() const;
  virtual int countRecords() const;

  virtual GeometryType extractGeometry(dimeArray <dimeVec3f> &verts,
				       dimeArray <int> &indices,
				       dimeVec3f &extrusionDir,
				       dxfdouble &thickness);

protected:  
  virtual bool handleRecord(const int groupcode,
			    const dimeParam &param,
			    dimeMemHandler * const memhandler);  
private:
  dimeVec3f center;
  dxfdouble radius;

}; // class dimeCircle

inline const dimeVec3f &
dimeCircle::getCenter() const
{
  return this->center;
}

inline void 
dimeCircle::setCenter(const dimeVec3f &c)
{
  this->center = c;
}

inline void 
dimeCircle::setRadius(const dxfdouble val)
{
  this->radius = val;
}

inline dxfdouble 
dimeCircle::getRadius() const
{
  return this->radius;
}

#endif // ! DIME_CIRCLE_H

