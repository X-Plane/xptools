/**************************************************************************\
 * 
 *  FILE: MemHandler.cpp
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
  \class dimeMemHandler dime/util/MemHandler.h
  \brief The dimeMemHandler class is a special-purpose memory manager.

  Using this class will give you efficient memory allocation and extremely
  efficient deallocation.  In addition, there is almost \e no overhead on
  the memory allocated.  The drawback is that it is not possible to
  deallocate a block of memory once it is allocated; all the allocated memory
  must be deallocated in one operation.

  If you plan to use DIME in a way that requires lots of dynamic allocation
  / deallocation of for instance entities, you should not use the memory
  manager - the standard new / delete operators should be used instead.
  The most common use of dime is to import and export DXF files, so
  the data structure is just built and then freed up all at once.  For this
  kind of usage, the special-purpose memory manager is far superior to the
  system memory manager.
*/

/*!
  \class dimeMemNode src/util/MemHandler.cpp
  \brief The dimeMemNode class is internal / private.

  Mind your own business ;)
*/

#include <dime/util/MemHandler.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MEMBLOCK_SIZE 65536 // the bigger the value, the less overhead

class dimeMemNode
{
  friend class dimeMemHandler;
public:
  dimeMemNode(const int numbytes, dimeMemNode *next_node) 
    : next( next_node ), currPos( 0 ), size( numbytes )
  {
    this->block = (unsigned char*)malloc(numbytes);
  }
  
  ~dimeMemNode()
  {
    free((void*)this->block);
  }

  bool initOk() const
  {
    return (this->block != NULL);
  }
  
  void *alloc(const int numbytes, const int alignment)
  {
    unsigned int mask = alignment - 1;
    unsigned char *ret = NULL;
    if (alignment > 1) {
      if (this->currPos & mask) 
	this->currPos = (this->currPos & ~mask) + alignment;
    }
    if (this->currPos + numbytes <= this->size) {
      ret = &this->block[currPos];
      this->currPos += numbytes;
    }
    return ret;
  }

private:
  dimeMemNode *next;
  unsigned char *block;
  unsigned int currPos;
  unsigned int size;

}; // class dimeMemNode

/*!
  Constructor. Get ready for fast alloc :-)
*/

dimeMemHandler::dimeMemHandler()
  : bigmemnode( NULL )
{
  this->memnode = new dimeMemNode(MEMBLOCK_SIZE, NULL);
}

/*!
  Frees all memory used.
*/

dimeMemHandler::~dimeMemHandler()
{
  dimeMemNode *curr = this->memnode;
  dimeMemNode *next;
  
  while (curr) {
    next = curr->next;
    delete curr;
    curr = next;
  }

  curr = this->bigmemnode;
  while (curr) {
    next = curr->next;
    delete curr;
    curr = next;
  } 
}

/*!
  Bullshit function.  Can be called right after constructor 
  to test if initial memory was allocated ok.
*/

bool
dimeMemHandler::initOk() const
{
  return this->memnode && this->memnode->initOk();
}

/*!
  Allocates memory for the string, copies string into memory, and
  returns the new string pointer.
*/

char *
dimeMemHandler::stringAlloc(const char * const string)
{
  int len = strlen(string)+1;
  char *ret = (char*)this->allocMem(len, 1);
  if (ret) {
    strcpy(ret, string);
  }
  return ret;
}

/*!
  Allocates a chunk (\a size) of memory. Memory is allocates in big 
  blocks. New blocks of memory is allocated whenever needed, and
  is handled automatically. The returned pointer is aligned according 
  to the \a alignment argument. The default alignment is four bytes, 
  but when compiled on 64 bits systems the default alignment 
  should probably be changed to eight.
*/

void *
dimeMemHandler::allocMem(const int size, const int alignment)
{
  void *ret = NULL;
  if (size > MEMBLOCK_SIZE/2) { // big blocks is allocated separately.
    this->bigmemnode = new dimeMemNode(size, this->bigmemnode);
    if (!this->bigmemnode || !this->bigmemnode->initOk()) return NULL;
    ret = (void*) this->bigmemnode->block;
  }
  else {
    ret = this->memnode->alloc(size, alignment);
    if (ret == NULL) {
      this->memnode = new dimeMemNode(MEMBLOCK_SIZE, this->memnode);
      if (!this->memnode || !this->memnode->initOk()) return NULL;
      ret = this->memnode->alloc(size, alignment);
    }
  }
  return ret;
}

