/**************************************************************************\
 *
 *  FILE: Base.cpp
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
  \class dimeBase dime/Base.h
  \brief The dimeBase class is the superclass for most classes in Dime.

  dimeBase implements the \e new operator to enable use of the special-purpose
  memory manager class, dimeMemHandler.  It also implements a simple run-time
  type checking system.
*/

#include <dime/Base.h>
#include <dime/util/MemHandler.h>
#include <stdio.h>

/*!
  \fn int dimeBase::typeId() const
  Must be implemented by all subclasses, and should return an unique id
  for that class.
*/

/*!
  Constructor.
*/
dimeBase::dimeBase(void)
{
}

/*!
  virtual destructor.
*/
dimeBase::~dimeBase()
{
}


/*!
  Returns \e true if the object is of type \a typeid or is inherited
  from it. Function in base class checks whether \a thetypeid
  equals the virtual dimeBase::typeId() value or equals \e dimeBaseType.
  Must be implemented by all subclasses that are superclasses of other
  classes, and should check if \a thetypeid equals its typeId,
  and then call its parent's isOfType function.  Leaf-classes
  do not have to implement this method.
*/

bool
dimeBase::isOfType(const int thetypeid) const
{
  return this->typeId() == thetypeid ||
    thetypeid == dimeBaseType;
}

void *
dimeBase::operator new(size_t size, dimeMemHandler *memhandler,
		      const int alignment)
{
  if (memhandler)
    return memhandler->allocMem(size, alignment);
  else return ::operator new(size);
}

void
dimeBase::operator delete(void * ptr)
{
  // will only get here if we don't use a memory handler
  ::operator delete(ptr);
}
