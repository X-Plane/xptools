/**************************************************************************\
 * 
 *  FILE: BSPTree.cpp
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
#include <dime/util/BSPTree.h>
#include <dime/util/Box.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>


/*!
  \class dimeBSPTree
  \brief The dimeBSPTree class is a simple BSP tree implementation.
*/

// define this to do a sorted split (slower, but more efficient?)
// #define BSP_SORTED_SPLIT

class dime_bspnode
{
public:
  dime_bspnode(dimeArray <dimeVec3f> *array);
  ~dime_bspnode();

  int addPoint(const dimeVec3f &pt, const int maxpts);
  int findPoint(const dimeVec3f &pt) const;
  int removePoint(const dimeVec3f &pt);

private:
  void sort();
  void split();
  bool leftOf(const dimeVec3f &pt) const;

  enum {
    // do not change these values!
    DIM_YZ = 0,
    DIM_XZ = 1,
    DIM_XY = 2,
    DIM_NONE
  };
  
  dime_bspnode *left;
  dime_bspnode *right;
  int dimension;   // which dimension?
  // position in dimension (use double to avoid floating point
  // precision problems)
  double position;  
  dimeArray <int> indices;
  dimeArray <dimeVec3f> *pointsArray;
};

dime_bspnode::dime_bspnode(dimeArray <dimeVec3f> *ptsarray)
  : indices(4)
{
  this->left = this->right = NULL;
  this->pointsArray = ptsarray;
  this->dimension = DIM_NONE;
}

dime_bspnode::~dime_bspnode()
{
  delete left;
  delete right;
}

inline bool 
dime_bspnode::leftOf(const dimeVec3f &pt) const
{
  return double(pt[this->dimension]) < this->position; 
}

int 
dime_bspnode::addPoint(const dimeVec3f &pt, const int maxpts)
{
  if (this->left) { // node has been split
    if (this->leftOf(pt)) return this->left->addPoint(pt, maxpts);
    else return this->right->addPoint(pt, maxpts);
  }
  else if (this->indices.count() >= maxpts) {
    split();
    return this->addPoint(pt, maxpts);
  }
  else {
    int n = this->indices.count();
    int i;
    dimeVec3f tmp;
    for (i = 0; i < n; i++) {
      pointsArray->getElem(this->indices[i], tmp);
      if (pt == tmp) break;
    }
    if (i == n) {
      int idx = this->pointsArray->count();      
      this->pointsArray->append(pt);
      this->indices.append(idx);
      return idx;
    }
    return this->indices[i];
  }
}
 
int 
dime_bspnode::findPoint(const dimeVec3f &pt) const
{
  if (this->left) {
    if (this->leftOf(pt)) return this->left->findPoint(pt);
    else return this->right->findPoint(pt);
  }
  else {
    int i, n = this->indices.count();
    for (i = 0; i < n; i++) {
      dimeVec3f arrpt;
      this->pointsArray->getElem(this->indices[i], arrpt);
      if (pt == arrpt) return this->indices[i];
    }
  }
  return -1;
}

int 
dime_bspnode::removePoint(const dimeVec3f &pt)
{
  if (this->left) {
    if (this->leftOf(pt)) return this->left->removePoint(pt);
    else return this->right->removePoint(pt);
  }
  else {
    int i, n = this->indices.count();
    for (i = 0; i < n; i++) {
      dimeVec3f arrpt;
      this->pointsArray->getElem(this->indices[i], arrpt);
      if (pt == arrpt) {
	int idx = this->indices[i];
	this->indices.removeElemFast(i);
	return idx;
      }
    }
  }
  return -1;

}

void 
dime_bspnode::split()
{
  assert(this->left == NULL && this->right == NULL);
  this->left = new dime_bspnode(this->pointsArray);
  this->right = new dime_bspnode(this->pointsArray); 

  dimeBox box;
  int i, n = this->indices.count();
  for (i = 0; i < n; i++) {
    box.grow(this->pointsArray->getElem(this->indices[i]));
  }
  dimeVec3f diag = box.max - box.min;
  int dim;
  double pos;

  if (diag[0] > diag[1]) {
    if (diag[0] > diag[2]) dim = DIM_YZ;
    else dim = DIM_XY;
  }
  else {
    if (diag[1] > diag[2]) dim = DIM_XZ;
    else dim = DIM_XY;
  }

  this->dimension = dim; // set the dimension

  dxfdouble mid = (box.min[dim] + box.max[dim]) / 2.0f;
#ifdef BSP_SORTED_SPLIT  
  this->sort(); // sort vertices on ascending dimension values
  
  int splitidx = n / 2;
  pos = (this->pointsArray->getElem(this->indices[splitidx-1])[dim]+
	 this->pointsArray->getElem(this->indices[splitidx])[dim])/ 2.0f;
  
  // got to check and adjust for special cases
  if (pos == box.min[dim] || pos == box.max[dim]) { 
    pos = (pos + mid) / 2.0f;
  }

#else
  pos = (double(box.min[this->dimension])+double(box.max[this->dimension])) / 2.0;
#endif // BSP_SORTED_SPLIT
  
  this->position = pos;
  
  for (i = 0; i < n; i++) {
    int idx = this->indices[i];
    if (this->leftOf(this->pointsArray->getElem(idx)))
      this->left->indices.append(idx);
    else
      this->right->indices.append(idx);
  }  
  assert(this->left->indices.count() && this->right->indices.count());
  
  // will never be used anymore
  this->indices.freeMemory();
}

//
// an implementation of the shellsort algorithm
//
void 
dime_bspnode::sort()
{
  int *idxarray = this->indices.arrayPointer();
  int num = this->indices.count();
  int dim = this->dimension;
  dimeVec3f *points = this->pointsArray->arrayPointer();
  int i, j, distance;
  int idx;
  for (distance = 1; distance <= num/9; distance = 3*distance + 1);
  for (; distance > 0; distance /= 3) {
    for (i = distance; i < num; i++) {
      idx = idxarray[i];
      j = i;
      while (j >= distance && 
	     points[idxarray[j-distance]][dim] > points[idx][dim]) {
        idxarray[j] = idxarray[j-distance];
        j -= distance;
      }
      idxarray[j] = idx;
    }
  }
}

/*!
  Constructor. Will create an empty BSP tree with one node.
  \a maxnodepts is the maximume number of points in a BSP
  node. \a initsize is the initial size of the arrays that 
  holds the coordinates and userdata.
*/
dimeBSPTree::dimeBSPTree(const int maxnodepts, const int initsize)
  : pointsArray(initsize),
    userdataArray(initsize)
{
  this->boundingBox = new dimeBox;
  this->topnode = new dime_bspnode(&this->pointsArray);
  this->maxnodepoints = maxnodepts;
}

/*!
  Destructor. Will free all memory used.
*/
dimeBSPTree::~dimeBSPTree()
{
  delete this->topnode;
  delete this->boundingBox;
}
  
/*!
  Returns the number of points in the BSP tree.
*/
int 
dimeBSPTree::numPoints() const
{
  return this->pointsArray.count();
}

/*!
  Returns the coordinates for the point at index \a idx.
  \sa dimeBSPTree::numPoints()
*/
void 
dimeBSPTree::getPoint(const int idx, dimeVec3f &pt)
{
  assert(idx < this->pointsArray.count());
  this->pointsArray.getElem(idx, pt);
}

/*!
  Returns the user data for the point at index \a idx.
*/
void *
dimeBSPTree::getUserData(const int idx) const
{
  assert(idx < this->userdataArray.count());
  return this->userdataArray[idx];
}

/*!
  Sets the user data for the point with index \a idx.
*/
void 
dimeBSPTree::setUserData(const int idx, void * const data)
{
  assert(idx < this->userdataArray.count());
  this->userdataArray[idx] = data;
}

/*!
  Attempts to add a new point into the BSP tree. If a point
  with the same coordinates as \a pt already is in the tree,
  the index to that point will be returned. Otherwise, the
  point is appended at the end of the list of points, the userdata 
  is set, and the new index is returned.
*/
int 
dimeBSPTree::addPoint(const dimeVec3f &pt, void * const data)
{
  this->boundingBox->grow(pt);
  int ret = this->topnode->addPoint(pt, this->maxnodepoints);
  if (ret == this->userdataArray.count()) {
    this->userdataArray.append(data);
  }
  return ret;
}

/*!
  \overload
*/
int 
dimeBSPTree::removePoint(const dimeVec3f &pt)
{
  return this->topnode->removePoint(pt);
}

/*!
  Removes the point at \a index. The BSP tree will not be
  restructured, no matter how many points you remove.
*/
void 
dimeBSPTree::removePoint(const int idx)
{
  assert(idx < this->pointsArray.count());
  this->removePoint(this->pointsArray[idx]);
}


/*!
  Searches for a point with coordinates \a pos. Returns 
  the index if found, -1 otherwise
*/

int 
dimeBSPTree::findPoint(const dimeVec3f &pos) const
{
  return topnode->findPoint(pos);
}


/*!
  Frees the memory used by the BSP tree. Do not use the BSP
  tree after this method has been called.
*/

void 
dimeBSPTree::clear(const int initsize)
{
  delete this->topnode;
  this->topnode = NULL;
  this->pointsArray.makeEmpty(initsize);
  this->userdataArray.makeEmpty(initsize);
  this->topnode = new dime_bspnode(&this->pointsArray);
  this->boundingBox->makeEmpty();
}

/*!
  Returns the bounding box for all points in the BSP tree.
*/

const dimeBox *
dimeBSPTree::getBBox() const
{
  return this->boundingBox;
}



