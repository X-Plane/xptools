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

#include <sdts++/logical/sl_Point.h>


int
main( int argc, char** argv )
{

   sl_Point point;

   assert( "NP" == point.code() );


   sl_AreaPoint area_point;

   assert( "NA" == area_point.code() );



   sl_EntityPoint entity_point;

   assert( "NE" == entity_point.code() );



   sl_LabelPoint label_point;

   assert( "NL" == label_point.code() );


   exit( 0 );
}
