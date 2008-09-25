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
// sio_8211Directory.cpp
//

#include "sio_8211Directory.h"

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#ifndef INCLUDED_SIO_8211DIRENTRY_H
#include <sdts++/io/sio_8211DirEntry.h>
#endif

#ifndef INCLUDED_SIO_8211UTILS_H
#include <sdts++/io/sio_8211Utils.h>
#endif


#include <algorithm>
#include <functional>
#include <iterator>


using namespace std;


sio_8211Directory::sio_8211Directory()
   : leader_( 0 )
{
}


sio_8211Directory::sio_8211Directory(sio_8211Leader & leader)
   : leader_( &leader )
{
}


sio_8211Directory::sio_8211Directory(sio_8211Directory const& rhs )
   : leader_( rhs.leader_ )
{
  insert( begin(), rhs.begin(), rhs.end() );
}


sio_8211Directory &
sio_8211Directory::operator=(sio_8211Directory const& rhs)
{
   if (&rhs == this)
   {
      return *this;
   }

   erase( begin(), end() );

   insert( begin(), rhs.begin(), rhs.end() );

   leader_ = rhs.leader_;

   return *this;
} // operator=()



std::istream&
operator>>( std::istream& istr, sio_8211Directory& dir )
{
   // First, make sure that we're starting with a clean directory.
   dir.erase( dir.begin(), dir.end() );


   // Read in the individual directory entries. Assumes the stream is positioned
   // on byte 0 of the first directory entry.

   while ((istr.peek() != sio_8211FieldTerminator) && (istr))
      {
         dir.push_back(  sio_8211DirEntry( )  );
         dir.back().setLeader( dir.leader_ );
         istr >> dir.back();
      }

   if (istr)
      {
         // Gobble up the field terminator at the end of the directory.
         char tempChar;
         istr.get(tempChar);
      }

   return istr;
} // operator>>




ostream& operator<<(ostream& ostr, sio_8211Directory const& dir)
{

  ostream_iterator<const sio_8211DirEntry> os_itr( ostr, "" );

  copy( dir.begin(), dir.end(), os_itr );

  // Write a field terminator to end the directory area.
  ostr << sio_8211FieldTerminator;

  // write out field data area
  return ostr;

} // operator<<
