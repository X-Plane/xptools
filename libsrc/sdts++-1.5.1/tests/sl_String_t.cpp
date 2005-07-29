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

#include <sdts++/logical/sl_String.h>


int
main( int argc, char** argv )
{

   sl_String sl_string;

   assert( "LS" == sl_string.code() );


   {
       for ( int i = 0; i < 10; i++ )
       {
	   sl_string.push_back( sl_Point() );
	   sl_string.back().coordinate().assign( i, i * 10, i * 100 );
       }
   }


   for ( int i = 0; i < 10; i++ )
   {
      long int t;

      sl_string[i].coordinate().x().getI( t );
      assert( i       == t );

      sl_string[i].coordinate().y().getI( t );
      assert( i * 10  == t );

      sl_string[i].coordinate().z().getI( t );
      assert( i * 100 == t );
   }

   exit( 0 );
}
