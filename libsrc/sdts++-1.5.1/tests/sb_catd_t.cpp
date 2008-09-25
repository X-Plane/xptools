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
      cout << "skipping read/write tests ..." << endl;
   }
   else
   {
#ifdef WIN32
      ifstream ddf( argv[1], ios::binary );
#else
      ifstream ddf( argv[1] );
#endif

      if ( ! ddf )
      {
         cerr << "couldn't open " << argv[1] << endl;
         exit( 2 );
      }

      sio_8211Reader  reader( ddf );
      sc_Record record;
      sio_8211ForwardIterator i( reader );

      {

         sb_Catd catd;

         while ( i )
         {
            if ( ! i.get( record ) ) break;

            cout << "what we read in:\n\n";
            cout << record << endl;

            catd.setRecord( record );

            cout << "\nand what the CATD object says it is:\n";
            cout << catd << endl;

            cout << "\nand what the record build from the CATD object says it is:\n";
            catd.getRecord( record );
            cout << record << endl;

            cout << "***\n\n";

            ++i;
         }
      }

   } // read/write tests

                                // test building an CATD from scratch
   {
      sb_Catd catd;

      assert( catd.setName( "is NAME" ) );

      assert( catd.setType( "is TYPE" ) );

      assert( catd.setVolume( "is Volume" ) );

      assert( catd.setFile( "is File" ) );

#ifdef NOT_SUPPORTED
      assert( catd.setRecord( "is Record" ) );
#endif

      assert( catd.setExternal( "is External" ) );

      assert( catd.setModuleVersion( "is Module Version" ) );

      assert( catd.setComment( "is Comment" ) );


      cout << catd << endl;
   }

  return 0 ;

}
