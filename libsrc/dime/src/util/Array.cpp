/**************************************************************************\
 * 
 *  FILE: Array.cpp
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
  \class dimeArray dime/util/Array.h
  \brief The dimeArray class is internal / private.

  The dimeArray class is a container class for a growable array.  Whenever
  the allocated space for the array becomes too small, the array is moved to
  a memory block that is twice as large.  This class is dangerous to use,
  because it does not check for bounds and other things for efficiency
  reasons.  Inspect the source code - don't assume anything...
*/

/*!
  \fn void dimeArray::removeElem( const int index )
  This method removes an element from the array, moving all subsequent
  elements one position up.
*/

/*!
  \fn void dimeArray::removeElemFast( const int index )
  This method removes an element from the array, and moves the last element
  into its place at the same index.  The order of the elements is distorted,
  in other words.
*/

/*!
  \fn void dimeArray::setCount( const int count )
  This method sets the logical size of the array to \a count without
  changing the size of the array.  Do not increase the array size with
  this function!  This method is useful for reusing an array that will
  grow to approximately the same size each time it is used.
*/

/*!
  \fn void dimeArray::makeEmpty( const int initsize = 4 )
  This method makes the logical array empty, and deallocates the memory used
  by it, only allocating space for a small array of size \a initsize.
*/

/*!
  \fn int dimeArray::count() const
  This method returns the number of elements in the array.
*/

/*!
  \fn int dimeArray::allocSize() const
  This method returns the size allocated for the array.
*/

/*!
  \fn void dimeArray::freeMemory()
  This method frees all the memory used by the class.  The dimeArray class
  is probably unusable afterwards.
*/

/*!
  \fn T * dimeArray::arrayPointer()
  This method returns a pointer to the allocated array.
*/

/*!
  \fn const T * dimeArray::constArrayPointer() const
  This method returns a pointer to the allocated array.
*/

/*!
  \fn void dimeArray::shrinkToFit()
  This method moves the array into a memory block exactly the same size of
  the array.  This will free up any overhead caused by the array doubling
  mechanism.
*/

