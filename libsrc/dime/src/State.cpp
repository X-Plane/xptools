/**************************************************************************\
 * 
 *  FILE: State.cpp
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
  \class dimeState dime/State.h
  \brief The dimeState class manages various state variables while the
  model is traversed.
*/

#include <dime/State.h>

/*!
  Constructor.
*/

dimeState::dimeState(const bool traversePolylineVertices,
		   const bool explodeInserts) 
{
  this->matrix.makeIdentity();
  this->invmatrix.makeIdentity();
  this->currentInsert = NULL;
  this->flags = 0;
  if (traversePolylineVertices) {
    this->flags |= TRAVERSE_POLYLINE_VERTICES;
  }
  if (explodeInserts) {
    this->flags |= EXPLODE_INSERTS;
  }
}

/*!
  Constructor.
*/

dimeState::dimeState(const dimeState &st) 
{
  this->matrix = st.matrix;
  this->invmatrix = st.invmatrix;
  this->flags = st.flags;
  this->currentInsert = st.currentInsert;
}

void 
dimeState::setMatrix(const dimeMatrix &m)
{
  this->matrix = m;
  this->flags |= INVMATRIX_DIRTY;
}


const dimeMatrix &
dimeState::getInvMatrix() const 
{
  if (this->flags & INVMATRIX_DIRTY) {
    dimeState *thisp = (dimeState*)this;
    thisp->invmatrix = thisp->matrix;
    thisp->invmatrix.inverse();
    thisp->flags ^= INVMATRIX_DIRTY;
  }
  return this->invmatrix;
}

void 
dimeState::getMatrix(dimeMatrix &m) const 
{
  m = this->matrix;
}

