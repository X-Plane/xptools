//
// $Id: sb_Spatial_t.cpp,v 1.2 2000/08/08 23:08:45 mcoletti Exp $
//

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include <sdts++/builder/sb_Spatial.h>





int
main( int argc, char** argv )
{
   sb_Spatial sadr;

   // insure that default values are, well, default; that is, they
   // should have no associated type or value

   assert( sadr.x().isUnvalued() );
   assert( sadr.y().isUnvalued() );
   assert( sadr.z().isUnvalued() );

   // now start tested set and get API

   sc_Subfield subfield;

   subfield.setBI32( 1 );

   sadr.x() = subfield;


   subfield.setBI32( 2 );

   sadr.y() = subfield;


   long val;

   assert( sadr.x().getBI32( val ) );
   assert( 1 == val );

   assert( sadr.y().getBI32( val ) );
   assert( 2 == val );

   exit( 0 );
}

