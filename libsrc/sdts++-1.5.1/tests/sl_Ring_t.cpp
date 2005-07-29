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

#include <iostream>
#include <fstream>

using namespace std;

#include <cassert>

#include <sdts++/logical/sl_Ring.h>


int
main( int argc, char** argv )
{

   sl_CompleteChainRing complete_ring;

   assert( "RU" == complete_ring.code() );


   sl_AreaChainRing area_ring;

   assert( "RU" == area_ring.code() );



   // XXX add checks for points() member


   exit( 0 );
}
