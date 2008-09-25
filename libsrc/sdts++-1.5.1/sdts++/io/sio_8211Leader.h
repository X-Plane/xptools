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
//  sio_8211Leader.h
//

#ifndef INCLUDED_SIO_8211LEADER_H
#define INCLUDED_SIO_8211LEADER_H

#include <iostream>
#include <iomanip>

using std::istream;
using std::ostream;


/// An ISO8211 Record Leader.
class sio_8211Leader
{
   public:

      ///
      sio_8211Leader();

      ///
      virtual ~sio_8211Leader();

      ///
      long getRecordLength() const;

      ///
      void setRecordLength(long length);

      ///
      char getLeaderIdentifier() const;

      ///
      void setLeaderIdentifier(char identifier);

      ///
      long getBaseAddrOfFieldArea() const;

      ///
      void setBaseAddrOfFieldArea(long base);

      ///
      long getSizeOfFieldLengthField() const;

      ///
      void setSizeOfFieldLengthField(long size);

      ///
      long getSizeOfFieldPosField() const;

      ///
      void setSizeOfFieldPosField(long size);

      ///
      long getSizeOfFieldTagField() const;

      ///
      void setSizeOfFieldTagField(long size);

      friend istream& operator>>(istream& istr, sio_8211Leader& leader);
      friend ostream& operator<<(ostream& ostr, sio_8211Leader const& leader);

   protected:

      ///
      virtual bool isValid() const;

      ///
      long recLength_;

      ///
      char leaderIden_;

      ///
      long fieldAreaStart_;

      ///
      long sizeFieldLength_;

      ///
      long sizeFieldPos_;

      ///
      long sizeFieldTag_;

      virtual istream& streamExtract(istream& istr) = 0;
      virtual ostream& streamInsert(ostream& ostr) const = 0;

};  // class sio_8211Leader



///
istream&
operator>>(istream& istr, sio_8211Leader& leader);


///
ostream&
operator<<(ostream& ostr, sio_8211Leader const& leader);


#endif // INCLUDED_SIO_8211LEADER_H

