/**************************************************************************\
 * 
 *  FILE: Spline.h
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

#ifndef DIME_SPLINE_H
#define DIME_SPLINE_H

#include <dime/entities/ExtrusionEntity.h>
#include <assert.h>

class DIME_DLL_API dimeSpline : public dimeEntity
{
public:
  dimeSpline();
  virtual ~dimeSpline();

  enum Flags {
    CLOSED   = 0x01,
    PERIODIC = 0x02, 
    RATIONAL = 0x04,
    PLANAR   = 0x08,
    LINEAR   = 0x10
  };

  bool hasWeights() const;

  int16 getFlags() const;
  void setFlags(const int16 flags);

  int16 getDegree() const;
  void setDegree(const int16 degree);

  dxfdouble getControlPointTolerance() const;
  void setControlPointTolerance(const dxfdouble tol);
  dxfdouble getFitPointTolerance() const;
  void setFitPointTolerance(const dxfdouble tol);
  dxfdouble getKnotTolerance() const;
  void setKnotTolerance(const dxfdouble tol);

  int getNumKnots() const;
  dxfdouble getKnotValue(const int idx) const;
  void setKnotValue(const int idx, const dxfdouble value);
  void setKnotValues(const dxfdouble * const values, const int numvalues,
		     dimeMemHandler * const memhandler = NULL);
  
  int getNumControlPoints() const;
  const dimeVec3f &getControlPoint(const int idx) const;
  void setControlPoint(const int idx, const dimeVec3f &v);
  void setControlPoints(const dimeVec3f * const pts, const int numpts,
			dimeMemHandler * const memhandler = NULL);
  
  int getNumWeights() const;
  dxfdouble getWeight(const int idx) const;
  void setWeight(const int idx, const dxfdouble w,
		 dimeMemHandler * const memhandler = NULL);
  
  int getNumFitPoints() const;
  const dimeVec3f &getFitPoint(const int idx) const;
  void setFitPoint(const int idx, const dimeVec3f &pt);
  void setFitPoints(const dimeVec3f * const pts, const int numpts,
		    dimeMemHandler * const memhandler = NULL);
  
  virtual dimeEntity *copy(dimeModel * const model) const;
  virtual bool getRecord(const int groupcode,
			 dimeParam &param,
			 const int index) const;
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
  int16 flags;
#ifdef DIME_FIXBIG
  int32 degree;
  int32 numKnots;
  int32 numControlPoints;
  int32 numFitPoints;
#else
  int16 degree;
  int16 numKnots;
  int16 numControlPoints;
  int16 numFitPoints;
#endif
  dxfdouble knotTolerance;
  dxfdouble fitTolerance;
  dxfdouble cpTolerance;
  dxfdouble *knots;
  dxfdouble *weights;
  dimeVec3f *controlPoints;
  dimeVec3f *fitPoints;

  // read/handle counters
  int16 knotCnt;
  int16 fitCnt;
  int16 cpCnt;
  int16 weightCnt;

}; // class dimeSpline

inline int16 
dimeSpline::getFlags() const
{
  return this->flags;
}

inline void 
dimeSpline::setFlags(const int16 flags)
{
  this->flags = flags;
}

inline int16 
dimeSpline::getDegree() const
{
  return this->degree;
}

inline void 
dimeSpline::setDegree(const int16 degree)
{
  this->degree = degree;
}

inline dxfdouble 
dimeSpline::getControlPointTolerance() const
{
  return this->cpTolerance;
}

inline void 
dimeSpline::setControlPointTolerance(const dxfdouble tol)
{
  this->cpTolerance = tol;
}

inline dxfdouble 
dimeSpline::getFitPointTolerance() const
{
  return this->fitTolerance;
}

inline void 
dimeSpline::setFitPointTolerance(const dxfdouble tol)
{
  this->fitTolerance = tol;
}

inline dxfdouble 
dimeSpline::getKnotTolerance() const
{
  return this->knotTolerance;
}

inline void 
dimeSpline::setKnotTolerance(const dxfdouble tol)
{
  this->knotTolerance = tol;
}

inline int 
dimeSpline::getNumKnots() const
{
  return this->numKnots;
}

inline dxfdouble 
dimeSpline::getKnotValue(const int idx) const
{
  assert(idx >= 0 && idx < this->numKnots);
  return this->knots[idx];
}

inline void 
dimeSpline::setKnotValue(const int idx, const dxfdouble value)
{
  assert(idx >= 0 && idx < this->numKnots);
  this->knots[idx] = value;
}

inline int 
dimeSpline::getNumControlPoints() const
{
  return this->numControlPoints;
}

inline const dimeVec3f &
dimeSpline::getControlPoint(const int idx) const
{
  assert(idx >= 0 && idx < this->numControlPoints);
  return this->controlPoints[idx];
}

inline void 
dimeSpline::setControlPoint(const int idx, const dimeVec3f &v)
{
  assert(idx >= 0 && idx < this->numControlPoints);
  this->controlPoints[idx] = v;
}

inline int 
dimeSpline::getNumWeights() const
{
  return this->getNumControlPoints();
}

inline dxfdouble 
dimeSpline::getWeight(const int idx) const
{
  if (this->hasWeights()) {
    assert(idx >= 0 && idx < this->numControlPoints);
    return this->weights[idx];
  }
  return 1.0;
}

inline int 
dimeSpline::getNumFitPoints() const
{
  return this->numFitPoints;
}

inline const dimeVec3f &
dimeSpline::getFitPoint(const int idx) const
{
  assert(idx >= 0 && idx < this->numFitPoints);
  return this->fitPoints[idx];
}

inline void 
dimeSpline::setFitPoint(const int idx, const dimeVec3f &pt)
{
  assert(idx >= 0 && idx < this->numFitPoints);
  this->fitPoints[idx] = pt;
}
 
#endif // ! DIME_SPLINE_H

