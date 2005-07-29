/**************************************************************************\
 * 
 *  FILE: Dict.cpp
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
  \class dimeDict dime/util/Dict.h
  \brief The dimeDict class is internal / private.

  It offers quick (hashing) lookup for strings.
*/

/*!
  \class dimeDictEntry dime/util/Dict.h
  \brief The dimeDictEntry class is internal / private.
*/

#include <dime/util/Dict.h>
#include <stdio.h>

/*!
  Constructor.  Creates \a entries buckets.
*/

dimeDict::dimeDict(const int entries)
{
  this->tableSize = entries;
  this->buckets = new dimeDictEntry *[tableSize];
  for (int i = 0; i < tableSize; i++)
    buckets[i] = NULL;
}

/*!
  Destructor.
*/

dimeDict::~dimeDict()
{
  clear();
  delete [] buckets;
}

/*!
  Clear all entries in the dictionary.  
*/

void
dimeDict::clear()
{
  int i;
  dimeDictEntry *entry, *nextEntry;

  for(i = 0; i < tableSize; i++) {
    for(entry = buckets[i]; entry != NULL; entry = nextEntry) {
      nextEntry = entry->next;
      delete entry;
    }
    buckets[i] = NULL;
  }
}

/*!
  Insert a new string \a key in the dictionary, and associates a
  id (= \a value) to it. Returns a pointer
  to the string. If the string is already in the dictionary, the
  old id is replaced with \a value;
*/

const char *
dimeDict::enter(const char * const key, void *value)
{
  dimeDictEntry *&entry = findEntry(key);
  
  if(entry == NULL) {
    entry = new dimeDictEntry(key, value);
    if (entry == NULL) return NULL;
    entry->next = NULL;
    return entry->key;
  }
  else {
    entry->value = value;
    return entry->key;
  }
}

/*!
  Inserts a new string \a key in the dictionary. This function
  returns \e true if the string was not already in the dictionary,
  \e false otherwise. The string pointer is returned in \a ptr.
*/

bool 
dimeDict::enter(const char * const key, char *&ptr, void *value)
{
  dimeDictEntry *&entry = findEntry(key);
  
  if(entry == NULL) {
    entry = new dimeDictEntry(key, value);
    if (entry == NULL) {
      ptr = NULL;
      return false;
    }
    entry->next = NULL;
    ptr = entry->key;
    return true;
  }
  else {
    entry->value = value;
    ptr = entry->key;
    return false;
  }
}

/*!
  Finds if \a key is in the dictionary. Returns the string pointer
  if found, NULL otherwise.  
*/

const char *
dimeDict::find(const char * const key) const
{
  dimeDictEntry *&entry = findEntry(key);
  if (entry) 
    return entry->key;
  return NULL;
}

/*!
  Finds if \a key is in the dictionary. Returns true if found.
*/

bool
dimeDict::find(const char * const key, void *&value) const
{
  dimeDictEntry *&entry = findEntry(key);

  if(entry == NULL) {
    value = NULL;
    return false;
  }
  else {
    value = entry->value;
    return true;
  }
}

/*!
  Remove \a key from the dictionary.
*/

bool
dimeDict::remove(const char * const key)
{
  dimeDictEntry *&entry = findEntry(key);
  dimeDictEntry *tmp;

  if(entry == NULL)
    return false;
  else {
    tmp = entry;
    entry = entry->next;
    delete tmp;
    return true;
  }
}

// private funcs

dimeDictEntry *&
dimeDict::findEntry(const char * const key) const
{
  dimeDictEntry **entry;

  entry = &buckets[bucketNr(key) % tableSize];
  
  while(*entry != NULL) {
    if(strcmp((*entry)->key, key) == 0) break;
    entry = &(*entry)->next;
  }
  return *entry;
}

unsigned int
dimeDict::bucketNr(const char *s) const
{
  unsigned int total, shift;
  total = shift = 0;
  while (*s) {
    total = total ^ ((*s) << shift);
    shift+=5;
    if (shift>24) shift -= 24;
    s++;
  }
  return total % tableSize;
}

/*
  For debugging
*/

void
dimeDict::dump(void)
{
  int i;
  dimeDictEntry *entry, *nextEntry;

  for(i = 0; i < tableSize; i++) {
    for(entry = buckets[i]; entry != NULL; entry = nextEntry) {
      nextEntry = entry->next;
      printf("entry: '%s' %p\n", entry->key, entry->value);
    }
  }
}

void 
dimeDict::print_info()
{
  int i, cnt;
  dimeDictEntry *entry;

  printf("---------- dict info ------------------\n");

  for (i = 0; i < tableSize; i++) {
    entry = buckets[i];
    cnt = 0;
    while(entry) {
      entry = entry->next;
      cnt++;
    }
    printf(" bucket: %d, cnt: %d\n",i, cnt);
  }
  printf("\n\n\n");
}

