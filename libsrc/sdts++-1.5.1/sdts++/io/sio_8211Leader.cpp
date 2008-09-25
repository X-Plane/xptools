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

#ifdef NEED_TYPEINFO
#include <typeinfo>
#endif

#ifndef INCLUDED_SIO_8211LEADER_H
#include <sdts++/io/sio_8211Leader.h>
#endif




sio_8211Leader::sio_8211Leader()
              : recLength_(0),
                fieldAreaStart_(0),
                sizeFieldLength_(0),
                sizeFieldPos_(0),
                sizeFieldTag_(0)
{
}

sio_8211Leader::~sio_8211Leader()
{

}



long
sio_8211Leader::getRecordLength() const
{
   return recLength_;
}

void
sio_8211Leader::setRecordLength(long length)
{
   recLength_ = length;
}

char
sio_8211Leader::getLeaderIdentifier() const
{
   return leaderIden_;
}

void
sio_8211Leader::setLeaderIdentifier(char identifier)
{
   leaderIden_ = identifier;
}

long
sio_8211Leader::getBaseAddrOfFieldArea() const
{
   return fieldAreaStart_;
}

void
sio_8211Leader::setBaseAddrOfFieldArea(long base)
{
   fieldAreaStart_ = base;
}

long
sio_8211Leader::getSizeOfFieldLengthField() const
{
   return sizeFieldLength_;
}

void
sio_8211Leader::setSizeOfFieldLengthField(long size)
{
   sizeFieldLength_ = size;
}

long
sio_8211Leader::getSizeOfFieldPosField() const
{
   return sizeFieldPos_;
}

void
sio_8211Leader::setSizeOfFieldPosField(long size)
{
   sizeFieldPos_ = size;
}

long
sio_8211Leader::getSizeOfFieldTagField() const
{
   return sizeFieldTag_;
}

void
sio_8211Leader::setSizeOfFieldTagField(long size)
{
   sizeFieldTag_ = size;
}



// returns true if the given leader has valid values
bool
sio_8211Leader::isValid( ) const
{
                                // if there is no field data, then
                                // something is wrong

  if ( recLength_ < 1 ) return false;


                                // the leader identifier should be
                                // 'L', 'D', or 'R' to indicate
                                // whether it's a DDR leader, a DR
                                // leader, or a leader for a DR that's
                                // for repeating format records

  if ( ! ( 'L' == leaderIden_ ||
           'D' == leaderIden_ ||
           'R' == leaderIden_ ) )
    {
      return false;
    }


                                // if the field area start is strange,
                                // then it's bad

  if ( fieldAreaStart_ < 1 ) return false;

                                // if any of these are zero or
                                // negative, then something's awry

  if ( sizeFieldLength_ < 1 ||
       sizeFieldPos_ < 1    ||
       sizeFieldTag_ < 1 ) return false;

  return true;

} // sio_8211Leader::isValid



istream&
operator>>(istream& istr, sio_8211Leader& leader)
{
   return leader.streamExtract(istr);
}

ostream&
operator<<(ostream& ostr, sio_8211Leader const& leader)
{
   return leader.streamInsert(ostr);
}
