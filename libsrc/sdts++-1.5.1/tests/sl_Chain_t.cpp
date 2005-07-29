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

#include <sdts++/logical/sl_Chain.h>


int
main( int argc, char** argv )
{

   sl_CompleteChain complete_chain;

   assert( "LE" == complete_chain.code() );



   sl_PlanarNetworkChain planarnetwork_chain;

   assert( "LW" == planarnetwork_chain.code() );



   sl_NonPlanarNetworkChain nonplanarnetwork_chain;

   assert( "LY" == nonplanarnetwork_chain.code() );



   sl_AreaChain area_chain;

   assert( "LL" == area_chain.code() );



   // XXX add checks for points() member


   exit( 0 );
}
