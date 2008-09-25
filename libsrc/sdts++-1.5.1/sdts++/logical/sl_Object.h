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
// $Id: sl_Object.h,v 1.4 2000/07/25 20:05:42 mcoletti Exp $

#ifndef INCLUDED_SL_OBJECT_H
#define INCLUDED_SL_OBJECT_H

#include <string>
#include <boost/smart_ptr.hpp>


/**
 This is a root object that is a generalization of the common
 features of all SDTS logical entities.
*/
class sl_Object
{
   public:

      ///
      virtual ~sl_Object() {}

      /// returns the object's two letter representation code
      std::string const & code() const { return objectCode(); }

   protected:

      /**
       each sub-class will define its own object code and use this
       function to give access to it.
      */
      virtual std::string const & objectCode() const = 0;

}; // class sl_Object


/**
 reference counted smart pointer for all SDTS logical layer objects;
 primarily used for storing in heterogeneous containers
*/
typedef boost::shared_ptr<sl_Object> sl_ObjectPtr;

#endif



