/**************************************************************************\
 * 
 *  FILE: Box.h
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

#ifndef DIME_BOX_H
#define DIME_BOX_H

#include <dime/Basic.h>
#include <dime/util/Linear.h>

class DIME_DLL_API dimeBox
{
public:
  dimeVec3f min, max;
public:
  dimeBox();
  dimeBox(const dxfdouble x0, const dxfdouble y0, const dxfdouble z0, 
	 const dxfdouble x1, const dxfdouble y1, const dxfdouble z1);
  
  void set(const dxfdouble x0, const dxfdouble y0, const dxfdouble z0, 
	   const dxfdouble x1, const dxfdouble y1, const dxfdouble z1);

  void get(dxfdouble &x0, dxfdouble &y0, dxfdouble &z0,
	   dxfdouble &x1, dxfdouble &y1, dxfdouble &z1) const;

  bool operator & (const dimeBox &box) const;
  
  bool pointInside(const dimeVec3f &pt) const;

  dimeVec3f center() const;
  
  void makeEmpty();
  void grow(const dimeVec3f &pt);
  dxfdouble size() const;
  bool hasExtent() const;
}; // class dimeBox

inline bool 
dimeBox::pointInside(const dimeVec3f &pt) const 
{
  return ! (pt[0] < this->min[0] || pt[0] >= this->max[0] || 
	    pt[1] < this->min[1] || pt[1] >= this->max[1] ||
	    pt[2] < this->min[2] || pt[2] >= this->max[2]);
}

inline dimeVec3f 
dimeBox::center() const 
{
  return dimeVec3f((min[0]+max[0])*0.5f, 
		   (min[1]+max[1])*0.5f,
		   (min[2]+max[2])*0.5f);
}

#endif // ! DIME_BOX_H

