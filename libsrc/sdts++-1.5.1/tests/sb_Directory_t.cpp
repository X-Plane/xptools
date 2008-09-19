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

#include <cassert>


#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_Reader.h>
#include <sdts++/container/sc_Record.h>
#include <sdts++/builder/sb_Catd.h>


using namespace std;



int
main( int argc, char** argv )
{

   if ( ! argv[1] ) 
   {
     cout << "usage: " << argv[0] << " CATD module file name" << endl;

     exit( 1 );
   }

   const string catd_filename( argv[1] );

   sb_Directory directory( catd_filename );

   // first insure we can get back the CATD module filename
   assert( catd_filename == directory.catdFilename() );

   // now try to retrieve the mandatory IDEN module CATD record
   sb_Catd iden_module_catd_entry;

   assert( directory.find( "IDEN", iden_module_catd_entry ) );

   // just blat out the CATD record for visual sanity checking
   cout << iden_module_catd_entry << endl;

   exit( 0 );
}
