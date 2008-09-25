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

#ifndef INCLUDED_SIO_UTILS_H
#define INCLUDED_SIO_UTILS_H

#ifdef WIN32
#pragma warning( disable : 4786 )
#endif

#include <string>
#include <iostream>


#ifndef INCLUDED_SIO8211FIELDFORMAT_H
#include <sdts++/io/sio_8211FieldFormat.h>
#endif

using std::string;
using std::ostream;


/// Simple utilities
namespace  sio_Utils
{
   /**
       convert (hopefully) character representation of numeric into long
   */
   long getLong(char const* buf, long startpos, long length);

   /**
      convert the specific raw string into a std string
    */
   std::string getString(char const* buf, long startpos, long length);


   /**
      Just dumps out the binary converter map to the given output stream
   */
   void dumpConverterDictionary( sio_8211_converter_dictionary const & cd,
                                 ostream & os );

} // namespace sio_Utils

#endif  // INCLUDED_SIO_UTILS_H
