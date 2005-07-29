/**************************************************************************\
 * 
 *  FILE: Polyline.h
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

#ifndef DIME_POLYLINE_H
#define DIME_POLYLINE_H

#include <dime/Basic.h>
#include <dime/entities/ExtrusionEntity.h>
#include <dime/util/Array.h>
#include <dime/util/Linear.h>


class dimeVertex;

class DIME_DLL_API dimePolyline : public dimeExtrusionEntity
{
public:
  dimePolyline();
  virtual ~dimePolyline();

  enum Type {
    POLYLINE,
    POLYFACE_MESH,
    POLYGON_MESH
  };

  enum Flags {
    CLOSED            = 0x01,
    POLYMESH_CLOSED_M = 0x01,
    CURVE_FIT         = 0x02,
    SPLINE_FIT        = 0x04,
    IS_POLYLINE_3D    = 0x08,
    IS_POLYMESH_3D    = 0x10,
    POLYMESH_CLOSED_N = 0x20,
    IS_POLYFACE_MESH  = 0x40,
    CONTINOUS_PATTERN = 0x80
  };

  enum SurfaceType {
    NONE            = 0,
    QUADRIC_BSPLINE = 5,
    CUBIC_BSPLINE   = 6,
    BEZIER          = 8
  };

  int16 getFlags() const;
  void setFlags(const int16 flags);

  int getType() const;

  const dimeVec3f &getElevation() const;
  void setElevation(const dimeVec3f &e);

  int16 getPolymeshCountN() const;
  int16 getPolymeshCountM() const;
  int16 getSmoothSurfaceMdensity() const;
  int16 getSmoothSurfaceNdensity() const;

  int getNumCoordVertices() const;
  int getNumIndexVertices() const;
  int getNumSplineFrameControlPoints() const;

  int16 getSurfaceType() const;
  void setSurfaceType(const int16 type);

  dimeVertex *getCoordVertex(const int index);
  dimeVertex *getIndexVertex(const int index);
  dimeVertex *getSplineFrameControlPoint(const int index);

  void setCoordVertices(dimeVertex **vertices, const int num,
			dimeMemHandler * const memhandler = NULL);
  void setIndexVertices(dimeVertex **vertices, const int num,
			dimeMemHandler * const memhandler = NULL);
  void setSplineFrameControlPoints(dimeVertex **vertices, const int num,
				   dimeMemHandler * const memhandler = NULL);

  virtual dimeEntity *copy(dimeModel *const model) const;
  virtual bool getRecord(const int groupcode,
			 dimeParam &param,
			 const int index = 0) const;

  virtual void setLayer(const dimeLayer * const layer);
  virtual const char *getEntityName() const;

  virtual bool read(dimeInput * const in);
  virtual bool write(dimeOutput * const out);
  virtual int typeId() const;
  virtual int countRecords() const;
   
  virtual GeometryType extractGeometry(dimeArray <dimeVec3f> &verts,
				       dimeArray <int> &indices,
				       dimeVec3f &extrusionDir,
				       dxfdouble &thickness);

  void clearSurfaceData();
    
protected:
  virtual bool handleRecord(const int groupcode, 
			    const dimeParam &param,
                            dimeMemHandler * const memhandler);
  virtual bool traverse(const dimeState * const state, 
			dimeCallback callback,
			void *userdata);
  
private:

  int numCoordVertices() const;
  int numIndexVertices() const;
 
  int16 flags;

#ifdef DIME_FIXBIG
  int32 countM;
  int32 countN;
  int32 smoothCountM;
  int32 smoothCountN;
#else
  int16 countM; 
  int16 countN; 
  int16 smoothCountM;
  int16 smoothCountN;
#endif

  int16 surfaceType;
#ifdef DIME_FIXBIG
  int32 coordCnt;  // real # of coordinate vertices
  int32 indexCnt;  // real # of index vertices
  int32 frameCnt; 
#else  
  int16 coordCnt;  // real # of coordinate vertices
  int16 indexCnt;  // real # of index vertices
  int16 frameCnt; 
#endif
  dimeVertex **coordVertices;
  dimeVertex **indexVertices;
  dimeVertex **frameVertices;
  dimeEntity *seqend;
  dimeVec3f elevation;
}; // class dimePolyline

inline int16 
dimePolyline::getFlags() const
{
  return this->flags;
}

inline void 
dimePolyline::setFlags(const int16 flags)
{
  this->flags = flags;
}

inline const dimeVec3f &
dimePolyline::getElevation() const
{
  return elevation;
}

inline void 
dimePolyline::setElevation(const dimeVec3f &e)
{
  this->elevation = e;
}

inline int16 
dimePolyline::getPolymeshCountN() const
{
  return this->countN;
}

inline int16 
dimePolyline::getPolymeshCountM() const
{
  return this->countM;
}

inline int16 
dimePolyline::getSmoothSurfaceMdensity() const
{
  return this->smoothCountM;
}

inline int16 
dimePolyline::getSmoothSurfaceNdensity() const
{
  return this->smoothCountN;
}

inline int 
dimePolyline::getNumCoordVertices() const
{
  return this->coordCnt;
}

inline int 
dimePolyline::getNumIndexVertices() const
{
  return this->indexCnt;
}

inline int
dimePolyline::getNumSplineFrameControlPoints() const
{
  return this->frameCnt;
}

inline dimeVertex *
dimePolyline::getCoordVertex(const int index)
{
  return this->coordVertices[index];
}

inline dimeVertex *
dimePolyline::getIndexVertex(const int index)
{
  return this->indexVertices[index];
}

inline dimeVertex *
dimePolyline::getSplineFrameControlPoint(const int index)
{
  return this->frameVertices[index];
}

inline int16 
dimePolyline::getSurfaceType() const
{
  return this->surfaceType;
}

inline void 
dimePolyline::setSurfaceType(const int16 type)
{
  this->surfaceType = type;
}


#endif // ! DIME_POLYLINE_H

