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
// sio_8211DDR.cpp
//


#ifdef NEED_TYPEINFO
#include <typeinfo>
#endif

#ifndef INCLUDED_SIO_8211DDRLEADER_H
#include <sdts++/io/sio_8211DDR.h>
#endif

#ifndef INCLUDED_SIO_8211DDRLEADER_H
#include <sdts++/io/sio_8211DDRLeader.h>
#endif

#ifndef INCLUDED_SIO_8211DIRENTRY_H
#include <sdts++/io/sio_8211DirEntry.h>
#endif

#ifndef INCLUDED_SIO_8211DIRECTORY_H
#include <sdts++/io/sio_8211Directory.h>
#endif


sio_8211DDR::sio_8211DDR()
{
   getDirectory_().setLeader( leader_ );
}


istream&
sio_8211DDR::streamExtract(istream& istr)
{
   // Assume istr is positioned on byte zero of an ISO8211 DDR.

   // Get the leader.
   istr >> getLeader_();

   if ( ! istr ) return istr;


   sio_8211DDRLeader& ddrLeader =
     static_cast<sio_8211DDRLeader&>(getLeader_()); // reference for convenience

   istr >> getDirectory_();     // Get the directory.

   if ( ! istr ) return istr;


                                // Get the fields.

   long fieldAreaStart = ddrLeader.getBaseAddrOfFieldArea();

   for ( sio_8211Directory::iterator curr_direntry = getDirectory_().begin();
         curr_direntry != getDirectory_().end();
         curr_direntry++ )
      {
         istr.seekg(fieldAreaStart + curr_direntry->getPosition());

         string fieldTag = curr_direntry->getTag();

         // note that when we add this new field to the field area, we
         // set its length to be one less than what the directory says
         // its length is -- this is to clip the field terminator, which
         // we don't want to keep around

         getFieldArea_().push_back(
             sio_8211Field(curr_direntry->getFieldLength() - 1));

         istr >> getFieldArea_().back();

         if ( ! istr ) return istr; // bail if problem

                                // establish the back link from the
                                // new directory entry to its
                                // corresponding field area; that way,
                                // we can instantly get to the field
                                // area from the directory entry

         curr_direntry->setField( &(getFieldArea_().back()) );
      }

   return istr;
} // sio_8211DDR::streamExtract(istream& istr)



sio_8211Leader const&
sio_8211DDR::getLeader() const
{
   return leader_;
}


sio_8211Leader &
sio_8211DDR::getLeader_()
{
   return leader_;
}
