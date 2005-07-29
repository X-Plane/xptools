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
// sio_8211Directory.h
//

#ifndef INCLUDED_SIO_8211DIRECTORY_H
#define INCLUDED_SIO_8211DIRECTORY_H

#include <iostream>
#include <list>

#ifndef INCLUDED_SIO_8211LEADER_H
#include <sdts++/io/sio_8211Leader.h>
#endif

#ifndef INCLUDED_SIO_8211DIRENTRY_H
#include <sdts++/io/sio_8211DirEntry.h>
#endif

#ifndef INCLUDED_SIO_8211UTILS_H
#include <sdts++/io/sio_8211Utils.h>
#endif


///
typedef std::list<sio_8211DirEntry> sio_8211DirEntryContainer;


/// Defines container of 8211 directory entries.
class sio_8211Directory : public sio_8211DirEntryContainer
{
   public:

       ///
      friend std::istream& operator>>(std::istream& istr, sio_8211Directory& dir);      
      friend std::ostream& operator<<(ostream& ostr, sio_8211Directory const& dir);


      ///
      sio_8211Directory();

      ///
      sio_8211Directory(sio_8211Directory const& dir);


      /**
       The leader is required to determine the widths of the various
       subfields in a directory entry. Once the values are determined,
       the leader is no longer required (may be deleted).
       XXX synch comment with new sio_8211Leader reality
      */
      explicit sio_8211Directory(sio_8211Leader & leader);

      /// Does a 'deep copy' of the directory contents.
      sio_8211Directory& operator=(sio_8211Directory const& rhs);



      ///
      void setLeader( sio_8211Leader & leader ) { leader_ = &leader; }

   private:

      /**
         The leader, which is stored in the encompassing sio_8211Record,
         contains formatting information used to read and write
         directory entries writable because adding directory
         entries might change the width values 
      */
      sio_8211Leader * leader_;


}; // sio_8211Directory 


///
std::istream& operator>>(std::istream& istr, sio_8211Directory& dir);

///
std::ostream& operator<<(ostream& ostr, sio_8211Directory const& dir);

#endif  // INCLUDED_SIO_8211DIRECTORY_H
