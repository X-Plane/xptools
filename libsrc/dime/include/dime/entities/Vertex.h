/**************************************************************************\
 * 
 *  FILE: Vertex.h
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

#ifndef DIME_VERTEX_H
#define DIME_VERTEX_H

#include <dime/Basic.h>
#include <dime/entities/Entity.h>
#include <dime/util/Linear.h>

class DIME_DLL_API dimeVertex : public dimeEntity
{
  friend class dimePolyline;
  friend class dimeEntity;
  
public:
  dimeVertex();

  enum Flags {
    CURVE_FITTING_VERTEX   = 0x01,
    HAS_CURVE_FIT_TANGENT  = 0x02,
    SPLINE_VERTEX          = 0x08,
    FRAME_CONTROL_POINT    = 0x10,
    POLYLINE_3D_VERTEX     = 0x20,
    POLYGON_MESH_VERTEX    = 0x40,
    POLYFACE_MESH_VERTEX   = 0x80
  };
  
  virtual dimeEntity *copy(dimeModel * const model) const;
  virtual bool getRecord(const int groupcode,
			 dimeParam &param,
			 const int index = 0) const;
  virtual const char *getEntityName() const;

  int16 getFlags() const;
  void setFlags(const int16 flags);

  void setCoords(const dimeVec3f &v);
  const dimeVec3f &getCoords() const;
  
  int numIndices() const;
  int getIndex(const int idx) const;
  void setIndex(const int idx, const int val);

  virtual bool write(dimeOutput * const out);
  virtual int typeId() const;
  virtual int countRecords() const;

protected:
  virtual bool handleRecord(const int groupcode, 
			    const dimeParam &param,
                            dimeMemHandler * const memhandler);
  
private:
  int16 flags;
#ifdef DIME_FIXBIG
  int32 indices[4];
#else
  int16 indices[4];
#endif
  dimeVec3f coords;
  dimePolyline *polyline; // link back to polyline...

}; // class dimeVertex

inline void 
dimeVertex::setCoords(const dimeVec3f &v)
{
  this->coords = v;
}

inline const dimeVec3f &
dimeVertex::getCoords() const
{
  return this->coords;
}

inline void 
dimeVertex::setIndex(const int idx, const int val)
{
  assert(idx >= 0 && idx < 4);
  this->indices[idx] = val;
}

inline int16 
dimeVertex::getFlags() const
{
  return this->flags;
}

inline void 
dimeVertex::setFlags(const int16 flags)
{
  this->flags = flags;
}

#endif // ! DIME_VERTEX_H

