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
//
// sio_8211DDRLeader.h


#ifndef INCLUDED_SIO_8211DDRLEADER_H
#define INCLUDED_SIO_8211DDRLEADER_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include <iostream>
#include <iomanip>

#ifndef INCLUDED_SIO_8211LEADER_H
#include <sdts++/io/sio_8211Leader.h>
#endif



/// This corresponds to the DDR leader data structure
class sio_8211DDRLeader : public sio_8211Leader  
{
   public:

      ///
      sio_8211DDRLeader();

      ///
      ~sio_8211DDRLeader();

      ///
      long getFieldControlLength() const;

   private:

      ///
      bool isValid( ) const;

      ///
      char interchangeLevel_;

      ///
      char inlineCodeExtensionIndicator_;

      ///
      char versionNumber_;

      ///
      char appIndicator_;

      ///
      long fieldControlLength_;

      ///
      std::istream& streamExtract( std::istream & istr );

      ///
      std::ostream& streamInsert( std::ostream & ostr ) const;

}; // class sio_8211DDRLeader


#endif  // INCLUDED_SIO_8211DDRLEADER_H

