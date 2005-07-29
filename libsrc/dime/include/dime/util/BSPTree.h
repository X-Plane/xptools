/**************************************************************************\
 * 
 *  FILE: BSPTree.h
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

#ifndef DIME_BSPTREE_H
#define DIME_BSPTREE_H

#include <dime/Basic.h>
#include <dime/util/Array.h>
#include <dime/util/Linear.h>

class dimeBox;
class dime_bspnode;

class DIME_DLL_API dimeBSPTree
{
public:
  dimeBSPTree(const int maxnodepts = 64, const int initsize = 4);
  ~dimeBSPTree();
  
  int numPoints() const;
  void getPoint(const int idx, dimeVec3f &pt);
  void *getUserData(const int idx) const;
  
  void setUserData(const int idx, void * const data);
  
  int addPoint(const dimeVec3f &pt, void * const userdata = NULL);
  int removePoint(const dimeVec3f &pt);
  void removePoint(const int idx);
  int findPoint(const dimeVec3f &pos) const;
  void clear(const int initsize = 4);
  
  const dimeBox *getBBox() const;
  
private:
  friend class dime_bspnode;
  dimeArray <dimeVec3f> pointsArray;
  dimeArray <void*> userdataArray;
  dime_bspnode *topnode;
  int maxnodepoints;
  dimeBox *boundingBox;
}; // class dimeBSPTree

#endif // ! DIME_BSPTREE_H

