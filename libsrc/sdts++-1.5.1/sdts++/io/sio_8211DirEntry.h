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
// sio_8211DirEntry.h
//

#ifndef INCLUDED_SIO_8211DIRENTRY_H
#define INCLUDED_SIO_8211DIRENTRY_H


#include <string>

#include <iostream>
#include <iomanip>


using std::string;
using std::ostream;
using std::istream;



class sio_8211Leader;
class sio_8211Field;

/// An ISO8211 Directory Entry.
class sio_8211DirEntry
{
   public:

      /**
         You can't really construct a valid DirEntry without a
         leader. This ctor allows sio_8211DirEntry to be stored in an
         STL container.
      */
      sio_8211DirEntry();


      /**
       The leader is required to determine the widths of the various
       subfields in a directory entry. Once the values are determined,
       the leader is no longer required (may be deleted).
      */
      sio_8211DirEntry(sio_8211Leader & leader);

      ///
      ~sio_8211DirEntry();

      /// We keep a ptr to the field we describe. We do not__ own it.
      sio_8211Field const* getField() const;

      ///
      void                 setField(sio_8211Field const* field);

      /// We keep a ptr to the leader we need for sizes. We do NOT own it.
      sio_8211Leader const*   getLeader() const;

      ///
      void                    setLeader(sio_8211Leader * leader);

      ///
      void setPosition(long pos);

      ///
      void setFieldLength( long length );

      ///
      void setTag(string const& tag);

      ///
      long           getFieldLength() const;

      ///
      long           getPosition() const;

      ///
      string const&  getTag() const;

      ///
      ostream& streamInsert(ostream& ostr) const;

      friend istream& operator>>(istream& istr, sio_8211DirEntry& dirEntry);
      friend ostream& operator<<(ostream& ostr, sio_8211DirEntry const& dirEntry);

   private:

      ///
      long     fieldLength_;

      ///
      long     fieldPos_;

      ///
      string   fieldTag_;

      /// pointer to corresponding field
      sio_8211Field const* field_;

      /// pointer to corresponding leader
      /**
       leader's widths changed dynamically
       with each sio_8211DirEntry::set*()
      */
      sio_8211Leader * leader_;

}; // class sio_8211DirEntry



///
istream& operator>>(istream& istr, sio_8211DirEntry& dirEntry);

///
ostream& operator<<(ostream& ostr, sio_8211DirEntry const& dirEntry);

#endif  // INCLUDED_SIO_8211DIRENTRY_H
