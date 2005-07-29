/**************************************************************************\
 * 
 *  FILE: Arc.h
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

#ifndef DIME_ARC_H
#define DIME_ARC_H
  
#include <dime/Basic.h>
#include <dime/entities/ExtrusionEntity.h>
#include <dime/util/Linear.h>

class DIME_DLL_API dimeArc : public dimeExtrusionEntity
{
public:
  dimeArc();

  void setCenter(const dimeVec3f &c);
  void getCenter(dimeVec3f &c) const;
  void setRadius(const dxfdouble r);
  dxfdouble getRadius() const;
  void setStartAngle(const dxfdouble a);
  dxfdouble getStartAngle() const;
  void setEndAngle(const dxfdouble a);
  dxfdouble getEndAngle() const;
  
  virtual bool getRecord(const int groupcode,
			 dimeParam &param,
			 const int index = 0) const;
  virtual const char *getEntityName() const;

  virtual dimeEntity *copy(dimeModel * const model) const;
  
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
  dxfdouble startAngle;
  dxfdouble endAngle;

}; // class dimeArc

//
// inline methods
//

inline void 
dimeArc::setCenter(const dimeVec3f &c)
{
  this->center = c;
}

inline void 
dimeArc::getCenter(dimeVec3f &c) const
{
  c = this->center;
}

inline void 
dimeArc::setRadius(const dxfdouble r)
{
  this->radius = r;
}

inline dxfdouble 
dimeArc::getRadius() const
{
  return this->radius;
}

inline void 
dimeArc::setStartAngle(const dxfdouble a)
{
  this->startAngle = a;
}

inline dxfdouble 
dimeArc::getStartAngle() const
{
  return this->startAngle;
}

inline void 
dimeArc::setEndAngle(const dxfdouble a)
{
  this->endAngle = a;
}

inline dxfdouble 
dimeArc::getEndAngle() const
{
  return this->endAngle;
}
 
#endif // ! DIME_ARC_H

