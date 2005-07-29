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
// sio_8211DirEntry.cpp
//

#include "sio_8211DirEntry.h"

#ifndef INCLUDED_SIO_8211LEADER_H
#include "sio_8211Leader.h"
#endif

#ifndef INCLUDED_SIO_UTILS_H
#include "sio_Utils.h"
#endif


using namespace std;


sio_8211DirEntry::sio_8211DirEntry()
                : fieldLength_(0),
                  fieldPos_(0),
                  field_(0),
                  leader_(0)
{
}

sio_8211DirEntry::sio_8211DirEntry(sio_8211Leader & leader)
                : fieldLength_(0),
                  fieldPos_(0),
                  field_(0),
                  leader_( & leader )
{
}



sio_8211DirEntry::~sio_8211DirEntry()
{}




long
sio_8211DirEntry::getFieldLength() const
{
   return fieldLength_;
}




long
sio_8211DirEntry::getPosition() const
{
   return fieldPos_;
}



string const&
sio_8211DirEntry::getTag() const
{
   return fieldTag_;
}



sio_8211Field const*
sio_8211DirEntry::getField() const
{
   return field_;
}




sio_8211Leader const*
sio_8211DirEntry::getLeader() const
{
   return leader_;
} // sio_8211DirEntry::getLeader



//
// returns how many character positions >= 5 that the
// value will need
//
// XXX must be a more elegant way to do this
static
int
maxWidth_( long value )
{
   int width = 5;

   for ( long max_value = 10000; max_value != 0; max_value /= 10, width-- )
   {
      if ( value / max_value ) return width;
   }

   return ( width ) ? width : 1; // will at least be one character wide
} // maxWidth_



void
sio_8211DirEntry::setFieldLength( long length )
{
   fieldLength_ = length;

   long width = maxWidth_( fieldLength_ );
   if ( width > leader_->getSizeOfFieldLengthField() )
   {
      leader_->setSizeOfFieldLengthField( width );
   }
} // sio_8211DirEntry::setFieldLength()



void
sio_8211DirEntry::setPosition(long pos)
{
   fieldPos_ = pos;

   long width = maxWidth_( fieldPos_ );
   if ( width > leader_->getSizeOfFieldPosField() )
   {
      leader_->setSizeOfFieldPosField( width );
   }
} // sio_8211DirEntry::setPosition


void
sio_8211DirEntry::setTag(string const& tag)
{
   fieldTag_ = tag;

   if ( fieldTag_.length() > leader_->getSizeOfFieldTagField() )
   {
      leader_->setSizeOfFieldTagField( fieldTag_.length() );
   }
} // sio_8211DirEntry::setTag


void
sio_8211DirEntry::setField(sio_8211Field const* field)
{
   field_ = field;
} // sio_8211DirEntry::setField



void
sio_8211DirEntry::setLeader(sio_8211Leader * leader)
{
   leader_ = leader;
} // sio_8211DirEntry::setLeader



istream& 
operator>>(istream& istr, sio_8211DirEntry& dirEntry)
{
   // Assumes that istr is positioned on byte 0 of an ISO8211 record directory entry.
   // Reads the directory entry, leaving the stream pointer positioned on the byte
   // after the last byte in the entry.

   long totalLength = dirEntry.getLeader()->getSizeOfFieldLengthField()+ 
                      dirEntry.getLeader()->getSizeOfFieldPosField()+ 
                      dirEntry.getLeader()->getSizeOfFieldTagField();

   char* entryBuffer = new char[totalLength]; // XXX check that we actually got it

   istr.read(entryBuffer, totalLength);
   if ((istr.gcount() < totalLength) || (!istr))
      {
         istr.clear(ios::failbit);
         delete [] entryBuffer;
         return istr;
      }

   long rp = 0;  // relative position
   dirEntry.fieldTag_ = sio_Utils::getString(entryBuffer,rp,dirEntry.getLeader()->getSizeOfFieldTagField());
   rp = rp + dirEntry.getLeader()->getSizeOfFieldTagField();
   dirEntry.fieldLength_ = sio_Utils::getLong(entryBuffer,rp,dirEntry.getLeader()->getSizeOfFieldLengthField());
   rp = rp + dirEntry.getLeader()->getSizeOfFieldLengthField();
   dirEntry.fieldPos_ = sio_Utils::getLong(entryBuffer,rp,dirEntry.getLeader()->getSizeOfFieldPosField());

   delete [] entryBuffer;

   return istr;

} // operator>>



ostream& 
sio_8211DirEntry::streamInsert(ostream& ostr) const
{
   // Stuff the directory entry out to the ostream.
   // XXX - The xxxxSize_ fields really should be set somewhere, as well as 
   // sanity checked.
   ostr << setw(getLeader()->getSizeOfFieldTagField())      << fieldTag_;
   ostr << setw(getLeader()->getSizeOfFieldLengthField())   << fieldLength_;
   ostr << setw(getLeader()->getSizeOfFieldPosField())      << fieldPos_;
   return ostr;
}


ostream&
operator<<(ostream& ostr, sio_8211DirEntry const& dirEntry)
{
   return dirEntry.streamInsert(ostr);
}
