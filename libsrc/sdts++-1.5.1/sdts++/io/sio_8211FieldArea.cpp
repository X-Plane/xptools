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

#include <iostream>
#include <iterator>

#ifndef INCLUDED_SIO_8211FIELDAREA_H
#include <sdts++/io/sio_8211FieldArea.h>
#endif


using namespace std;


ostream&
sio_8211FieldArea::streamInsert(ostream& ostr) const
{
  ostream_iterator<sio_8211Field> os_itr( ostr, "" );

  copy( begin(), end(), os_itr );

  return ostr;
}


ostream&
operator<<(ostream& ostr, sio_8211FieldArea const& area)
{
  return area.streamInsert(ostr);
}
