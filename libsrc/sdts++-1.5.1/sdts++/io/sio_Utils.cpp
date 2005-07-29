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

#ifdef WIN32
#pragma warning( disable : 4786 )
#endif

#include <cstdlib>
#include <string.h>
#include <stdlib.h>

#include "sdts++/io/sio_Utils.h"

#include "sio_ConverterFactory.h"



static const char * ident_ = 
  "$Id: sio_Utils.cpp,v 1.5 2002/10/07 20:44:24 mcoletti Exp $";



long
sio_Utils::getLong(char const* buf, long startpos, long length)
{
   // XXX There's probably a better way to do this, but I don't see it right now.

   char* tempBuf = new char[length+1];

   strncpy(tempBuf, buf + startpos, length);
   tempBuf[length] = '\0';

   long result = atol(tempBuf);

   delete [] tempBuf;

   return result;

} // sio_Utils::getLong




string
sio_Utils::getString(char const* buf, long startpos, long length)
{
   char* tempBuf = new char[length+1];

   strncpy(tempBuf, buf + startpos, length);
   tempBuf[length] = '\0';

   string result = tempBuf;

   delete [] tempBuf;

   return result;

} // sio_Utils::getString



void
sio_Utils::dumpConverterDictionary( sio_8211_converter_dictionary const & cd,
                                    ostream & os )
{

  for ( sio_8211_converter_dictionary::const_iterator i = cd.begin();
        i != cd.end();
        i++ )
    {
      os << (*i).first << " : ";

      if ( sio_ConverterFactory::instance()->get( "BI8" ) == (*i).second)
        {
          os << "BI8\n";
        }
      else if ( sio_ConverterFactory::instance()->get( "BI16" ) == (*i).second)
        {
          os << "BI16\n";
        }
      else if ( sio_ConverterFactory::instance()->get( "BI24" ) == (*i).second)
        {
          os << "BI24\n";
        }
      else if ( sio_ConverterFactory::instance()->get( "BI32" ) == (*i).second)
        {
          os << "BI32\n";
        }
      else if ( sio_ConverterFactory::instance()->get( "BUI8" ) == (*i).second)
        {
          os << "BUI8\n";
        }
      else if ( sio_ConverterFactory::instance()->get( "BUI16" ) == (*i).second)
        {
          os << "BUI16\n";
        }
      else if ( sio_ConverterFactory::instance()->get( "BUI24" ) == (*i).second)
        {
          os << "BUI24\n";
        }
      else if ( sio_ConverterFactory::instance()->get( "BUI32" ) == (*i).second)
        {
          os << "BUI32\n";
        }
      else if ( sio_ConverterFactory::instance()->get( "BFP32" ) == (*i).second)
        {
          os << "BFP32\n";
        }
      else if ( sio_ConverterFactory::instance()->get( "BFP64" ) == (*i).second)
        {
          os << "BFP64\n";
        }
      else
        os << "unknown\n";
    }

} // dump_converter_dictionary


