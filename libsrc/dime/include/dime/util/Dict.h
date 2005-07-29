/**************************************************************************\
 * 
 *  FILE: Dict.h
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

#ifndef DIME_DICT_H
#define DIME_DICT_H

#include <dime/Basic.h>
#include <string.h>

class DIME_DLL_API dimeDictEntry
{
  friend class dimeDict;

private:
  dimeDictEntry *next;
  dimeDictEntry(const char * const k, void *v) {key = strdup(k); value = v; };
  ~dimeDictEntry() {free(key);} 
  char *key;
  void *value;

}; // class dimeDictEntry

class DIME_DLL_API dimeDict
{
public:
  dimeDict(const int entries = 17989);
  ~dimeDict();
  void clear();

  bool enter(const char * const key, char *&ptr, void *value);
  const char *enter(const char * const key, void *value);
  const char *find(const char * const key) const;
  bool find(const char * const key, void *&value) const;
  bool remove(const char * const key);
  void dump(void);

private:
  int tableSize;
  dimeDictEntry **buckets;
  dimeDictEntry *&findEntry(const char * const key) const;
  unsigned int bucketNr(const char *key) const;

public:
  void print_info();
  
}; // class dimeDict

#endif // ! DIME_DICT_H

