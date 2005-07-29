/**************************************************************************\
 * 
 *  FILE: ClassesSection.cpp
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
  \class dimeClassesSection dime/sections/ClassesSection.h
  \brief The dimeClassesSection class handles a CLASSES \e section.
*/

#include <dime/sections/ClassesSection.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>
#include <dime/classes/Class.h>
#include <dime/Model.h>

#include <string.h>

static const char sectionName[] = "CLASSES";

/*!
  Constructor.
*/

dimeClassesSection::dimeClassesSection(dimeMemHandler * const memhandler)
  : dimeSection(memhandler)
{
}

/*!
  Destructor.
*/

dimeClassesSection::~dimeClassesSection()
{
  if (!this->memHandler) {
    for (int i = 0; i < this->classes.count(); i++)
      delete this->classes[i];
  }
}

//!

dimeSection *
dimeClassesSection::copy(dimeModel * const model) const
{
  dimeMemHandler *memh = model->getMemHandler();
  dimeClassesSection *cs = new dimeClassesSection(memh); 
  bool ok = cs != NULL;

  int num  = this->classes.count();
  if (ok && num) {
    cs->classes.makeEmpty(num);
    for (int i = 0; i < num; i++) {
      cs->classes.append(this->classes[i]->copy(model));
      if (cs->classes[i] == NULL) {
	ok = false;
	break;
      }
    }
  }
  
  if (!ok) {
    if (!memh) delete cs;
    cs = NULL;
  }
//  sim_trace("classes section copy: %p\n", cs);
  return cs;
}
 
//!

bool 
dimeClassesSection::read(dimeInput * const file)
{
  int32 groupcode;
  const char *string;
  bool ok = true;
  dimeClass *myclass = NULL;
  dimeMemHandler *memhandler = file->getMemHandler();
  this->classes.makeEmpty(64);

//  sim_trace("Reading section: CLASSES\n");

  while (true) {
    if (!file->readGroupCode(groupcode) || (groupcode != 9 && groupcode != 0)) {
      fprintf(stderr,"Error reading classes groupcode: %d.\n", groupcode);
//      sim_warning("Error reading classes groupcode: %d.\n", groupcode);
      ok = false;
      break;
    }
    string = file->readString();
    if (!strcmp(string, "ENDSEC")) break; 
    myclass = dimeClass::createClass(string, memhandler);
    if (myclass == NULL) {
      fprintf(stderr,"error creating class: %s.\n", string);
//      sim_warning("error creating class: %s.\n", string);
      ok = false;
      break;
    }
    if (!myclass->read(file)) {
      fprintf(stderr,"error reading class: %s.\n", string);
//      sim_warning("error reading class: %s.\n", string);
      ok = false;
      break;
    }
    this->classes.append(myclass);
  }
  return ok;
}

//!

bool 
dimeClassesSection::write(dimeOutput * const file)
{
//  sim_trace("Writing section: CLASSES\n");

  file->writeGroupCode(2);
  file->writeString(sectionName);
 
  int i, n = this->classes.count();
  for (i = 0; i < n; i++) {
    if (!this->classes[i]->write(file)) break;
  }
  if (i == n) {
    file->writeGroupCode(0);
    file->writeString("ENDSEC");
    return true;
  }
  return false;
}

//!

int 
dimeClassesSection::typeId() const
{
  return dimeBase::dimeClassesSectionType;
}

//!

int
dimeClassesSection::countRecords() const
{
  int cnt = 0;
  int n = this->classes.count();
  for (int i = 0; i < n; i++)
    cnt += this->classes[i]->countRecords();
  return cnt + 2; // two additional records are written in write()
}

//!

const char *
dimeClassesSection::getSectionName() const
{
  return sectionName;
}

/*!
  Returns the number of classes in this section. 
*/

int 
dimeClassesSection::getNumClasses() const
{
  return this->classes.count();
}

/*!
  Returns the class at index \a idx.
*/

dimeClass *
dimeClassesSection::getClass(const int idx)
{
  assert(idx >= 0 && idx < this->classes.count());
  return this->classes[idx];
}

/*!
  Removes (and deletes if no memhandler is used) the class at index \a idx.
*/

void 
dimeClassesSection::removeClass(const int idx)
{
  assert(idx >= 0 && idx < this->classes.count());
  if (!this->memHandler) delete this->classes[idx];
  this->classes.removeElem(idx);
}

/*!
  Inserts a new class at index \a idx. If \a idx is negative, the
  class will be inserted at the end of the list of classes.
*/

void 
dimeClassesSection::insertClass(dimeClass * const myclass, const int idx)
{
  if (idx < 0) this->classes.append(myclass);
  else {
    assert(idx <= this->classes.count());
    this->classes.insertElem(idx, myclass);
  }
}

