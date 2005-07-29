/**************************************************************************\
 * 
 *  FILE: State.h
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

#ifndef DIME_STATE_H
#define DIME_STATE_H

#include <dime/util/Linear.h>

class dimeInsert;

class DIME_DLL_API dimeState
{
public:
  dimeState(const bool traversePolylineVertices,
	    const bool explodeInserts);
  dimeState(const dimeState &st);
  
  const dimeMatrix &getMatrix() const;
  const dimeMatrix &getInvMatrix() const;
  void getMatrix(dimeMatrix &m) const;
  void setMatrix(const dimeMatrix &matrix);
  
  enum {
    TRAVERSE_POLYLINE_VERTICES = 0x1,
    EXPLODE_INSERTS = 0x2,
    // private flags
    PUBLIC_MASK = 0x7fff,
    PRIVATE_MASK = 0x8000,
    INVMATRIX_DIRTY = 0x8000
  };

  void setFlags(const unsigned int flags);
  unsigned int getFlags() const;

  const dimeInsert *getCurrentInsert() const;

private:
  friend class dimeInsert;
  dimeMatrix matrix;
  dimeMatrix invmatrix; // to speed up things...
  unsigned int flags;
  const dimeInsert *currentInsert;
}; // class dimeState

inline const dimeMatrix &
dimeState::getMatrix() const {
  return this->matrix;
}

inline void 
dimeState::setFlags(const unsigned int flags)
{
  this->flags = (this->flags & PRIVATE_MASK) | flags;
}
 
inline unsigned int 
dimeState::getFlags() const
{
  return (this->flags & PUBLIC_MASK);
}

inline const dimeInsert *
dimeState::getCurrentInsert() const
{
  return this->currentInsert;
}

#endif // ! DIME_STATE_H

