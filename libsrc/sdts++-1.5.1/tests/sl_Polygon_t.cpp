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

#include <sdts++/logical/sl_Polygon.h>


int
main( int argc, char** argv )
{

   sl_GPolygon g_poly;

   assert( "PG" == g_poly.code() );


   sl_GTChainPolygon gt_chain_poly;

   assert( "PC" == gt_chain_poly.code() );


   sl_GTRingPolygon gt_ring_poly;

   assert( "PR" == gt_ring_poly.code() );


   sl_UniverseChainPolygon universe_chain_poly;

   assert( "PW" == universe_chain_poly.code() );


   sl_UniverseRingPolygon universe_ring_poly;

   assert( "PU" == universe_ring_poly.code() );



   sl_VoidChainPolygon void_chain_poly;

   assert( "PX" == void_chain_poly.code() );


   sl_VoidRingPolygon void_ring_poly;

   assert( "PV" == void_ring_poly.code() );



   // XXX add checks for lines() member


   exit( 0 );
}
