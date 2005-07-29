/**************************************************************************\
 * 
 *  FILE: Ellipse.h
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

#ifndef DIME_ELLIPSE_H
#define DIME_ELLIPSE_H

#include <dime/Basic.h>
#include <dime/entities/ExtrusionEntity.h>
#include <dime/util/Linear.h>

//
// please note that the thickness will always be 0.0 for this entity
//

class DIME_DLL_API dimeEllipse : public dimeExtrusionEntity
{
public:
  dimeEllipse();

  void setCenter(const dimeVec3f &c);
  const dimeVec3f &getCenter() const;

  void setMajorAxisEndpoint(const dimeVec3f &v);
  const dimeVec3f &getMajorAxisEndpoint() const;
  
  void setMinorMajorRatio(const dxfdouble ratio);
  dxfdouble getMinorMajorRatio() const;

  void setStartParam(const dxfdouble p);
  dxfdouble getStartParam() const;

  void setEndParam(const dxfdouble p);
  dxfdouble getEndParam() const;
  
  virtual dimeEntity *copy(dimeModel * const model) const;
  virtual bool getRecord(const int groupcode,
			 dimeParam &param,
			 const int index = 0) const;
  virtual const char *getEntityName() const;
  virtual void print() const;
  virtual bool write(dimeOutput * const out);
  virtual int typeId() const;
  virtual int countRecords() const;

protected:  
  virtual bool handleRecord(const int groupcode,
			    const dimeParam &param,
			    dimeMemHandler * const memhandler);  
private:
  dimeVec3f center;
  dimeVec3f majorAxisEndpoint;
  dxfdouble ratio;
  dxfdouble startParam;
  dxfdouble endParam;

}; // class dimeEllipse

inline const dimeVec3f &
dimeEllipse::getCenter() const
{
  return this->center;
}

inline void 
dimeEllipse::setCenter(const dimeVec3f &c)
{
  this->center = c;
}

inline void
dimeEllipse::setMajorAxisEndpoint(const dimeVec3f &v)
{
  this->majorAxisEndpoint = v;
}

inline const dimeVec3f &
dimeEllipse::getMajorAxisEndpoint() const
{
  return this->majorAxisEndpoint;
}
  
inline void 
dimeEllipse::setMinorMajorRatio(const dxfdouble ratio)
{
  this->ratio = ratio;
}

inline dxfdouble 
dimeEllipse::getMinorMajorRatio() const
{
  return this->ratio;
}

inline void 
dimeEllipse::setStartParam(const dxfdouble p)
{
  this->startParam = p;
}

inline dxfdouble 
dimeEllipse::getStartParam() const
{
  return this->startParam;
}

inline void 
dimeEllipse::setEndParam(const dxfdouble p)
{
  this->endParam = p;
}

inline dxfdouble 
dimeEllipse::getEndParam() const
{
  return this->endParam;
}

#endif // ! DIME_ELLIPSE_H

