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


#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_Reader.h>
#include <sdts++/container/sc_Record.h>
#include <sdts++/builder/sb_Xref.h>


int
main( int argc, char** argv )
{

  if ( ! argv[1] )
  {
    cerr << "usage: " << argv[0] << " XREF module " << endl;
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

  sio_8211Reader  reader( ddf );
  sc_Record record;
  sio_8211ForwardIterator i( reader );

  {

    sb_Xref xref;

    while ( i )
      {
        if ( ! i.get( record ) ) break;

        cout << "what we read in:\n\n";
        cout << record << endl;

        xref.setRecord( record );

        cout << "\nand what the XREF object says it is:\n";
        cout << xref << endl;

        cout << "\nand what the record build from the XREF object says it is:\n";
        xref.getRecord( record );
        cout << record << endl;

        ++i;
      }
  }

                                // test building an XREF from scratch
  {
    sb_Xref xref;

    assert( xref.setCOMT( "This is not a comment." ) );

    assert( xref.setRDOC("Reference Documentation") );

    assert( xref.setRSNM("UTM") );

    //    assert( xref.setVDAT("Vertical Datum") );

    // assert( xref.setSDAT("MLLW") );

    assert( xref.setHDAT("Horizontal Datum") );

    assert( xref.setZONE("UTM Zone") );

    assert( xref.setPROJ("Projection") );

    cout << xref << endl;
  }

  exit( 0 );

}
