//
// This file is part of the SDTS++ toolkit, written by the U.S.
// Geological Survey.  It is experimental software, written to support
// USGS research and cartographic data production.
//
// SDTS++ is public domain software.  It may be freely copied,
// distributed, and modified.  The USGS welcomes user feedback, but makes
// no committment to any level of support for this code.  See the SDTS
// web site at http://mcmcweb.er.usgs.gov/sdts for more information,
// including points of contact.
//

#ifndef INCLUDED_SC_RECORD_H
#define INCLUDED_SC_RECORD_H

#include <list>
#include <iostream>


#ifndef INCLUDED_SC_FIELD_H
#include <sdts++/container/sc_Field.h>
#endif


/// Canonical field container
typedef std::list<sc_Field> sc_FieldCntr;


/// Corresponds to an SDTS logical record
/**
   An SDTS record contains SDTS fields.
*/
class sc_Record : public sc_FieldCntr
{
   public:

      ///
      typedef sc_FieldCntr::iterator iterator;

      ///
      typedef sc_FieldCntr::const_iterator const_iterator;

   private:

      friend std::ostream& operator<<( std::ostream&, sc_Record const& );

}; // sc_Record


#endif  // INCLUDED_SC_RECORD_H
