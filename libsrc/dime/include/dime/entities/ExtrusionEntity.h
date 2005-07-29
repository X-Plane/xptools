/**************************************************************************\
 * 
 *  FILE: ExtrusionEntity.h
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

#ifndef DIME_EXTRUSIONENTITY_H
#define DIME_EXTRUSIONENTITY_H

#include <dime/entities/Entity.h>

class DIME_DLL_API dimeExtrusionEntity : public dimeEntity
{
public:
  dimeExtrusionEntity();

  virtual bool getRecord(const int groupcode,
			 dimeParam &param,
			 const int index = 0) const;

  void setExtrusionDir(const dimeVec3f &v);
  const dimeVec3f &getExtrusionDir() const;

  void setThickness(const dxfdouble val);
  dxfdouble getThickness() const;
    
  virtual int typeId() const;
  virtual bool isOfType(const int thtypeid) const;
  virtual int countRecords() const;

protected:

  virtual bool handleRecord(const int groupcode,
			    const dimeParam &param,
			    dimeMemHandler * const memhandler);  

  void copyExtrusionData(const dimeExtrusionEntity * const entity);
  bool writeExtrusionData(dimeOutput * const out);
  
protected: // should be private :-(
  dimeVec3f extrusionDir;
  dxfdouble thickness;

}; // class dimeExtrusionEntity

inline void 
dimeExtrusionEntity::setExtrusionDir(const dimeVec3f &v)
{
  this->extrusionDir = v;
}

inline const dimeVec3f &
dimeExtrusionEntity::getExtrusionDir() const
{
  return this->extrusionDir;
}

inline void 
dimeExtrusionEntity::setThickness(const dxfdouble val)
{
  this->thickness = val;
}

inline dxfdouble 
dimeExtrusionEntity::getThickness() const
{
  return this->thickness;
}

#endif // ! DIME_EXTRUSIONENTITY_H

