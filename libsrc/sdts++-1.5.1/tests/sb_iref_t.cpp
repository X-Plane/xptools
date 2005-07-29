//
// $Id: sb_iref_t.cpp,v 1.1 1999/03/11 22:32:54 mcoletti Exp $
//


#include <iostream>
#include <fstream>

using namespace std;

#include <sdts++/io/sio_8211Converter.h>
#include <sdts++/io/sio_Reader.h>
#include <sdts++/container/sc_Record.h>
#include <sdts++/builder/sb_Iref.h>



// binary 32 bit integer converter function
sio_8211Converter_BI32  bi32_converter;


int
main( int argc, char** argv )
{

  if ( ! argv[1] ) 
  {
    cerr << "usage: " << argv[0] << " IREF module " << endl;
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

    sb_Iref iref;

    while ( i )
      {
        if ( ! i.get( record ) ) break;

        cout << "what we read in:\n\n";
        cout << record << endl;

        iref.setRecord( record );
      
        cout << "\nand what the IREF object says it is:\n";
        cout << iref << endl;

        cout << "\nand what the record build from the IREF object says it is:\n";
        iref.getRecord( record );
        cout << record << endl;

        ++i;
      }
  }

                                // test building an IREF from scratch

  {
    sb_Iref iref;

    iref.setCOMT( "This is not a comment." );
    iref.setSATP( THREE_TUPLE );
    iref.setXLBL( "This is X." );
    iref.setYLBL( "This is Y." );
    iref.setHFMT( "BI32" );
    //    iref.setVFMT( "BFP64" );
    iref.setSFAX( 1.0 );
    iref.setSFAY( 2.0 );
    //    iref.setSFAZ( 3.0 );
    iref.setXORG( 4.0 );
    iref.setYORG( 5.0 );
    //    iref.setZORG( 6.0 );
    iref.setXHRS( 7.0 );
    iref.setYHRS( 8.0 );
    //    iref.setVRES( 9.0 );

// iref.setDimensionID( fid ); XXX we don't mess with foreign identifiers
// yet

    cout << iref << endl;

  }

  return 0;
}
