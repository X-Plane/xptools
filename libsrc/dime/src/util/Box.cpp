/**************************************************************************\
 * 
 *  FILE: Box.cpp
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
#include <dime/util/Box.h>
#include <float.h>

template <class Type>
inline Type SIMSQR(const Type val) 
{
  return val*val;
}


dimeBox::dimeBox() 
{
  makeEmpty();
}

dimeBox::dimeBox(const dxfdouble x0, const dxfdouble y0, const dxfdouble z0, 
	       const dxfdouble x1, const dxfdouble y1, const dxfdouble z1) 
{
  this->min.setValue(x0, y0, z0);
  this->max.setValue(x1, y1, z1);
}

void 
dimeBox::set(const dxfdouble x0, const dxfdouble y0, const dxfdouble z0, 
	    const dxfdouble x1, const dxfdouble y1, const dxfdouble z1) 
{
  this->min.setValue(x0, y0, z0);
  this->max.setValue(x1, y1, z1);
}

void 
dimeBox::get(dxfdouble &x0, dxfdouble &y0, dxfdouble &z0,
	    dxfdouble &x1, dxfdouble &y1, dxfdouble &z1) const 
{
  this->min.getValue(x0, y0, z0);
  this->max.getValue(x1, y1, z1);
}

bool 
dimeBox::operator & (const dimeBox &box) const 
{
  return ! (box.min[0] >= this->max[0] || box.max[0] < this->min[0] || 
	    box.min[1] >= this->max[1] || box.max[1] < this->min[1] ||
	    box.min[2] >= this->max[2] || box.max[2] < this->min[2]);
}

void 
dimeBox::grow(const dimeVec3f &pt) 
{
  if (min[0] > pt[0]) min[0] = pt[0];
  if (max[0] < pt[0]) max[0] = pt[0];
  if (min[1] > pt[1]) min[1] = pt[1];
  if (max[1] < pt[1]) max[1] = pt[1];
  if (min[2] > pt[2]) min[2] = pt[2];
  if (max[2] < pt[2]) max[2] = pt[2];
}

dxfdouble 
dimeBox::size() const 
{
  dxfdouble dx = max[0] - min[0];
  dxfdouble dy = max[1] - min[1];
  dxfdouble dz = max[2] - min[2];
  return (dxfdouble) sqrt(dx*dx+dy*dy+dz*dz); 
}

bool 
dimeBox::hasExtent() const 
{
  return !(min[0] == FLT_MAX || min[1] == FLT_MAX || min[2] == FLT_MAX ||
	   max[0] == -FLT_MAX || max[1] == -FLT_MAX || max[2] == -FLT_MAX);
}


void 
dimeBox::makeEmpty()
{
  this->min.setValue(FLT_MAX, FLT_MAX, FLT_MAX);
  this->max.setValue(-FLT_MAX, -FLT_MAX, -FLT_MAX);  
}
