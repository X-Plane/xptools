/**************************************************************************\
 * 
 *  FILE: Section.cpp
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
  \class dimeSection dime/sections/Section.h
  \brief The dimeSection class is the superclass for all \e section classes.
  
  Currently supported sections are:
  - Header   (dimeHeaderSection)
  - Classes  (dimeClassSection)
  - Tables   (dimeTableSection)
  - Blocks   (dimeBlockSection)
  - Entities (dimeEntititySection)
  - Objects  (dimeObjectSection)
*/

#include <dime/sections/Section.h>
#include <string.h>
#include <dime/util/MemHandler.h>
#include <dime/sections/UnknownSection.h>
#include <dime/sections/EntitiesSection.h>
#include <dime/sections/HeaderSection.h>
#include <dime/sections/BlocksSection.h>
#include <dime/sections/TablesSection.h>
#include <dime/sections/ClassesSection.h>
#include <dime/sections/ObjectsSection.h>

/*!
  \fn int dimeSection::countRecords() const
  Returns the number of records in this section. 
*/

/*!
  Constructor
*/

dimeSection::dimeSection(dimeMemHandler * const memhandler)
  : memHandler( memhandler )
{
}
 
/*!
  Empty virtual destructor.
*/

dimeSection::~dimeSection()
{
}

/*!
  Static function used to create the correct section object
  from a text string.
*/

dimeSection *
dimeSection::createSection(const char * const sectionname,
			  dimeMemHandler *memhandler)
{
  if (!strcmp(sectionname, "HEADER"))
    return new dimeHeaderSection(memhandler);
#if 0 // passthrough for the moment. I can't imaging anybody is using them 
  if (!strcmp(sectionname, "CLASSES"))
    return new dimeClassesSection(memhandler);
  if (!strcmp(sectionname, "OBJECTS"))
    return new dimeObjectsSection(memhandler);
#endif
  if (!strcmp(sectionname, "TABLES"))
    return new dimeTablesSection(memhandler);
  if (!strcmp(sectionname, "BLOCKS"))
    return new dimeBlocksSection(memhandler);
  if (!strcmp(sectionname, "ENTITIES"))
    return new dimeEntitiesSection(memhandler);
  return new dimeUnknownSection(sectionname, memhandler);
}

//!

bool 
dimeSection::isOfType(const int thetypeid) const
{
  return thetypeid == dimeSectionType ||
    dimeBase::isOfType(thetypeid);
}

/*!
  \fn const char * dimeSection::getSectionName() const = 0
*/

/*!
  \fn dimeSection * dimeSection::copy(dimeModel * const model) const = 0
*/

/*!
  \fn bool dimeSection::read(dimeInput * const file) = 0
*/

/*!
  \fn bool dimeSection::write(dimeOutput * const file) = 0
*/

/*!
  \fn int dimeSection::typeId() const = 0
*/

