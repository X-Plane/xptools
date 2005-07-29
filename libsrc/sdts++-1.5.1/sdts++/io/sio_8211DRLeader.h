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
// sio_8211DRLeader.h
//

#ifndef INCLUDED_SIO_8211DRLEADER_H
#define INCLUDED_SIO_8211DRLEADER_H

#include <iostream>

#ifndef INCLUDED_SIO_8211LEADER_H
#include <sdts++/io/sio_8211Leader.h>
#endif




/// This corresponds to a Data Record (DR) leader
class sio_8211DRLeader : public sio_8211Leader  
{
   public:

      ///
      sio_8211DRLeader();

   protected:

      ///
      std::istream& streamExtract( std::istream& istr );

      ///
      std::ostream& streamInsert( std::ostream& ostr ) const;

}; // class sio_8211DRLeader



#endif // INCLUDED_SIO_8211DRLEADER_H

