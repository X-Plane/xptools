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

#ifndef INCLUDED_SC_MODULE_H
#define INCLUDED_SC_MODULE_H

#include <string>
#include <list>
#include <iostream>

#ifndef INCLUDED_SC_RECORD_H
#include <sdts++/container/sc_Record.h>
#endif


/// Canonical record container type
typedef std::list<sc_Record> sc_RecordCntr;


/// Corresponds to an SDTS module
/**

   SDTS logical module, which is a container of SDTS records.

   /sa class sc_Record

*/
class sc_Module : public sc_RecordCntr
{
   public:
      
      ///
      typedef sc_RecordCntr::iterator iterator;

      ///
      typedef sc_RecordCntr::const_iterator const_iterator;

   private:

      friend std::ostream& operator<<( std::ostream&, sc_Module const& );

}; // sc_Module

#endif  // INCLUDED_SC_MODULE_H
