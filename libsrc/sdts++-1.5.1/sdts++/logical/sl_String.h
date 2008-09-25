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
// $Id: sl_String.h,v 1.3 2002/11/24 22:07:44 mcoletti Exp $

#ifndef INCLUDED_SL_STRING_H
#define INCLUDED_SL_STRING_H

#ifndef INCLUDED_SL_POINT_H
#include <sdts++/logical/sl_Point.h>
#endif

#include <vector>


/**
 This represents an SDTS logical string, which is a container
 of sl_Points
*/
class sl_String : public sl_Object, public std::vector<sl_Point>
{
   public:

      ///
      virtual ~sl_String() {}


   protected:

      /// returns the line object code
      virtual std::string const & objectCode() const;

}; // class sl_String



#endif
