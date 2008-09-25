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


#include <sdts++/container/sc_Record.h>

#include <iterator>
#include <algorithm>


using namespace std;



ostream&
operator<<( ostream& os , sc_Record const& record )
{
  ostream_iterator<sc_Field> os_itr( os, "\n" );

  copy( record.begin(), record.end(), os_itr );

  return os;
}
