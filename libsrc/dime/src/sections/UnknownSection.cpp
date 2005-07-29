/**************************************************************************\
 * 
 *  FILE: UnknownSection.cpp
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
  \class dimeUnknownSection dime/sections/UnknownSection.h
  \brief The dimeUnknownSection class is used to support unknown sections.
*/

#include <dime/sections/UnknownSection.h>
#include <dime/records/Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/util/Array.h>
#include <dime/Model.h>

#include <string.h>
#include <stdio.h>

/*!
  Constructor which stores the sectioname.
*/

dimeUnknownSection::dimeUnknownSection(const char * const sectionname,
				       dimeMemHandler *memhandler)
  : dimeSection(memhandler), records( NULL ), numRecords( 0 )
{
  this->sectionName = new char[strlen(sectionname)+1];
  if (this->sectionName) strcpy(this->sectionName, sectionname);
}

/*!
  Destructor.
*/

dimeUnknownSection::~dimeUnknownSection()
{
  delete [] this->sectionName;
  if (!this->memHandler) {
    for (int i = 0; i < this->numRecords; i++)
      delete this->records[i];
    delete [] this->records;
  }
}

//!

dimeSection *
dimeUnknownSection::copy(dimeModel * const model) const
{
  int i;
  dimeMemHandler *memh = model->getMemHandler();
  dimeUnknownSection *us = new dimeUnknownSection(this->sectionName,
						memh);
  bool ok = us != NULL;
  if (ok && this->numRecords) {
    us->records = ARRAY_NEW(memh, dimeRecord*, this->numRecords);
    bool ok = us->records != NULL;
    if (ok) {
      for (i = 0; i < this->numRecords && ok; i++) {
	us->records[i] = this->records[i]->copy(memh);
	ok = us->records[i] != NULL;
      }
      us->numRecords = i;
    }
  }
  if (!ok) {
    if (!memh) delete us;
    us = NULL;
  }
//  sim_trace("unknown section copy: %p\n", us);
  return us;
}

//!

bool 
dimeUnknownSection::read(dimeInput * const file)
{
  dimeRecord *record;
  bool ok = true;
  dimeArray <dimeRecord*> array(512);
  dimeMemHandler *memhandler = file->getMemHandler();
  
  while (true) {
    record = dimeRecord::readRecord(file);
    if (record == NULL) {
      fprintf(stderr,"could not create/read record (dimeUnknownSection.cpp)"
	      "line: %d\n", file->getFilePosition());
      ok = false;
      break;
    } 
    array.append(record);
    if (record->isEndOfSectionRecord()) break;
  }
  if (ok && array.count()) {
    if (memhandler) {
      this->records = (dimeRecord**)
	memhandler->allocMem(array.count()*sizeof(dimeRecord*));
    }
    else {
      this->records = new dimeRecord*[array.count()];
    }
    if (this->records) {
      int n = this->numRecords = array.count();
      for (int i = 0; i < n; i++)
	this->records[i] = array[i];
    }
  }
  return ok;
}

//!

bool 
dimeUnknownSection::write(dimeOutput * const file)
{
  if (file->writeGroupCode(2) && file->writeString(this->sectionName)) {
    int i;
    for (i = 0; i < this->numRecords; i++) {
      if (!this->records[i]->write(file)) break;
    }
    if (i == this->numRecords) return true;
  }
  return false;
}

//!

int 
dimeUnknownSection::typeId() const
{
  return dimeBase::dimeUnknownSectionType;
}

//!

int
dimeUnknownSection::countRecords() const
{
  return this->numRecords + 1; // onw record is written in write()
}

//!

const char *
dimeUnknownSection::getSectionName() const
{
  return this->sectionName;
}

