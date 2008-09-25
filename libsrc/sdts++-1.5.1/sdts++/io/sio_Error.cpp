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
// sio_Error.cpp
//

#include <sdts++/io/sio_Error.h>


sio_Error::sio_Error()
   : state_( goodbit )
{}



bool
sio_Error::good() const
{ return ! state_; }



bool
sio_Error::bad() const
{ return state_ & badbit; }



bool
sio_Error::fail() const
{ return 0 != (state_ & (failbit | badbit)); }



int
sio_Error::rdstate() const
{ return state_; }



int
sio_Error::setstate( int state )
{ return state_ = state; }



int
sio_Error::clear()
{ return state_ = goodbit; }

