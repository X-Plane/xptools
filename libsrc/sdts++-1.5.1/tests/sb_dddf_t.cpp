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
// $Id: sb_dddf_t.cpp,v 1.3 2002/11/27 00:21:34 mcoletti Exp $
//


#include <iostream>
#include <fstream>


using namespace std;


#include <cassert>

#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_Reader.h>
#include <sdts++/container/sc_Record.h>
#include <sdts++/builder/sb_Dddf.h>


int
main( int argc, char** argv )
{

  if ( ! argv[1] )
    {
      cerr << "usage: " << argv[0] << " DDDF module " << endl;
      exit( 1 );
    }

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

  sio_8211Reader          reader( ddf );
  sc_Record               record;
  sio_8211ForwardIterator i( reader );

  {

    sb_Dddf dddf;

    while ( i )
      {
        if ( ! i.get( record ) ) break;

        cout << "what we read in:\n\n";
        cout << record << endl;

        dddf.setRecord( record );

        cout << "\nand what the DDDF object says it is:\n";
        cout << dddf << endl;

        cout << "\nand what the record built from the DDDF object says it is:\n";
        dddf.getRecord( record );

        cout << record << endl;

        ++i;
      }
  }

  // test building a record from scratch
  {
    sb_Dddf dddf;

    assert( dddf.setEORA( "ATTP" ) );

    assert( dddf.setEALB( "Label" ) );

    assert( dddf.setSRCE( "Source" ) );

    assert( dddf.setDFIN( "Definition" ) );

    assert( dddf.setAUTH( "Authority" ) );

    assert( dddf.setADSC( "Attribute Authority Description" ) );

    cout << dddf << endl;
  }

  exit(0);
}
